#define IMPORT_SIFT_DLL

#include <vector>
#include <iostream>
#include "../src/lib.h"

int main() {
    try {
        auto[color, grad]=imageFC::imageFC("E:\\img\\l1.jpg", "E:\\img\\l2.jpg");
        std::cout << color << ' ' << grad << std::endl;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}