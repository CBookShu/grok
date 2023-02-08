#pragma once
#include "grok/grok.h"

grok::stdtimerPtr  listen_file_modify(
    grok::WorkStaff& staff, 
    const char* path, 
    std::function<void(const char*)> cb,
    int millsec);
