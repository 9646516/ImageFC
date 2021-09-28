#include "lib.h"

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || __cplusplus >= 202002L)
#define lerp(a, b, t) std::lerp(a,b,t)
#else
#define lerp(a, b, t) ((a)+t*((b)-(a)))
#endif

void imageFC::resize(const std::vector<std::vector<float>> &src, std::vector<std::vector<float>> &dst, int row, int col) {
    std::vector<std::vector<float>> _dst;
    int srcRow = (int) src.size();
    int srcCol = (int) src.front().size();
    float dx = static_cast<float>(srcRow) / static_cast<float>(row);
    float dy = static_cast<float>(srcCol) / static_cast<float>(col);
    _dst.resize(row);
    for (int i = 0; i < row; i++) {
        _dst[i].resize(col);
        for (int j = 0; j < col; j++) {
            float posX = dx * static_cast<float>(i);
            float posY = dy * static_cast<float>(j);
            int x0 = int(posX);
            int y0 = int(posY);
            int x1 = x0 + 1;
            if (x1 >= srcRow)x1 = x0;
            int y1 = y0 + 1;
            if (y1 >= srcCol)y1 = y0;
            float ox = posX - x0;
            float oy = posY - y0;
            float topInterpolationResult = lerp(static_cast<float>(src[x0][y0]), src[x0][y1], oy);
            float bottomInterpolationResult = lerp(static_cast<float>(src[x1][y0]), src[x1][y1], oy);
            float res = lerp(topInterpolationResult, bottomInterpolationResult, ox);
            _dst[i][j] = res;
        }
    }
    std::swap(dst, _dst);
}


std::unique_ptr<imageFC::imageFeature>
imageFC::extractFeature(const std::vector<std::vector<float>> &R,
                        const std::vector<std::vector<float>> &G,
                        const std::vector<std::vector<float>> &B
) {
    std::vector<std::vector<std::vector<int>>> lab;
    rgbHist::encode(R, G, B, lab);

    std::vector<std::vector<float>> Gray;

    const int row = (int) R.size();
    const int col = (int) R.front().size();
    Gray.resize(row);
    for (int i = 0; i < row; i++) {
        Gray[i].resize(col);
        int j = 0;
        while (true) {
            __m256 r = _mm256_load_ps(R[i].data() + j);
            __m256 g = _mm256_load_ps(G[i].data() + j);
            __m256 b = _mm256_load_ps(B[i].data() + j);
            r = _mm256_mul_ps(r, _mm256_set1_ps(0.299));
            g = _mm256_mul_ps(g, _mm256_set1_ps(0.587));
            b = _mm256_mul_ps(b, _mm256_set1_ps(0.114));

            r = _mm256_add_ps(_mm256_add_ps(r, g), b);

            _mm256_store_ps(Gray[i].data() + j, r);
            int j2 = j + packedSizeAVX;
            if (j2 >= col - packedSizeAVX)break;
            else j = j2;
        }
        for (; j < col; j++) {
            Gray[i][j] = R[i][j] * 0.299f + G[i][j] * 0.587f + B[i][j] * 0.114f;
        }
    }
    std::vector<float> hog;
    HOG::encode(Gray, hog);
    std::unique_ptr<imageFeature> ret = std::make_unique<imageFeature>();
    ret->hist = std::move(lab);
    ret->HOG = std::move(hog);
    return ret;
}

inline float imageFC::sumOfVec(__m256 x) {
    __m128 hiQuad = _mm256_extractf128_ps(x, 1);
    __m128 loQuad = _mm256_castps256_ps128(x);
    __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
    __m128 loDual = sumQuad;
    __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
    __m128 sumDual = _mm_add_ps(loDual, hiDual);
    __m128 lo = sumDual;
    __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
    __m128 sum = _mm_add_ss(lo, hi);
    return _mm_cvtss_f32(sum);
}

float imageFC::calcCosDistance(std::vector<float> &lhs, std::vector<float> &rhs) {
    if (lhs.size() != rhs.size()) {
        throw std::exception("cos distance require same feature size");
    } else {
        double ret = 0;
        int j = 0;
        double s1 = 0, s2 = 0;
        const int sz = (int) lhs.size();
        while (true) {
            __m256 fst = _mm256_load_ps(lhs.data() + j);
            __m256 snd = _mm256_load_ps(rhs.data() + j);

            s1 += sumOfVec(_mm256_mul_ps(fst, fst));
            s2 += sumOfVec(_mm256_mul_ps(snd, snd));


            __m256 dp = _mm256_dp_ps(fst, snd, 0xF1);
            __m128 hi = _mm256_extractf128_ps(dp, 1);
            __m128 lo = _mm256_castps256_ps128(dp);
            ret += _mm_cvtss_f32(hi) + _mm_cvtss_f32(lo);

            int j2 = j + packedSizeAVX;
            if (j2 >= sz - packedSizeAVX)break;
            else j = j2;
        }
        for (; j < sz; j++) {
            ret += lhs[j] * rhs[j];
            s1 += lhs[j] * lhs[j];
            s2 += rhs[j] * rhs[j];
        }
        s1 = std::sqrt(s1);
        s2 = std::sqrt(s2);
        if (std::abs(s1) < 1e-4 && std::abs(s2) < 1e-4)return 1;
        else if (std::abs(s1) > 1e-4 && std::abs(s2) > 1e-4)return float(ret / s1 / s2);
        else return 0;
    }
}

void
imageFC::imread(const char *sb, std::vector<std::vector<float>> &R, std::vector<std::vector<float>> &G, std::vector<std::vector<float>> &B) {
    int n, m, nc;
    uint8_t *data = stbi_load(sb, &n, &m, &nc, 3);
    uint8_t *raw = data;
    if (!data)throw std::exception("read image failed");
    R.resize(n);
    G.resize(n);
    B.resize(n);
    for (int i = 0; i < n; i++) {
        R[i].resize(m);
        G[i].resize(m);
        B[i].resize(m);
        for (int j = 0; j < m; j++) {
            uint8_t r = *data++;
            uint8_t g = *data++;
            uint8_t b = *data++;
            R[i][j] = float(r) / 255.0f;
            G[i][j] = float(g) / 255.0f;
            B[i][j] = float(b) / 255.0f;
        }
    }
    stbi_image_free(raw);
}

std::pair<float, float> imageFC::calcDistance(std::unique_ptr<imageFC::imageFeature> x, std::unique_ptr<imageFC::imageFeature> y) {
    try {
        float dis1 = calcEMDDistance(x->hist, y->hist);
        float dis2 = calcCosDistance(x->HOG, y->HOG);
        return std::make_pair(dis1, dis2);
    } catch (std::exception &e) {
        throw e;
    }
}

std::pair<float, float> imageFC::imageFC(const char *lhs, const char *rhs) {
    try {
        std::vector<std::vector<float>> r1, g1, b1;
        std::vector<std::vector<float>> r2, g2, b2;
        imread(lhs, r1, g1, b1);
        imread(rhs, r2, g2, b2);
        for (auto i: {&r1, &r2, &g1, &g2, &b1, &b2}) {
            resize(*i, *i, 128, 128);
        }
        auto f1 = extractFeature(r1, g1, b1);
        auto f2 = extractFeature(r2, g2, b2);
        auto ret = calcDistance(std::move(f1), std::move(f2));
        return ret;
    } catch (std::exception &e) {
        throw e;
    }
}

float imageFC::calcEMDDistance(const std::vector<std::vector<std::vector<int>>> &lhs, const std::vector<std::vector<std::vector<int>>> &rhs) {
    const int sz1 = (int) lhs.size();
    const int sz2 = (int) lhs.front().size();
    const int sz3 = (int) lhs.front().front().size();
    const int N = sz1 * sz2 * sz3;
    std::unique_ptr<MCMF> solver = std::make_unique<MCMF>();
    const int numberOfV = N + N + 2 + 5;
    solver->init(numberOfV);
    const int source = N + N + 4;
    const int sink = N + N + 3;
    auto add = [&](int x, int y, int z) {
        int idx = x * sz2 * sz3 + y * sz3 + z;
        int idx2 = N;
        solver->add(source, idx, lhs[x][y][z], 0);
        for (int i = 0; i < sz1; i++) {
            for (int j = 0; j < sz2; j++) {
                for (int k = 0; k < sz3; k++) {
                    solver->add(idx, idx2++, solver->INF, std::abs(x - i) + std::abs(y - j) + std::abs(z - k));
                }
            }
        }
        solver->add(idx + N, sink, rhs[x][y][z], 0);
    };
    for (int i = 0; i < sz1; i++) {
        for (int j = 0; j < sz2; j++) {
            for (int k = 0; k < sz3; k++) {
                add(i, j, k);
            }
        }
    }
    int cost = solver->MinCost(source, sink);
    float res = 1.0 * cost / sz1 / sz2 / sz3;
    return 1 - std::tanh(res / 100.0f);
}
