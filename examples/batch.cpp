#define IMPORT_SIFT_DLL

#include <vector>
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <fstream>
#include "../src/lib.h"

std::string to_string(int x) {
    std::string ret;
    while (x) {
        ret.push_back('0' + (x % 10));
        x /= 10;
    }
    std::reverse(ret.begin(), ret.end());
    if (ret.empty())ret.push_back('0');
    return ret;
}

int main() {
    try {
        std::queue<int> Q;
        for (int i = 0; i <= 178; i++) {
            Q.push(i);
        }
        std::mutex Q_locker, result_locker;
        std::vector<std::thread> V;
        std::vector<std::tuple<int, float, float>> result;

        const int NUMBER_OF_CPU = std::max(2, (int) std::thread::hardware_concurrency());
        std::cout << "NUMBER_OF_CPU = " << NUMBER_OF_CPU << std::endl;

        V.reserve(NUMBER_OF_CPU);
        for (int i = 0; i < NUMBER_OF_CPU; i++) {
            V.emplace_back([&]() {
                static const std::string base = R"(E:\img\char\img\)";
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
            });
        }
        for (int i = 0; i < NUMBER_OF_CPU; i++) {
            V[i].join();
        }

        std::ofstream os(R"(E:\img\char\report.html)");
        os << R"(<!DOCTYPE html>)" << '\n';
        os << R"(<html lang="en">)" << '\n';
        os << R"(<head>)" << '\n';
        os << R"(<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">)" << '\n';
        os << R"(<title>)" << '\n';
        os << R"(   batch test report)" << '\n';
        os << R"(</title>)" << '\n';
        os << R"(<style>)" << '\n';
        os << R"( img.cover {)" << '\n';
        os << R"(   height: 224px;)" << '\n';
        os << R"( })" << '\n';
        os << R"(div.res-container {)" << '\n';
        os << R"(   border: solid;)" << '\n';
        os << R"(   padding: 5px;)" << '\n';
        os << R"(   margin: 5px 2px;)" << '\n';
        os << R"(})" << '\n';
        os << R"(</style>)" << '\n';
        os << R"(</head> )" << '\n';
        os << R"(<body>)" << '\n';
        for (auto &i: result) {
            int idx = std::get<0>(i);
            std::string L = std::string("./img/") + to_string(idx << 1) + std::string(".jpg");
            std::string R = std::string("./img/") + to_string(idx << 1 | 1) + std::string(".jpg");
            os << R"(<div class="res-container">)" << '\n';
            os << R"(<p>Color Similarity: )" << std::get<1>(i) << R"(</p>)" << '\n';
            os << R"(<p>Grad Similarity: )" << std::get<2>(i) << R"(</p>)" << '\n';
            os << R"(<img class="cover" src=")" << L << R"(" alt="1">)" << '\n';
            os << R"(<img class="cover" src=")" << R << R"(" alt="2">)" << '\n';
            os << R"(</div>)" << '\n';
        }
        os << R"(</body>)" << '\n';
        os << R"(</html>)" << std::endl;
        os.close();
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}