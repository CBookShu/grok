# 项目概述
    此项目功能开发，目的在于把平时项目里想做但是很难实际开发的功能收集起来
    项目的功能有些已经经历了线上的检验，有一些可能仅仅是个【玩具】，但是尽量会保证功能运行的通过性
    开发时，是会稍微注重速度，先实现起来，后面再慢慢整理的【好看】一些
    虽然开发环境是ubuntu，但是并没有任何平台依赖性，即工程稍作修改，应该是可以在win32上运行的
    
# 功能描述
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

# 编译说明
### 目前的平台: Wsl2 Ubuntu18.0.4
### 依赖库
    sudo apt-get install libmysqlclient-dev  
    sudo apt-get install libboost-all-dev  
    sudo apt-get install libhiredis-dev
    sudo apt install libprotobuf-dev protobuf-compiler
    
### xmake 安装[一个比cmake更好用的工程管理工具]
#### 工程地址：https://github.com/xmake-io/xmake  
    sudo add-apt-repository ppa:xmake-io/xmake
    sudo apt update
    sudo apt install xmake
  

<!-- ### cpp mysql sdk
    git clone -b 1.1.13 git@github.com:mysql/mysql-connector-cpp.git  
    cd mysql-connector-cpp  
    cmake .  
    make  
    sudo make install   -->

### grok代码编译运行
    git clone git@github.com:CBookShu/grok.git  
    cd grok  
    xmake
    xmake run 

## TODO:
    1. redis异步同步mysql功能
    2. 联锁
    3. lua绑定
    4. 业务示例