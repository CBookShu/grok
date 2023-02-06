## lua绑定
    1.  消息处理的lua脚本设计
    2.  lua model的设计
    3.  给Lua绑定各种接口
## redis缓存mysql功能


## 业务实现
    1.  玩家登录
    2.  任务
    3.  排行赛

## node协议修改
    1.  原始c包增加type，标识register
    2.  将msgpack的解析分离出去
    3.  将原始数据投递给lua，在lua中进行解析
    4.  c++ 中通过绑定进行解析
    5.  node client 增加断开不关闭，自动重连，持续维护stream重连后继续投递原有的消息