# -*- coding: utf-8 -*-
import TSP
import serial
import time
import thread

# setup
rcv_df = TSP.DataFrame()
send_df = TSP.DataFrame()

s_port = serial.Serial('COM3', 9600)

# getdata
rcv_flag = True
isMetStart = False

rcv_pointer = 0
rcv_data_buf = bytearray()

# pre-define functions in script
def handle_new_rcevDF(_df):
    '''when get a new frame from serial port do sth  
    '''
    iValue = _df.getValueInt(3)
    print ('Python get DF - data 3 value: %d' %iValue)


def set_send_DF(_df):
    '''fresh sensor value here  
    '''
    Value = _df.double2int32(18.7)
    _df.setDataDF(1, Value, 11, 22)

def send_DFdata_to_serial(_df):
    send_data_buf = bytearray()
    send_data_buf = _df.makeSendData()
    sendbyteStream = bytearray(send_data_buf)
    s_port.write(sendbyteStream)
    #print ('send data>>>: %x' %bytevalue)#TS.Test 
    #print len(sendbyteStream)#TS.Test 
    
sendSwitch = True
isEmergency = False
def timer_function(interval, none):
    if sendSwitch == False:
        return None
    else:
        cnt = 5
        while True:
            # fresh sensor value
            set_send_DF(send_df);
            time.sleep(interval)
            # send
            if(isEmergency == True or cnt<1):
                cnt = 5
                send_DFdata_to_serial(send_df)
            else:
                cnt -=1
            
thread.start_new_thread(timer_function, (1,1))        

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
                handle_new_rcevDF(rcv_df)
        else:
            if isMetStart==False:
                # donothing
                isMetStart==False
            else:
                rcv_data_buf.append(hexdata)
                rcv_pointer+=1                
                
#final
s_port.close()