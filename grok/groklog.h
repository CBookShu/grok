#pragma once
#include <stdio.h>

#ifndef NDEBUG
#define DBG(fmt,...)        \
    printf("[%s][%d]:" fmt "\n",__FILE__,__LINE__,##__VA_ARGS__);
#else
// TODO: 未来把DBG改为真正的日志代码
#define DBG(fmt,...)        \
    printf("[%s][%d]:" fmt "\n",__FILE__,__LINE__,##__VA_ARGS__);
#endif