# 背景说明：
使用一个比较纯粹的C++代码，尽量不带平台的接口，出一份简单易用的多线程事件框架
代码的风格，就是简单，明了，不会刻意考虑极限性能

现在接触比较多的框架，基本上都是纯粹的异步，回调或者通知模型。因为首先它不会阻塞业务代码，其次，在很多脚本框架上，本身脚本运行就没有良好的多线程支持，必须通过异步。
对于很多自带GC的语言来说，异步相对很好，比如node.js，大量的回调，或者通过协程拼接业务，让代码看起来更连贯。但是到了C++上，由于需要考虑生命周期，大量的回调，就很麻烦了。

我在想，我直接将业务模型多线程化，纯粹的单个业务，就去阻塞执行，不在考虑异步，它会极大的简化代码开发。因为C++原生就在操作线程，这样抵扣下来，简单，性能也不会特别难看。
于是，想到什么就去做什么，作为一个业余有趣的活动也不错。

做自己喜欢的事情，是幸福的~~~

## 编译说明
1. 最初在windows下 visual studio上部署了第一版本；后面重点维护linux，环境是wsl2[ubuntu 18.04],g++ 7.5.0,依赖c++11
2. 需要的环境：
    ## boost 1.67 部署,其他版本也应该可以
    ## openssl 安装
    ## mysql客户端sdk https://github.com/mysql/mysql-connector-cpp.git branch:1.1.13
    ## libmysqlclient库安装
    ## redis环境安装

3. CMakeLists.txt我已经对Linux做了处理，会编译出带调试信息的版本，默认就支持编译
4. vscode的配置，我也提交上去,之后就不需要修改了

说明:
## mysql客户端环境安装
    sudo apt-get install libmysqlclient-dev
    安装过后，要看一下本地的LD_LIBRARY_PATH 环境变量是否包含 /usr/local/lib，因为libmysqlclient.so存放在这个位置
    如果LD_LIBRARY_PATH 不包含该目录，在启动的时候会报错，找不到so


编译:
# 创建buld目录，把临时文件收集起来，便于管理
mkdir build
# 生成makefile
cd build
cmake ../
# 生成可执行程序
make

然后就可以在vscode中使用c++插件调试test/main.cpp了

