/*
 * test data
 * 7E 33 02 01 C0 0D 00 00 1E 00 00 00 23 00 00 00 7E
 * 17 bytes {0x7E, 0x33, 0x02, 0x01, 0xC0, 0x0D, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x7E}
 *             0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16   
 * 18 bytes 7E 33 02 01 C0 0D 00 00 1E 00 00 00 7D 5E 00 00 00 7E
 * 18 bytes {0x7E, 0x33, 0x02, 0x01, 0xC0, 0x0D, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x7D, 0x5E, 0x00, 0x00, 0x00, 0x7E}
 *             0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17
 * data3 = 126
*/

#include <TSP.h>
#include <malloc.h> 

DataFrame dataFrame;

// 数据转化方法
value = dataFrame.double2int32(35.2); //得到一个double对应的int形式的值，之后用于将这个int值填入数据域
//// 之后从dataFrame中往出读直接用getValueInt，getValueDouble往出读取

// 创建operation封包
dataFrame.setOperationDF(1, value, 30, 35);

// 获取某个位的值
double dValue = dataFrame.getValueDouble(1);
int iValue = dataFrame.getValueInt(3);

// 获得发送封包
uint8_t * send_data_buf = (uint8_t *)malloc(sizeof(uint8_t)*32);
int ds_len = dataFrame.makeSendData(&send_data_buf);
Serial.print(ds_len);Serial.println("<<len");
for(int i=0;i<ds_len;i++)
    Serial.println(send_data_buf[i], HEX);

free(send_data_buf);
//// 外部的数据发送代码
Serial.write(send_data_buf, ds_len);

// 获得字符串表示（一般用于打印查错，实际传输用字节流）
char * char_buf = (char *)malloc(sizeof(uint8_t)*96);
int str_len = getFrameString(send_data_buf, ds_len, &char_buf);
Serial.print(str_len);Serial.println("<<len");
Serial.println(char_buf);

free(char_buf);

// 解析字节流
//// data不带转义，data2带有转义（第三个data）
uint8_t data[17] = {0x7E, 0x33, 0x02, 0x01, 0xC0, 0x0D, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x7E};
uint8_t data2[18] = {0x7E, 0x33, 0x02, 0x01, 0xC0, 0x0D, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x7D, 0x5E, 0x00, 0x00, 0x00, 0x7E};
dataFrame.setDataByFormatByteStream(data);
double dValue = dataFrame.getValueDouble(1);
Serial.println(dValue);
dataFrame.setDataByOriginByteStream(data2, 18);
int iValue = dataFrame.getValueInt(3);
Serial.println(iValue);
