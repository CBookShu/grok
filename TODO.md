## 首要任务
    1.  启动msg的脚本
    2.  模拟消息接口
    3.  给Lua绑定各种接口
        1)  db接口 [doing]
        2） 消息接口
        3)  优化启动流程，在lua中通过配置接口启动
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

##     
    1. entity 用于各种类的组件化【TODO:用例】
    2. event 模拟实现了一套委托,可以用delegate来绑定事件;还有一个简单的function_wrapper;
        该功能用于让组件化的各个类互相通信【TODO:用例】
    3. eventPool 线程池，支持定时器，还有一个简化的strand基类，用于方便的实现多线程模块而不用考虑锁【TODO:用例】
    4. grokSimpleNet 简单的tcp server和client 代码。用例：test/netserver_test/netserver_test.cpp
    5. locklist 条件变量模板，用于保护一些数量有限的资源在多线程使用。用例: test/locklist_test/locklist_test.cpp
    6. mysqlConnector mysql pool 用例:test/mysqlpool_test/mysqlpool_test.cpp
    7. nodeService 节点类，用于实现多服务器通信。一个Center，其他client注册名字，互相通信。用例:test/nodeservice_test/nodeservice_test.cpp
    8. redisConnector redis pool 用例:test/redispool_test/redispool_test.cpp
    9. unionlock 一个本地化的多字符串锁 用例:test/unionlock_test/unionlock_test.cpp
    10. workPool 定制化的线程池，支持缓存、联锁。【TODO:用例】
    11. utils 包含: 类型安全的读写锁。【TODO: 用例】