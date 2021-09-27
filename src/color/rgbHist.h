#ifndef IMAGEFC_RGBHIST_H
#define IMAGEFC_RGBHIST_H

#include <vector>
#include <algorithm>
#include <tuple>
#include <deque>
#include <array>
#include <functional>
#include <cmath>
#include <immintrin.h>
#include <iostream>

namespace rgbHist {
    void
    encode(const std::vector<std::vector<float>> &R, const std::vector<std::vector<float>> &G, const std::vector<std::vector<float>> &B,
           std::vector<std::vector<std::vector<int>>> &dst
    );
}

#endif
