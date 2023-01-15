#include "configTool.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>


ConfigTool::ConfigTool(char const* path) {
    processPath_ = boost::filesystem::path(path);
    auto dir = processPath_.parent_path().string();
    initPath_ = dir + "/" + processPath_.filename().string() + ".ini";
}

void ConfigTool::readNum(const char*area, const char* key, int& number) {
    try {
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini("/home/cbookshu/github/grok/build/demo.ini", pt);
        auto& child = pt.get_child(area);
        number = child.get<int>(key, number);

    } catch(std::exception& e) {

    }
    
}

void ConfigTool::readStr(const char*area, const char* key, std::string& str) {
    try {
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini("/home/cbookshu/github/grok/build/demo.ini", pt);
        auto& child = pt.get_child(area);
        str = child.get<std::string>(key, str);

    } catch(std::exception& e) {

    }
}

std::string ConfigTool::getProcessName() {
    return processPath_.filename().string();
}

std::string ConfigTool::getProcessDir() {
    return processPath_.parent_path().string();
}