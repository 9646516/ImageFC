#ifndef IMAGEFC_LIB_CC
#define IMAGEFC_LIB_CC

#define packedSizeAVX 8
#define STB_IMAGE_IMPLEMENTATION

#include "io/stb_image.h"
#include "color/rgbHist.h"
#include "hog/HOGEncoder.h"
#include <vector>
#include <tuple>
#include <deque>
#include <array>
#include <functional>
#include <cmath>
#include <immintrin.h>
#include "flow/mcmf.h"

#ifdef IMPORT_SIFT_DLL
#define IMAGE_FC_API __declspec(dllimport)
#else
#define IMAGE_FC_API __declspec(dllexport)
#endif

namespace imageFC {
    struct imageFeature {
        std::vector<std::vector<std::vector<int>>> hist;
        std::vector<float> HOG;
    };

    IMAGE_FC_API std::unique_ptr<imageFC::imageFeature>
    extractFeature(const std::vector<std::vector<float>> &R, const std::vector<std::vector<float>> &G,
                   const std::vector<std::vector<float>> &B);

    inline float sumOfVec(__m256 x);

    float calcCosDistance(std::vector<float> &lhs, std::vector<float> &rhs);

    float calcEMDDistance(const std::vector<std::vector<std::vector<int>>> &lhs, const std::vector<std::vector<std::vector<int>>> &rhs);

    IMAGE_FC_API void
    imread(const char *sb, std::vector<std::vector<float>> &R, std::vector<std::vector<float>> &G, std::vector<std::vector<float>> &B);

    IMAGE_FC_API std::pair<float, float> calcDistance(std::unique_ptr<imageFC::imageFeature> lhs, std::unique_ptr<imageFC::imageFeature> rhs);

    IMAGE_FC_API  std::pair<float, float> imageFC(const char *lhs, const char *rhs);

    void resize(const std::vector<std::vector<float>> &src, std::vector<std::vector<float>> &dst, int row, int col);
}

#endif