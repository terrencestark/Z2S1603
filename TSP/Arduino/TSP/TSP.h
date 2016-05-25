#ifndef TSP_H
#define TSP_H

#include <inttypes.h>

using namespace std;

int8_t getFrameString(uint8_t * data, int data_len, char * char_buf[96]);//32*3


class DataFrame
{
public:
  DataFrame();
  // util
  uint32_t double2int32(double data);
  uint8_t makeSendData(uint8_t * databuf[32]);// TODO 需要外部有至少32长度的空间
  
  // setter
  void setOperationDF(uint8_t _sub_type, uint32_t operation_1, 
					uint32_t operation_2, uint32_t operation_3);
  void setDataDF(uint8_t _sub_type, uint32_t operation_1, 
					uint32_t operation_2, uint32_t operation_3);
  void setDataByFormatByteStream(uint8_t * byte_stream);// 已经去掉转义的字节流
  void setDataByOriginByteStream(uint8_t * ori_byte_stream, int len);//7e-7e未去掉转义字符，需要长度
  // getter
  uint32_t getValueInt(uint8_t index);
  double getValueDouble(uint8_t index);
  
  
private:
  // total length is: 17. 
  uint8_t start[2];
	
  uint8_t msg_type; // 1 for data, 2 for operation [0]
	// payload
  uint8_t sub_type;								// [1]
  uint8_t data1[4];								// [2][3][4][5]
  uint8_t data2[4];								// [6][7][8][9]
  uint8_t data3[4];								// [10][11][12][13]
   
  uint8_t end;
};

#endif
