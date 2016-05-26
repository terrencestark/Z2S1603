#Linux CPP version - TSP

主要是串口配置和pthread部分的代码。

在linux上编译：
g++ basicFramework.cpp TSP.cpp -o framework -lpthread
运行：
./framework /dev/ttyUSB0
./framework /dev/ttyACM1
看具体插上Arduino在系统注册的设备名
