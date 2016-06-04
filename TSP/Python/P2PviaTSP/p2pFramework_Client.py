'''conn via socket on Unix - Server get data'''
import TSP
import time
import thread
import socket
import os

# setup
sock_send_df = TSP.DataFrame()

def set_sock_send_DF(_df):
    '''fresh sensor value here  
    '''
    Value = _df.double2int32(18.7)
    _df.setDataDF(1, Value, 11, 22)

def send_sock_DFdata_via_socket(_df):
    client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    client.connect("/tmp/test.sock")

    send_data_buf = bytearray()
    send_data_buf = _df.makeSendData()
    for i in range(0,len(send_data_buf)):
        print ("%x" % send_data_buf[i])
    client.send(send_data_buf)

    #print client.recv(1024)  #for test
    client.close()

if __name__=='__main__':
    set_sock_send_DF(sock_send_df)
    send_sock_DFdata_via_socket(sock_send_df)



