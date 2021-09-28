#define IMPORT_SIFT_DLL

#include <vector>
#include <iostream>
#include "../src/lib.h"

int main() {
    try {
        auto x = imageFC::imageFC("E:\\img\\3.jpg", "E:\\img\\5.jpg");
        std::cout << "color = " << x.first << std::endl;
        std::cout << "grad = " << x.second << std::endl;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}