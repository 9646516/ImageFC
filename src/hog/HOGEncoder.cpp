#include "HOGEncoder.h"

#define packedSizeAVX 8

void HOG::encode(const std::vector<std::vector<float>> &src, std::vector<float> &dst) {
    const int row = (int) src.size();
    const int col = (int) src.front().size();
    //calc gradr
    std::vector<std::vector<float>> img(row + 2, std::vector<float>(col + 3, 0));
    for (int i = 0; i < row; i++) {
        int j = 0;
        while (true) {
            __m256 to = _mm256_load_ps(src[i].data() + j);
            _mm256_store_ps(img[i + 1].data() + 1 + j, to);
            int j2 = j + packedSizeAVX;
            if (j2 >= col - packedSizeAVX)break;
            else j = j2;
        }
        for (; j < col; j++) {
            img[i + 1][j + 1] = src[i][j];
        }
    }
    const std::vector<std::vector<float>> dxKernel{
            {-1, 0, 1, 0},
            {-2, 0, 2, 0},
            {-1, 0, 1, 0}
    };
    const std::vector<std::vector<float>> dyKernel{
            {-1, -2, -1, 0},
            {0,  0,  0,  0},
            {1,  2,  1,  0}
    };

    std::vector<std::vector<float>> gx, gy, mag, angle;
    mapConv3KernelSSE(img, dxKernel, gx);
    mapConv3KernelSSE(img, dyKernel, gy);
    mag.resize(row);
    angle.resize(row);
//    180.0 / 3.15 = 57.142857
    const __m256 ps_v = _mm256_set1_ps(57.142857);

    for (int i = 0; i < row; i++) {
        int j = 0;
        mag[i].resize(col);
        angle[i].resize(col);
        while (true) {
            __m256 x = _mm256_load_ps(gx[i].data() + j);
            __m256 y = _mm256_load_ps(gy[i].data() + j);

            __m256 m = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(x, x), _mm256_mul_ps(y, y)));
            _mm256_store_ps(mag[i].data() + j, m);
            __m256 arc = _mm256_mul_ps(ps_v, _mm256_atan2_ps(y, x));

            _mm256_store_ps(angle[i].data() + j, arc);

            int j2 = j + packedSizeAVX;
            if (j2 >= col - packedSizeAVX)break;
            else j = j2;
        }
        for (; j < col; j++) {
            float x = gx[i][j];
            float y = gy[i][j];
            mag[i][j] = std::sqrt(x * x + y * y);
            angle[i][j] = std::atan2f(y, x) * 57.142857f;
        }
        for (j = 0; j < col; j++) {
            if (angle[i][j] < 0)angle[i][j] += 180;
        }
    }

    //split into bucket
    const int cellN = row / 8 + (row % 8 != 0);
    const int cellM = col / 8 + (col % 8 != 0);
    std::vector<std::vector<std::vector<float>>> histograms(
            cellN,
            std::vector<std::vector<float>>(cellM, std::vector<float>(9, 0))
    );
    for (int i = 0; i < row; i += 8) {
        for (int j = 0; j < col; j += 8) {
            std::vector<float> &bucket = histograms[i / 8][j / 8];
            for (int dx = 0; dx < 8 && i + dx < row; dx++) {
                for (int dy = 0; dy < 8 && j + dy < col; dy++) {
                    float direction = angle[i + dx][j + dy];
                    int now = ((int) (direction / 20.0)) % 9;
                    int nxt = (now + 1) % 9;
                    float offset = direction - now * 20;
                    bucket[now] += mag[i + dx][j + dy] * (20 - offset) / 20;
                    bucket[nxt] += mag[i + dx][j + dy] * offset / 20;
                }
            }
        }
    }

    //normalize
    dst.clear();
    std::deque<float> Q;
    std::function<void(int, int)> add = [&Q, &histograms](int i, int j) {
        float sum = 0;
        for (auto val: histograms[i][j]) {
            sum += val * val;
        }
        for (auto val: histograms[i + 1][j]) {
            sum += val * val;
        }
        Q.push_back(sum);
    };
    for (int i = 0; i + 1 < cellN; i++) {
        Q.clear();
        add(i, 0);
        for (int j = 1; j < cellM; j++) {
            add(i, j);
            float sum = std::sqrt(Q[0] + Q[1]);
            bool ok = std::abs(sum) > 1e-5;
            for (int dx: {0, 1}) {
                for (int dy: {-1, 0}) {
                    for (auto val: histograms[i + dx][j + dy]) {
                        if (ok) {
                            dst.push_back(val / sum);
                        } else {
                            dst.push_back(0);
                        }
                    }
                }
            }
            Q.pop_front();
        }
    }
}

void HOG::mapConv3KernelSSE(const std::vector<std::vector<float>> &src, const std::vector<std::vector<float>> &kernel,
                            std::vector<std::vector<float>> &dst) {
    const int n = (int) src.size() - 2;
    const int m = (int) src.front().size() - 3;
    __m128 A1 = _mm_load_ps(kernel[0].data());
    __m128 A2 = _mm_load_ps(kernel[1].data());
    __m128 A3 = _mm_load_ps(kernel[2].data());

    dst.resize(n);
    for (int i = 0; i < n; i++) {
        dst[i].resize(m);
        const float *ptr1 = src[i + 1 - 1].data();
        const float *ptr2 = src[i + 1 + 0].data();
        const float *ptr3 = src[i + 1 + 1].data();
        for (int j = 0; j < m; j++) {
            __m128 B1 = _mm_loadu_ps(ptr1++);
            __m128 B2 = _mm_loadu_ps(ptr2++);
            __m128 B3 = _mm_loadu_ps(ptr3++);
            dst[i][j] = _mm_cvtss_f32(_mm_dp_ps(A1, B1, 0xF1)) + _mm_cvtss_f32(_mm_dp_ps(A2, B2, 0xF1)) + _mm_cvtss_f32(_mm_dp_ps(A3, B3, 0xF1));
        }
    }
}