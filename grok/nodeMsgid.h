#pragma once

enum NodeServiceMsgID {
    // node client发往node center进行注册用的
    NODE_SERVICE_REGISTER = 1,
    //TODO: 找不到要处理的node 
    NODE_SERVICE_NODEMISS = 2,
    //TODO: 对应的node不处理这样的消息
    NODE_SERVICE_MSGIDMISS = 3,

    // 自定义消息
    NODE_SERVICE_CUSTOM = 1000,
};