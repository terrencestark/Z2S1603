'''conn via socket on Unix - Server get data'''
import TSP
import time
import thread
import socket
import os

# setup
sock_rcv_df = TSP.DataFrame()

def sock_handle_new_rcevDF(_df):
    '''when get a new frame from serial port do sth  
    '''
    iValue = _df.getValueInt(3)
    print ('Python get DF -  data 3 value: %d' %iValue)

def timer_readSocketData(interval, none):

    server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    if os.path.exists("/tmp/test.sock"):
        os.unlink("/tmp/test.sock")
    server.bind("/tmp/test.sock")
    server.listen(0)
    while True:
        connection, address = server.accept()
        # get data from socket
        getstr = connection.recv(1024)
        byteStream = bytes(getstr)
	streamlen = len(byteStream)
        # set parameters
        sock_isMetStart = False
        sock_rcv_pointer = 0
        sock_rcv_data_buf = bytearray()
	for i in range(0,streamlen):
            hexdata = ord(byteStream[i])
            # print("%x" % hexdata) # for test
            if(hexdata == 0x7e):
                if sock_isMetStart == False:
                    sock_isMetStart = True
                    sock_rcv_pointer = 0
                    sock_rcv_data_buf = bytearray()
                    sock_rcv_data_buf.append(hexdata)
                    sock_rcv_pointer +=1
                elif sock_isMetStart == True:
                    sock_isMetStart = False
                    sock_rcv_data_buf.append(hexdata)
                    sock_rcv_pointer+=1
                    frame_len = sock_rcv_pointer
                    #set data
                    sock_rcv_df.setDataByByteStream(sock_rcv_data_buf)
                    #handle data
                    sock_handle_new_rcevDF(sock_rcv_df)
            else:
                if sock_isMetStart==False:
                    # donothing
                    sock_isMetStart==False
                else:
                    sock_rcv_data_buf.append(hexdata)
                    sock_rcv_pointer+=1  
        #one socket stream read finished
        #connection.send("test: %s"% getstr) # for test
    connection.close()

if __name__=='__main__':
    t1 = thread.start_new_thread(timer_readSocketData, (1,1)) #(1,1) are parameters
    # to wait thread
    while True:
        i = 0

