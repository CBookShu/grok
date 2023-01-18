#include <iostream>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <algorithm>
#include <thread>
#include "grok/grok.h"

using namespace std;
using namespace grok;
using ULL = UnionLockLocal<std::string>;
/*
    测试用例:
    1.  多线程互相不竞争调用
    2.  多线程互相竞争调用
*/
void test_1() {
    ULL ull;
    ULL::Keys keys1 = {"1","2", "3"};
    ULL::Keys keys2 = {"4","5", "6"};
    ULL::Keys keys3 = {"7","8", "9"};
    std::thread t1([&](){
        auto g = ull.LockGuard(keys1);
        cout << "t1" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    std::thread t2([&](){
        auto g = ull.LockGuard(keys2);
        cout << "t2" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    std::thread t3([&](){
        auto g = ull.LockGuard(keys3);
        cout << "t3" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    t1.join();
    t2.join();
    t3.join();
}

void test_2() {
    ULL ull;
    ULL::Keys keys1 = {"1","2", "3"};
    ULL::Keys keys2 = {"3","4", "5"};
    ULL::Keys keys3 = {"5","6", "7"};
    std::thread t1([&](){
        auto g = ull.LockGuard(keys1);
        cout << "t1" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    std::thread t2([&](){
        auto g = ull.LockGuard(keys2);
        cout << "t2" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    std::thread t3([&](){
        auto g = ull.LockGuard(keys3);
        cout << "t3" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    t1.join();
    t2.join();
    t3.join();
}

int main(int argc, char**argv) {
    test_1();
    test_2();
    return 0;
}