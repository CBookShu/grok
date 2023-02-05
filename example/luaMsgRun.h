#pragma once
#include "grok/grok.h"

class LuaMsgRun : public grok::GrokRunable {
    grok::Session::Ptr s;
    grok::MsgPackSPtr p;

    virtual void run() override;
};
