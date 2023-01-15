#include <iostream>
#include "grok/grok.h"

using namespace std;
using namespace grok;
/*
    测试用例:
    1. thread <= locklist 验证
    2. thread > locklist 验证
    3. 测试初始化后立刻Get，然后再进行不断Give增加的情况
    4. 测试时调试走查析构函数，资源控制等
*/

void test_1_2(int m,int n) {
    int threadnum = m;
    int locklistcount = n;
    std::vector<std::thread> threads;
    LockList<int> ll;
    for(int i = 0; i < locklistcount; ++i) {
        ll.Give(new int(i));
    }

    for(int i = 0; i < threadnum; ++i) {
        threads.push_back(std::thread([&](){
            auto guard = ll.GetByGuard();
            int * p = guard.Get();
            cout << *p << endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }));
    }
    for(int i = 0; i < threadnum; ++i) {
        threads[i].join();
    }
}

void test_3() {
    // 此时ll是空的
    LockList<int> ll;
    // 立刻起线程进行Get
    auto t1 = std::thread([&](){
        for (int i = 0; i < 10; ++i) {
            auto g = ll.GetByGuard();
            int n = *g.Get();
            cout << n << endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    // 睡眠一下，保证t1一定执行起来了
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Give的值姗姗来迟~
    auto t2 = std::thread([&](){
        for(int i = 0; i < 5;++i) {
            ll.Give(new int(i));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    t2.join();
    t1.join();
}

int main(int argc, char**argv) {
    test_1_2(4, 4);
    test_1_2(4, 1);
    test_3();
    return 0;
}