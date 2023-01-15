#include "simpleLogService.h"
#include <boost/date_time/gregorian/gregorian.hpp>

SimpleDailyLogService::SimpleDailyLogService(const char*dir, const char* logname) {
    dir_ = dir;
    logname_ = logname;
}

SimpleDailyLogService::~SimpleDailyLogService() {
    timer_ = nullptr;
    if (fp_) {
        fclose(fp_);
        fp_ = nullptr;
    }
}

void SimpleDailyLogService::OnStart() {
    dateTag_ = getTodayDate();
    char filepath[512];
    memset(filepath, 0, sizeof(filepath));
    sprintf(filepath, "%s/%s_%d.log", dir_.c_str(), logname_.c_str(), dateTag_);

    fp_ = fopen(filepath, "a+");
    if (!fp_) {
        printf("error init log file %s", filepath);
        exit(1);
    }
}

void SimpleDailyLogService::OnStop() {
    timer_ = nullptr;
    if (this->evp().ios().stopped()) {
        if (fp_) {
            fclose(fp_);
            fp_ = nullptr;
        }
    }  else {
        auto r = async<void>([this](){
            if (fp_) {
                fclose(fp_);
                fp_ = nullptr;
            }
        }).wait_for(std::chrono::seconds(5));
        if (r != std::future_status::ready) {
            printf("SimpleDailyLogService may close error");
        }
    }
}

void SimpleDailyLogService::OnTest(bool&ok, std::string& cmd) {
    if (cmd == "SimpleDailyLogService") {
        const char* s = "hello world";
        this->pushLog(s, strlen(s));
    }
}

void SimpleDailyLogService::pushLog(const std::string& s) {
    // 这里需要将日志数据投递到strand中进行操作，所以需要一次拷贝
    std::shared_ptr<std::string> buffer = std::make_shared<std::string>(s);
    async<void>([buffer, this](){
        if (fp_) {
            fwrite(buffer->c_str(), buffer->size(), 1, fp_);
            fflush(fp_);
        }
    });
}

void SimpleDailyLogService::pushLog(const char*s, int len) {
    // 这里需要将日志数据投递到strand中进行操作，所以需要一次拷贝
    std::shared_ptr<std::string> buffer = std::make_shared<std::string>(s, len);
    async<void>([buffer, this](){
        if (fp_) {
            fwrite(buffer->c_str(), buffer->size(), 1, fp_);
            fflush(fp_);
        }
    });
}

void SimpleDailyLogService::OnTimerFresh() {
    auto dateTag = getTodayDate();
    if (dateTag_ != dateTag) {
        // 先创建新一天的文件
        char filepath[512];
        memset(filepath, 0, sizeof(filepath));
        sprintf(filepath, "%s/%s_d%d.log", dir_.c_str(), logname_.c_str(), dateTag);

        FILE* fpnew = fopen(filepath, "a+");
        if (!fpnew) {
            // 打开新的日志文件失败，则直接退出，仍然使用旧的文件进行日志输出，并等待下一次判断
            return;
        }

        // 只要新一天的文件创建成功，则修改tag和fp
        if (fp_) {
            fclose(fp_);        
            fp_ = nullptr;
        }
        fp_ = fpnew;
        dateTag_ = dateTag;
    }
}

int SimpleDailyLogService::getTodayDate() {
    auto today = boost::gregorian::day_clock::local_day();
    int year = today.year();
    int month = today.month();
    int day = today.day();

    int value = (year * 10000) + (month * 100) + day;

    return value;
}