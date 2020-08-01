依赖库:boost_1_67_0

1. 最初在windows下 visual studio上部署了第一版本；后面重点维护linux，环境是wsl2[ubuntu 18.04],g++ 7.5.0,依赖c++11
2. 首先安装，或者直接自己手动编译Boost1.67版本，不是说其他版本不支持，是自己在此版本之上进行的部署，其他版本应该也可以
3. CMakeLists.txt我已经对Linux做了处理，会编译出带调试信息的版本，默认就支持编译
4. vscode的配置，我也提交上去,之后就不需要修改了


编译:
# 创建buld目录，把临时文件收集起来，便于管理
mkdir build
# 生成makefile
cd build
cmake ../
# 生成可执行程序
make

然后就可以在vscode中使用c++插件调试test/main.cpp了

