#include "utils.h"
#include <boost/filesystem.hpp>

static time_t get_file_last_write_time(const char* path) {
    boost::filesystem::path p(path);
    boost::system::error_code ec;
    auto t = boost::filesystem::last_write_time(p, ec);
    return ec ? 0 : t;
}

struct FileListenerNode {
    std::string path;
    time_t file_last_write_time;
};
grok::stdtimerPtr listen_file_modify(grok::WorkStaff& staff, const char *path, std::function<void(const char *)> cb)
{
    auto sptr = std::make_shared<FileListenerNode>();
    sptr->path = path;
    sptr->file_last_write_time = get_file_last_write_time(path);
    return staff.evp().loopTimer([cb,sptr](){
        auto r = get_file_last_write_time(sptr->path.c_str());
        if (r == 0) {
            return;
        }
        if(r != sptr->file_last_write_time) {
            sptr->file_last_write_time = r;
            cb(sptr->path.c_str());
        }
    },std::chrono::seconds(30),staff.strand());
}
