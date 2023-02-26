#include <iostream>
#include "grok/lru.h"

using namespace std;
using namespace grok;

struct TestV {
    int m_v;
    TestV(int v):m_v(v){}
};

int main(int argc, char**argv) {
    gLru<int,int> lru(5);
    lru.set(10, 10);
    lru.get(10);
    for (int i = 0; i < 6; ++i) {
        lru.set(i*10, i*11);
    }

    int k = 10;
    gLru<std::string, TestV> lru1(5);
    for (int i = 0; i < 100; ++i) {
        lru1.set(std::to_string(i * 10), i * 11);
    }
    for(int i = 0; i < 100; ++i) {
        auto* v = lru1.get(std::to_string(i * 10));
        if (v) {
            printf("[%d]=[%d],", i * 10, v->m_v);
        }
    }
    cout << endl;
    return 0;
}