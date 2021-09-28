#include "rgbHist.h"

#define packedSizeAVX 8

void rgbHist::encode(const std::vector<std::vector<float>> &R,
                     const std::vector<std::vector<float>> &G,
                     const std::vector<std::vector<float>> &B,
                     std::vector<std::vector<std::vector<int>>> &dst
) {
    const int binSize = 42;
    const float K = 255.0f / binSize;
    const int numberOfBins = 255 / binSize;
    const int row = (int) R.size();
    const int col = (int) R.front().size();

    std::vector<std::vector<std::vector<int>>> bin(numberOfBins, std::vector<std::vector<int>>(numberOfBins, std::vector<int>(numberOfBins, 0)));

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            int idxOfR = std::min(int(R[i][j] * K), numberOfBins - 1);
            int idxOfG = std::min(int(G[i][j] * K), numberOfBins - 1);
            int idxOfB = std::min(int(B[i][j] * K), numberOfBins - 1);
            bin[idxOfR][idxOfG][idxOfB]++;
        }
    }
    dst.resize(numberOfBins);
    for (int i = 0; i < numberOfBins; i++) {
        dst[i].resize(numberOfBins);
        for (int j = 0; j < numberOfBins; j++) {
            dst[i][j].resize(numberOfBins);
            for (int k = 0; k < numberOfBins; k++) {
                dst[i][j][k] = bin[i][j][k];
            }
        }
    }
}