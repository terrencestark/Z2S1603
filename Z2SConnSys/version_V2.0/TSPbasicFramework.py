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
       In version 2.0 received DataFrame are all sensor status.
    '''
    str_table = 'dev_status'
    str_dataitem = ''
    str_aimsubtype = '%d' % _df.sub_type
    print 'TEST:get datatype: %d' % _df.sub_type
    # remenber ','
    str_status = '0'  # data1 
    str_data2 = '' 
    str_data3 = ''
    flag = True
    if _df.sub_type == 1 or _df.sub_type ==2:
        str_aimsubtype = '1'
        str_dataitem = "subtype,status,data2"
        iValue = _df.getValueInt(1)
        str_status = ',%d' % iValue
        iValue = _df.getValueInt(2)
        str_data2 = ',%d' % iValue
    elif _df.sub_type == 3:
        str_aimsubtype = '1'
        str_dataitem = "subtype,status"
        iValue = _df.getValueInt(1)
        str_status = ',%d' % iValue
    elif _df.sub_type == 4:
        str_aimsubtype = '2'
        str_dataitem = "subtype,status,data2,data3"
        iValue = _df.getValueInt(1)
        str_status = ',%d' % iValue
        iValue = _df.getValueInt(2)
        str_data2 = ',%d' % iValue
        iValue = _df.getValueInt(3)
        str_data3 = ',%d' % iValue
    elif _df.sub_type >= 11:
        str_dataitem = "subtype,status"
        iValue = _df.getValueInt(1)
        str_status = ',%d' % iValue
    else:
        print "unknown data type"
        flag = False
    if flag == True:  
        str_t = GetNowTime()
        cmdstr = "DELETE FROM "+ str_table +" WHERE subtype="+ str_aimsubtype+";"
        _cur.execute(cmdstr)
        cmdstr = "INSERT INTO "+ str_table +" ("+ str_dataitem +",time) values(" + str_aimsubtype + str_status + str_data2 + str_data3 + ",'"+str_t+"');"
        print cmdstr
        _cur.execute(cmdstr)
        _conn.commit()

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
        conn=MySQLdb.connect(host='localhost',user='root',passwd='123123',db='z2sdb',port=3306)
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
            #print ('get data %x' % hexdata)#TS.Test
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