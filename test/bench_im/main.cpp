#include <iostream>
#include "../../grok/grok.h"
#include <chrono>
using namespace grok;
using namespace std;

static int n = 10;
static int func() {
    for(int i = 0; i < 100;++i) {
        n += i;
    }
    return n;
}

class OpTest {
public:
    int func() {
        for(int i = 0; i < 100;++i) {
            n += i;
        }
        return n;
    }
};

template <typename F>
static void invoke_n(F& f, int n) {
    for (size_t i = 0; i < n; i++)
    {
        f();
    }
}

// 测试ImportFunctional 和 原生函数调用之间的差别
// 基本就是 std::function和原生之间的差别了，几乎可以忽略不计
// 除非真的是在那些递归大量调用对CPU要求非常高的地方，可以再考虑
// add1: 在使用release 编译加上优化之后，纯函数由于没有任何内容，被优化为空，耗时为0
// add2: 在调整代码使得release优化非空的情况下，原生调用的耗时是event的数十倍提高！
int main(int argc,char**argv) {
    int count = 1000000;

    // 第一种，纯函数
    ImportFunctional<int()> f = make_function_wrapper(func);
    auto t1 = std::chrono::system_clock::now();
    invoke_n(func, count);
    auto t2 = std::chrono::system_clock::now();
    invoke_n(f, count);
    auto t3 = std::chrono::system_clock::now();

    cout << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
    cout << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() << endl;

    OpTest p;
    auto lf = [&p](){return p.func();};
    // 第二种,类函数
    ImportFunctional<int()> f1 = make_function_wrapper(&p, &OpTest::func);
    t1 = std::chrono::system_clock::now();
    invoke_n(lf, count);
    t2 = std::chrono::system_clock::now();
    invoke_n(f1, count);
    t3 = std::chrono::system_clock::now();
    cout << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
    cout << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() << endl;

    return 0;
}