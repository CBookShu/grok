# 功能描述
    1. 通过Keys在单机[未来支持多进程]上进行任务细粒度的划分，解决业务锁问题
    2. 提取出event、threadpools、component等组件，可以把业务尽量的解耦合
    3. TODO: 未来会加上redis和mysql的灵活使用
    4. 目的是对过去服务端开发的经验进行一个总结

# 编译说明
### 目前的平台: Wsl2 Ubuntu18.0.4
### 依赖库
    sudo apt-get install libmysqlclient-dev  
    sudo apt-get install libboost-dev-all  

### xmake 安装[一个比cmake更好用的工程管理工具]
    工程地址：https://github.com/xmake-io/xmake
### 安装
    bash <(wget https://xmake.io/shget.text -O -)  

### cpp mysql sdk
    git clone -b 1.1.13 git@github.com:mysql/mysql-connector-cpp.git  
    cd mysql-connector-cpp  
    cmake .  
    make  
    sudo make install  

### grok代码编译运行
    git clone git@github.com:CBookShu/grok.git  
    cd grok  
    xmake
    xmake run example

## TODO:
    1. dbentry，redis和Mysql的sdk连接
    2. 服务器和客户端的通信问题
    3. lua绑定
    4. 业务开发