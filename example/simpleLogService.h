#pragma once
#include "../grok/grok.h"
#include <iostream>

/*
    一天一个文件记录日志
    1. 没有缓存直接输出，每次输出都要flush
    2. 通过定时器来触发文件的更新，轮询而不是每次操作的时候进行判断，一分钟轮询的频率，一天下来肯定比日志操作量大吧
*/

class SimpleDailyLogService : public grok::WorkStaff
{
public:
    SimpleDailyLogService(const char*dir, const char* logname);
    ~SimpleDailyLogService();

    void OnStart();
    void OnStop();

    void OnTest(bool&ok, std::string& cmd);

    void pushLog(const std::string& s);
    void pushLog(const char*s, int len);

protected:
    void OnTimerFresh();

    static int getTodayDate();

private:
    int dateTag_ = 0; // 时间戳代表当前的日志在哪一天
    std::string logname_;
    std::string dir_;
    grok::stdtimerPtr timer_;   // 每分钟刷新一次查看是否该换新的文件了

    FILE* fp_ = nullptr;
};  