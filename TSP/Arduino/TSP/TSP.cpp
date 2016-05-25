#include <Arduino.h> 
#include "tsp.h"

/*
 * 公共函数
 */
char getChar(uint8_t data, int isHigh){
	char c;
	if(isHigh!=0){
		// high 4 bit
		c = (data>>4) & 0x0F;
	}else{
		c = data & 0x0F;
	}
	if((uint8_t)c < 10){
		c += 48;
	}else{
		c += 55;
	}
	return c;
}
int8_t getFrameString(uint8_t * data, int data_len, char * char_buf[96]){
	int strlen = 0;
	for(int i=0;i<data_len;i++){
		(*char_buf)[i*3] = getChar(data[i], 1);
		(*char_buf)[i*3+1] = getChar(data[i], 0);
		(*char_buf)[i*3+2] = ' ';
		strlen +=3;
	}
	(*char_buf)[strlen-1] = '\0';
	return strlen;
}
/*
 * 类内部
 */
DataFrame::DataFrame()
{
	//constructor
	start[0] = 0x7E;
	start[1] = 0x33;
	
	msg_type = 0; // 1 for data, 2 for operation
	// payload
	sub_type = 0;
	int i=0;
	for(i=0;i<4;i++){
		data1[i] = 0;
		data2[i] = 0;
		data3[i] = 0;
	}
	
   
	end = 0x7e;
}

uint32_t DataFrame::double2int32(double data){
	uint32_t result =0;
	result = data*100/1;
	return result;
}

/*
 * 设定操作
 */
void DataFrame::setOperationDF(uint8_t _sub_type, uint32_t operation_1, 
                      uint32_t operation_2, uint32_t operation_3){
	msg_type = 2;
	sub_type = _sub_type;
	int i;
	for(i=0; i<4; i++){
		data1[i]=(operation_1>>(i*8)) & 0xff;
		data2[i]=(operation_2>>(i*8)) & 0xff;
		data3[i]=(operation_3>>(i*8)) & 0xff;
	}
}


void DataFrame::setDataDF(uint8_t _sub_type, uint32_t data_1, 
					uint32_t data_2, uint32_t data_3){
	msg_type = 1;
	sub_type = _sub_type;
	int i;
	for(i=0; i<4; i++){
		data1[i]=(data_1>>(i*8)) & 0xff;
		data2[i]=(data_2>>(i*8)) & 0xff;
		data3[i]=(data_3>>(i*8)) & 0xff;
	}
}

void DataFrame::setDataByFormatByteStream(uint8_t * byte_stream){
	msg_type = byte_stream[2];
	sub_type = byte_stream[3];
	for(int i=0; i<4; i++){
		data1[i]= byte_stream[4+i];
		data2[i]= byte_stream[8+i];
		data3[i]= byte_stream[12+i];
	}
}

void DataFrame::setDataByOriginByteStream(uint8_t * ori_byte_stream, int len){
	// 起始验证
	if(ori_byte_stream[0]!=0x7E|| ori_byte_stream[1] != 0x33){
			return;
	}
	// rid off 转义
	uint8_t result_stream[17]; //V1.0版协议的总长度
	int aim_pointer = 0;
	int ori_pointer = 0;
	for(ori_pointer = 0; ori_pointer<len;){
		if(ori_byte_stream[ori_pointer]==0x7D){  //遇到7D
			if(ori_byte_stream[ori_pointer+1]==0x5E || ori_byte_stream[ori_pointer+1]==0x5D){
				result_stream[aim_pointer] = ori_byte_stream[ori_pointer+1] ^ 0x20;
				aim_pointer++;
				ori_pointer+=2;
			}else{
				result_stream[aim_pointer] = ori_byte_stream[ori_pointer];
				aim_pointer++;
				ori_pointer++;
			}
		}else{
			result_stream[aim_pointer] = ori_byte_stream[ori_pointer];
			aim_pointer++;
			ori_pointer++;
		}
	}
	if(aim_pointer==17){
		// 长度正确，解析完成
		setDataByFormatByteStream(result_stream);
	}
}


/*
 * 获取数据
 */	
uint32_t DataFrame::getValueInt(uint8_t index){
	uint32_t value=0;
	uint8_t * data;
	switch(index){
		case 1:
			data = data1;
			break;
		case 2:
			data = data2;
			break;
		case 3:
			data = data3;
			break;
		default:
			return 0;
	}
	int i;

	for(i=0;i<4;i++){
		value = value << 8;
		value += data[3-i] & 0xff;
	}

	return value;
}

/*
 * 获取数据
 */

double DataFrame::getValueDouble(uint8_t index){
	uint32_t tempValue = getValueInt(index);
	double result = tempValue/100.0;
	return result;
}

int pushFormatData(uint8_t ** aim_array, int index, uint8_t bytedata){
	int len = 0;
	if(bytedata==0x7E){
		(*aim_array)[index] = 0x7D;
		(*aim_array)[index+1] = 0x5E;
		len = 2;
	}else if(bytedata==0x7D){
		(*aim_array)[index] = 0x7D;
		(*aim_array)[index+1] = 0x5D;
		len = 2;
	}else{
		(*aim_array)[index] = bytedata;
		len = 1;
	}
	return len;
}
uint8_t DataFrame::makeSendData(uint8_t * databuf[32]){ // TODO 32 max
	//uint8_t senddata[32];
	uint8_t len = 0;
	//start
	(*databuf)[0] = start[0];
	(*databuf)[1] = start[1];
	len = 2;
	// payload
	
	int end_flag = 0;
	for(int i=2,j=0; end_flag<1;){
		int addLen = 0;
		if(j<1){
			addLen = pushFormatData(databuf, i, msg_type);
		}else if(j<2){
			addLen = pushFormatData(databuf, i, sub_type);
		}else if(j<6){
			addLen = pushFormatData(databuf, i, data1[j-2]);
		}else if(j<10){
			addLen = pushFormatData(databuf, i, data2[j-6]);
		}else if(j<14){
			addLen = pushFormatData(databuf, i, data3[j-10]);
			if(j == 13){
				end_flag = 1;
			}
		}
		j ++;
		i += addLen;
		len += addLen;
	}
	
	//end 
	(*databuf)[len] = end;
	len++;
	
	//result) = senddata;
	
	return len;	
}
