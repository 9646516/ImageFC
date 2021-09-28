#define IMPORT_SIFT_DLL

#include <vector>
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <fstream>
#include "../src/lib.h"

int main() {
    try {
        std::ofstream os(R"(E:\img\char\report.html)");
        os << R"(<!DOCTYPE html>)" << std::endl;
        os << R"(<html lang="en">)" << std::endl;
        os << R"(<head>)" << std::endl;
        os << R"(<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">)" << std::endl;
        os << R"(<title>)" << std::endl;
        os << R"(   batch test report)" << std::endl;
        os << R"(</title>)" << std::endl;
        os << R"(<style>)" << std::endl;
        os << R"( img.cover {)" << std::endl;
        os << R"(   height: 224px;)" << std::endl;
        os << R"( })" << std::endl;
        os << R"(div.res-container {)" << std::endl;
        os << R"(   border: solid;)" << std::endl;
        os << R"(   padding: 5px;)" << std::endl;
        os << R"(   margin: 5px 2px;)" << std::endl;
        os << R"(})" << std::endl;
        os << R"(</style>)" << std::endl;
        os << R"(</head> )" << std::endl;
        os << R"(<body>)" << std::endl;
        std::queue<int> Q;
        for (int i = 0; i <= 178; i++) {
            Q.push(i);
        }
        std::mutex Q_locker, result_locker;
        std::vector<std::thread> V;
        const std::string base = R"(E:\img\char\img\)";
        std::function<std::string(int)> to_string = [](int x) -> std::string {
            std::string ret;
            while (x) {
                ret.push_back('0' + (x % 10));
                x /= 10;
            }
            std::reverse(ret.begin(), ret.end());
            if (ret.empty())ret.push_back('0');
            return ret;
        };
        std::vector<std::tuple<int, float, float>> result;
        std::function<void(void)> F = [&]() {
            while (true) {
                int idx;
                {
                    std::lock_guard<std::mutex> guard(Q_locker);
                    if (Q.empty())break;
                    idx = Q.front();
                    Q.pop();
                }
                std::string L = base + to_string(idx << 1) + std::string(".jpg");
                std::string R = base + to_string(idx << 1 | 1) + std::string(".jpg");
                auto x = imageFC::imageFC(L.c_str(), R.c_str());
                {
                    std::lock_guard<std::mutex> guard(result_locker);
                    result.emplace_back(idx, x.first, x.second);
                }
            }
        };
        const int NUMBER_OF_CPU = std::max(2, (int) std::thread::hardware_concurrency());
        std::cout << "NUMBER_OF_CPU = " << NUMBER_OF_CPU << std::endl;

        V.reserve(NUMBER_OF_CPU);
        for (int i = 0; i < NUMBER_OF_CPU; i++) {
            V.emplace_back(F);
        }
        for (int i = 0; i < NUMBER_OF_CPU; i++) {
            V[i].join();
        }
        for (auto &i: result) {
            int idx = std::get<0>(i);
            std::string L = std::string("./img/") + to_string(idx << 1) + std::string(".jpg");
            std::string R = std::string("./img/") + to_string(idx << 1 | 1) + std::string(".jpg");
            os << R"(<div class="res-container">)" << std::endl;
            os << "<p>Color Similarity: " << std::get<1>(i) << "</p>" << std::endl;
            os << "<p>Grad Similarity: " << std::get<2>(i) << "</p>" << std::endl;
            os << R"(<img class="cover" src=")" << L << R"(" alt="1">)" << std::endl;
            os << R"(<img class="cover" src=")" << R << R"(" alt="2">)" << std::endl;
            os << R"(</div>)" << std::endl;
        }
        os << R"(</body>)" << std::endl;
        os << R"(</html>)" << std::endl;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}