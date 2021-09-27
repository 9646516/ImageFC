#ifndef IMAGEFC_HOGENCODER_H
#define IMAGEFC_HOGENCODER_H

#include <vector>
#include <tuple>
#include <deque>
#include <functional>
#include <cmath>
#include <immintrin.h>

namespace HOG {
    void encode(const std::vector<std::vector<float>> &src, std::vector<float> &dst);

    void mapConv3KernelSSE(const std::vector<std::vector<float>> &src, const std::vector<std::vector<float>> &kernel,
                           std::vector<std::vector<float>> &dst);

}

#endif