# Z2S1603
A tiny smart house system.

## TSP

TS Protocol in different platform. Now it has ++Arduino++ version and ++python++ version

### basicframework 样例程序简述

**关于封包**

封包设计如下：
// head
start[2];
msg_type; // 1 for data, 2 for operation [0]
// payload
sub_type;				// [1]
data1[4];				// [2][3][4][5]
data2[4];				// [6][7][8][9]
data3[4];				// [10][11][12][13]
// tail
end;

共17bytes

**程序功能**

Arduino：
不断接收从串口发来的程序，并解析，每得到一个完整封包就组合成一个对象并做处理。样例中的处理时提取data3代表的int数值。如果等于0x16或十进制22，就改变pin 13灯的状态。所以程序正常应该可以收到python每5s发来的一个封包，data3在python中设定为0x16，所以灯会每5s左右改变一次状态。
另一方面，用定时器MsTimer2开启一个线程，每五秒发送一次数据，带有的数据是(26.5, 30, 35)。

Python：
从串口不断接收数据，每当收到一个完整封包，赋值给对象。处理方式就是打印data3的数值。
另一方面用thread开启一个线程，每5s发送一个封包，data3是22。
