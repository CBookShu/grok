#pragma once
#include <stdio.h>

#ifndef NDEBUG
#define DBG(fmt,...)        \
    printf("[%s][%d]:" fmt "\n",__FILE__,__LINE__,##__VA_ARGS__);
#else
#define DBG(fmt,...)        
#endif