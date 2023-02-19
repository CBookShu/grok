# 项目概述
    lua绑定的节点服务器项目
    
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

### grok代码编译运行【项目依赖redis和mysql服务器，具体配置可以在example.cpp的init中查看]
    git clone git@github.com:CBookShu/grok.git  
    cd grok  
    xmake
    xmake run test_group
