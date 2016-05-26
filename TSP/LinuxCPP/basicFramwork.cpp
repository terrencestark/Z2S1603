#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#include <pthread.h> //-lpthread
#include <malloc.h>
#include "TSP.h"

#define FALSE -1
#define TRUE 0

int UART_Open(int fd,char* port);
void UART_Close(int fd);
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART_Init(int fd, int speed,int flow_ctrlint ,int databits,int stopbits,char parity);
int UART_Recv(int fd, char *rcv_buf,int data_len);
int UART_Send(int fd, char *send_buf,int data_len);

/*****************************************************************
* 名称： UART0_Open
* 功能： 打开串口并返回串口设备文件描述
* 入口参数： fd :文件描述符 port :串口号(ttyS0,ttyS1,ttyS2)
* 出口参数： 正确返回为1，错误返回为0
*****************************************************************/
int UART_Open(int fd,char* port)
{

	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (FALSE == fd){
		perror("Can't Open Serial Port");
		return(FALSE);
	}

	//判断串口的状态是否为阻塞状态
	if(fcntl(fd, F_SETFL, 0) < 0){
		printf("fcntl failed!\n");
		return(FALSE);
	} else {
		//    printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	}

	//测试是否为终端设备
	if(0 == isatty(STDIN_FILENO)){
		printf("standard input is not a terminal device\n");
		return(FALSE);
	}

	return fd;
}

void UART_Close(int fd)
{
	close(fd);
}

/*******************************************************************
* 名称： UART0_Set
* 功能： 设置串口数据位，停止位和效验位
* 入口参数： fd 串口文件描述符
* speed 串口速度
* flow_ctrl 数据流控制
* databits 数据位 取值为 7 或者8
* stopbits 停止位 取值为 1 或者2
* parity 效验类型 取值为N,E,O,,S
*出口参数： 正确返回为1，错误返回为0
*******************************************************************/
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
	int i;
	
	int speed_arr[] = { 
		B38400, B19200, B9600, B4800, B2400, B1200, B300,
		B38400, B19200, B9600, B4800, B2400, B1200, B300
	};
	int name_arr[] = {
		38400, 19200, 9600, 4800, 2400, 1200, 300, 38400,
		19200, 9600, 4800, 2400, 1200, 300
	};
	struct termios options;

/*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数,还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
*/
	if(tcgetattr( fd,&options) != 0){
		perror("SetupSerial 1");
		return(FALSE);
	}
    
	//设置串口输入波特率和输出波特率
	for(i= 0;i < sizeof(speed_arr) / sizeof(int);i++) {     
		if (speed == name_arr[i]) {
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}    
	//修改控制模式，保证程序不会占用串口        
	options.c_cflag |= CLOCAL;
	//修改控制模式，使得能够从串口中读取输入数据
	options.c_cflag |= CREAD;
	//设置数据流控制
	switch(flow_ctrl){
	case 0 : //不使用流控制
		options.c_cflag &= ~CRTSCTS;
		break;    
	case 1 : //使用硬件流控制
		options.c_cflag |= CRTSCTS;
		break;
	case 2 : //使用软件流控制
		options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	}
	//设置数据位
	options.c_cflag &= ~CSIZE; //屏蔽其他标志位
	switch (databits){
	case 5 :
		options.c_cflag |= CS5;
		break;
	case 6 :
		options.c_cflag |= CS6;
		break;
        case 7  :
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (FALSE);
	}
	//设置校验位
	switch (parity) {
	case 'n':
	case 'N': //无奇偶校验位。
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O': //设置为奇校验
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E': //设置为偶校验
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S': //设置为空格
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
	}
	// 设置停止位
	switch (stopbits){
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return (FALSE);
	}
	//修改输出模式，原始数据输出
	options.c_oflag &= ~OPOST;
	//设置等待时间和最小接收字符
	options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
	options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */

	//如果发生数据溢出，接收数据，但是不再读取
	tcflush(fd,TCIFLUSH);

	//激活配置 (将修改后的termios数据设置到串口中）
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("com set error!/n");
		return (FALSE);
	}
	return (TRUE);
}


int UART_Init(int fd, int speed,int flow_ctrlint ,int databits,int stopbits,char parity)
{
	//设置串口数据帧格式
	if (FALSE == UART_Set(fd,speed,flow_ctrlint,databits,stopbits,parity)) {         
		return FALSE;
	} else {
		return TRUE;
	}
}



/*******************************************************************
* 名称： UART0_Recv
* 功能： 接收串口数据
* 入口参数： fd :文件描述符
* rcv_buf :接收串口中数据存入rcv_buf缓冲区中
* data_len :一帧数据的长度
* 出口参数： 正确返回为1，错误返回为0
*******************************************************************/
int UART_Recv(int fd, char *rcv_buf,int data_len)
{
	int len,fs_sel;
	fd_set fs_read;

	struct timeval time;
	
	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);

	time.tv_sec = 10;
	time.tv_usec = 0;

	//使用select实现串口的多路通信
	fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
	if(fs_sel){
		len = read(fd,rcv_buf,data_len);    
		return len;
	} else {
		return FALSE;
	}    
}

/*******************************************************************
* 名称： UART0_Send
* 功能： 发送数据
* 入口参数： fd :文件描述符
* send_buf :存放串口发送数据
* data_len :一帧数据的个数
* 出口参数： 正确返回为1，错误返回为0
*******************************************************************/
int UART_Send(int fd, char *send_buf,int data_len)
{
	int ret;

	ret = write(fd,send_buf,data_len);
	if (data_len == ret ){    
 		return ret;
	} else {
		tcflush(fd,TCOFLUSH);
		return FALSE;  
	}
}
/*
 * VAR DECLARATIONS - TSP
*/
int fd;  // global file descriptor
DataFrame df_send;
DataFrame df_rcev;

pthread_t mythread;
bool sendSwitch = true;
bool isEmergency = false;
uint8_t * send_data_buf;

const int SEND_CYCLE = 5;//5 round one send

/*
 * FUNCTION DECLARATIONS - TSP
*/
void handle_new_rcevDF(DataFrame new_rcev_df);
void *timer_function(void *arg);
void set_send_DF(DataFrame *_send_DF);
void send_DFdata_to_serial(DataFrame *_send_DF);

void final_works();
/*******************************************************************
* 名称： Main 函数
* 功能： 听端口，解析封包
*******************************************************************/
int main(int argc, char **argv)
{

	fd = FALSE;             
	int ret;                          
	char rcv_buf[512];
	int i;
	if(argc != 2){
		printf("Usage: %s /dev/ttySn \n",argv[0]);    
		return FALSE;    
	}
	fd = UART_Open(fd,argv[1]);
	if(FALSE == fd){    
 		printf("open error\n");    
 		exit(1);     
	}
	ret = UART_Init(fd,9600,0,8,1,'N');
	if (FALSE == fd){    
 		printf("Set Port Error\n");    
 		exit(1);
	}
	// thread setup PART - TSP
	int err;
	err = pthread_create(&mythread, NULL, timer_function, NULL);
	if(err != 0){
		printf("create send thread ERROR.");
	}
	// RECEIVE PART - TSP
	int start_flag = 0;
	int end_flag = 0;
	int data_index = 0;
	unsigned char data_stream[9]; 
	memset(rcv_buf,0,sizeof(rcv_buf));
	// *****TSP framework vars
	uint8_t rcv_data_buf[64];
	send_data_buf = (uint8_t *)malloc(sizeof(uint8_t)*32);

	bool isMetStart = false;
	int rcv_pointer = 0;

	//for(i=0;;i++)
	while(1)
	{
 		ret = UART_Recv(fd, rcv_buf,1);//1 byte per time
		if( ret > 0){
			rcv_buf[ret]='\0';
			//recv data one by one
			unsigned char inByte = rcv_buf[0] & 0xff;
			// ***** Handle part *****
			if(inByte == 0x7e){
				if(isMetStart==false){
					isMetStart = true;
					rcv_pointer = 0;
					rcv_data_buf[rcv_pointer]=inByte;
					rcv_pointer++;
				}else if(isMetStart==true){
					isMetStart = false;
					rcv_data_buf[rcv_pointer]=inByte;
					rcv_pointer++;
					int ori_frame_len = rcv_pointer;
					df_rcev.setDataByOriginByteStream(rcv_data_buf, ori_frame_len);
					handle_new_rcevDF(df_rcev);
				}
			}else{
				if(isMetStart==false){
				}else{
					rcv_data_buf[rcv_pointer]=inByte;
        				rcv_pointer++;
				}
			}


			// ***** Handle part END*****
 		} else {    
 			printf("cannot receive data!\n");    
        		break;
		}
 		if(ret == 0x99)
			break;
	}// end of for

	UART_Close(fd);
	final_works();
	return 0;
}//main

/*******************************************************************
* 名称： handle_new_rcevDF
* 功能： 处理解析好的封包
*******************************************************************/
void handle_new_rcevDF(DataFrame new_rcev_df){
	// example: read data3 of new data
	int iValue = new_rcev_df.getValueInt(3);
	double dValue = new_rcev_df.getValueDouble(1);
	printf("get new dataFrame, msgT: %d valued: %.2f value:%d\n", new_rcev_df.msg_type, dValue, iValue);
}

/*******************************************************************
* 名称： timer_function
* 功能： 处理解析好的封包
*******************************************************************/
void *timer_function(void *arg){
	sendSwitch = true;
	isEmergency = false;
	int cnt = SEND_CYCLE;
	while(1){
		sleep(1);
		set_send_DF(&df_send);
		if(sendSwitch == false)
			break;
		if(isEmergency==true || cnt<1){
			cnt = SEND_CYCLE;
			send_DFdata_to_serial(&df_send);
		}else{
			cnt--;
		}
	}
}

/*******************************************************************
* 名称： set_send_DF
* 功能： 设定一个封包
*******************************************************************/
void set_send_DF(DataFrame *_send_DF){
  uint32_t value = (*_send_DF).double2int32(66.6);
  (*_send_DF).setDataDF(1, value, 18, 22);
}
/*******************************************************************
* 名称： send_DFdata_to_serial
* 功能： 发送一个封包
*******************************************************************/
void send_DFdata_to_serial(DataFrame *_send_DF){
  int ds_len = (*_send_DF).makeSendData(&send_data_buf);
  char send_buf[ds_len];
  for(int i=0;i<ds_len;i++)
	send_buf[i]=(char)send_data_buf[i];
  //printf("TEST:send data. len: %d\n", ds_len);
  UART_Send(fd,send_buf, ds_len); // send
}

void final_works(){
  free(send_data_buf);
}
