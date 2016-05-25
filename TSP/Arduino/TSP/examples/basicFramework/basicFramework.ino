#include <TSP.h>
#include <malloc.h> 
#include <MsTimer2.h> 

// data frame
DataFrame df_send;
DataFrame df_rcev;
// read var
uint8_t rcv_data_buf[64];
int rcv_pointer;
boolean isMetStart;
// send var
boolean sendSwitch;
boolean isEmergency;
int sendCounter;
const int SEND_CYCLE = 5; //5 * timer send once
uint8_t * send_data_buf;//buffer
// sensor data
double temprature = 0.0;
// control var
int light = 0;

// functions declarations
void handle_new_rcevDF(DataFrame new_rcev_df);
void timer_function();
void set_send_DF(DataFrame *_send_DF);
void send_DFdata_to_serial(DataFrame *_send_DF);

void final_works();

void setup(){ 
  Serial.begin(9600); 
  // init works
  //// receive and send
  rcv_pointer = 0;
  isMetStart = false;
  sendSwitch = true;
  isEmergency = false; //if have emergency, send ASAP
  sendCounter = SEND_CYCLE;
  send_data_buf = (uint8_t *)malloc(sizeof(uint8_t)*32);
  //// sensors
  temprature = 0.0;
  //// controls
  light = 0;
  
  // timer init
  MsTimer2::set(1000, timer_function); //1000ms once
  MsTimer2::start(); 
}

void loop(){
  // receive data from serial prot
  if (Serial.available() > 0){
    uint8_t inByte = Serial.read();
    if(inByte == 0x7e){
      // get a 7e
      if(isMetStart==false){
        //get a start of a frame
        isMetStart = true;
        rcv_pointer = 0;
        rcv_data_buf[rcv_pointer]=inByte;
        rcv_pointer++;
      }else if(isMetStart==true){
        //get a end of a frame
        isMetStart = false;
        rcv_data_buf[rcv_pointer]=inByte;
        rcv_pointer++;
        int ori_frame_len = rcv_pointer;
        df_rcev.setDataByOriginByteStream(rcv_data_buf, ori_frame_len);
        handle_new_rcevDF(df_rcev);// handle a new data
      }
    }else{
      //not 7e
      if(isMetStart==false){
        // do nothing
      }else{
        rcv_data_buf[rcv_pointer]=inByte;
        rcv_pointer++;
      }
    }
  }
}

// handle a new data
void handle_new_rcevDF(DataFrame new_rcev_df){
  // example: read data3 of new data
  int iValue = new_rcev_df.getValueInt(3);
  Serial.println(iValue);
}

// timer function
void timer_function(){
  if(sendSwitch == false)
    return;

  // do something in every timer
  // ...for example set a data frame
  set_send_DF(&df_send);
  // send data per SEND_CYCLE
  if(isEmergency==true || sendCounter<1){
    sendCounter = SEND_CYCLE;
    send_DFdata_to_serial(&df_send);
  }else{
    sendCounter--;
  }
}
// set a data frame to send
void set_send_DF(DataFrame *_send_DF){
  uint32_t value = (*_send_DF).double2int32(26.5);
  (*_send_DF).setDataDF(1, value, 30, 35);
}
// send work for data frame
void send_DFdata_to_serial(DataFrame *_send_DF){
  int ds_len = (*_send_DF).makeSendData(&send_data_buf);
  Serial.write(send_data_buf, ds_len); // send
}

// final works - maybe not be executed
void final_works(){
  free(send_data_buf);
}

