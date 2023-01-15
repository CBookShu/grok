#pragma once
#include <iostream>
#include <boost/filesystem.hpp>


class ConfigTool
{
public:
    ConfigTool(char const* path);

    void readNum(const char*area, const char* key, int& number);
    void readStr(const char*area, const char* key, std::string& str);

    std::string getProcessName();
    std::string getProcessDir();
private:
    boost::filesystem::path processPath_;
    std::string initPath_;
};