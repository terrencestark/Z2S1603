# -*- coding: utf-8 -*-

class DataFrame:

    def __init__(self):
        data = [0x7e, 0x33]     #[0][1]
        self.start = bytearray(data)

        self.msg_type = 0            #[2]
        self.sub_type = 0            #[3]
        
        data = [0,0,0,0]
        self.data1 = bytearray(data) #[4][5][6][7]
        self.data2 = bytearray(data) #[8][9][10][11]
        self.data3 = bytearray(data) #[12][13][14][15]
        
        data = 0x7e        
        self.end = data              #[16]
    
    def setDataByFormatByteSream(self, _mid_byte_stream):
        self.msg_type = _mid_byte_stream[2]
        self.sub_type = _mid_byte_stream[3]
        self.data1 = _mid_byte_stream[4:8]
        self.data2 = _mid_byte_stream[8:12]
        self.data3 = _mid_byte_stream[12:16]
        self.end = _mid_byte_stream[16]
        
    def setDataByByteStream(self, rcv_data_buf):
        result = False
        if(len(rcv_data_buf)<17):
            return result
        elif rcv_data_buf[0]!=0x7e or rcv_data_buf[1]!=0x33:
            return result
        else:
            mid_byte_stream = bytearray()
            trans_flag = False
            for inbyte in rcv_data_buf:
                if inbyte == 0x7D:
                    trans_flag = True
                elif inbyte == 0x5E or inbyte == 0x5D:
                    if trans_flag == True:
                        mid_byte_stream.append(inbyte ^ 0x20)
                    else:
                        mid_byte_stream.append(inbyte)
                    trans_flag = False
                else:
                    mid_byte_stream.append(inbyte)
                    trans_flag = False
            # end of transfer
            if len(mid_byte_stream) == 17:
                print "get a right frame" #TS.Test
                self.setDataByFormatByteSream(mid_byte_stream)
                return True
    
    def getValueInt(self, _index):
        intValue = 0
        if _index ==1:
            data = self.data1
        elif _index==2:
            data = self.data2
        elif _index==3:
            data = self.data3
        else:
            return 0
        for i in range(0,4):
            intValue = intValue<<8
            intValue = intValue + data[3-i]
        return intValue
        
    def double2int32(self, dValue):
        result = int(dValue*100//1)
        return result
    
    #frame setter
    def setDataDF(self, _sub_type, _data_1, _data_2, _data_3):
        self.msg_type = 1;
        self.sub_type = _sub_type
        for i in range(0,4):
            self.data1[i] = (_data_1>>(8*i)) & 0x0FF
            self.data2[i] = (_data_2>>(8*i)) & 0x0FF
            self.data3[i] = (_data_3>>(8*i)) & 0x0FF
    
    def setOperationDF(self, _sub_type, _data_1, _data_2, _data_3):
        self.msg_type = 2;
        self.sub_type = _sub_type
        for i in range(0,4):
            self.data1[i] = (_data_1>>(8*i)) & 0x0FF
            self.data2[i] = (_data_2>>(8*i)) & 0x0FF
            self.data3[i] = (_data_3>>(8*i)) & 0x0FF
    
    def makeSendData(self):
        send_data_stream = bytearray()
        for i in range(0,2):
            send_data_stream.append(self.start[i])
        send_data_stream.append(self.msg_type)
        send_data_stream.append(self.sub_type)
        for i in range(0,4):
            send_data_stream.append(self.data1[i])
        for i in range(0,4):
            send_data_stream.append(self.data2[i])
        for i in range(0,4):
            send_data_stream.append(self.data3[i])
        send_data_stream.append(self.end)
        #ts test
        '''        
        for i in range(0, len(send_data_stream)):
            print ('%x' %send_data_stream[i] )
        '''
        return send_data_stream