# -*- coding: utf-8 -*-
import TSP
import serial
import socket
import time
import thread
import sys
import MySQLdb

# setup
rcv_df = TSP.DataFrame()
send_df = TSP.DataFrame()

# getdata
rcv_flag = True
isMetStart = False

rcv_pointer = 0
rcv_data_buf = bytearray()

def GetNowTime():
    return time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time()))
    
# pre-define functions in script
def handle_new_rcevDF(_df, _cur, _conn):
    '''when get a new frame from serial port do sth  
    '''
    iValue = _df.getValueInt(1)
    dValue = iValue/100.0
    str_temp = '%.2f' % dValue
    str_t = GetNowTime()
    cmdstr = "delete from temprature where id>0;"
    _cur.execute(cmdstr)
    cmdstr = "insert into temprature (id,value,time) values(" + "1"+ ","+str_temp+",'"+str_t + "');"
    print cmdstr
    _cur.execute(cmdstr)
    _conn.commit()
    #_cur.commit()

def timre_socket_listen(_server, none):
    while True:
        connection,address = _server.accept()
        print "get a connection from PHP socket"
        buf = connection.recv(1024)
        connection.close()
        PHP_data_buf = bytearray()
        for i in range(0, len(buf)):
            print ("%x" % ord(buf[i]))  #for test
            PHP_data_buf.append(ord(buf[i]))
        #7e subtype d1 d2 d3 7e
        subtype = PHP_data_buf[1]
        data1 = PHP_data_buf[2]
        data2 = PHP_data_buf[3]
        data3 = PHP_data_buf[4]
        send_df.setDataDF(subtype, data1, data2, data3)
        
        send_data_buf = bytearray()
        send_data_buf = send_df.makeSendData()
        sendbyteStream = bytearray(send_data_buf)
        #for i in range(0, len(sendbyteStream)):
        #    print ("%x" % sendbyteStream[i])  #for test
        s_port.write(sendbyteStream)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print "[use] like: ./program /dev/ttyUSB0"
        exit()
    
    #zigbee 通信端口
    port_name = sys.argv[1]
    s_port = serial.Serial(port_name, 9600)
    
    #PHP监听端口
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(('localhost', 6666))
    server.listen(5)
    
    #连接数据库
    try:
        conn=MySQLdb.connect(host='localhost',user='root',passwd='123123',db='zdata',port=3306)
        cur=conn.cursor()
    except MySQLdb.Error,e:
        print "mysql DB connect failed."
        conn.close()
        
    #thread.start_new_thread(timer_function, (1,1)) #原来是定时发送
    #PHP触发的发送
    thread.start_new_thread(timre_socket_listen, (server,1))            

    #监听端口发送的消息
    while rcv_flag:
        if s_port.inWaiting() > 0:
            hexdata = ord(s_port.read(1))
            print ('get data %x' % hexdata)#TS.Test
            if(hexdata == 0x7e):
                if isMetStart == False:
                    isMetStart = True
                    rcv_pointer = 0
                    rcv_data_buf = bytearray()
                    rcv_data_buf.append(hexdata)
                    rcv_pointer +=1
                elif isMetStart == True:
                    isMetStart = False
                    rcv_data_buf.append(hexdata)
                    rcv_pointer+=1
                    frame_len = rcv_pointer
                    #set data
                    rcv_df.setDataByByteStream(rcv_data_buf)
                    #handle data
                    handle_new_rcevDF(rcv_df, cur, conn)
            else:
                if isMetStart==False:
                    # donothing
                    isMetStart==False
                else:
                    rcv_data_buf.append(hexdata)
                    rcv_pointer+=1                             
    #final
    s_port.close()
    cur.close()
    conn.close()