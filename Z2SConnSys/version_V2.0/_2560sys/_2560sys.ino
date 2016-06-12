#include <TSP.h>
#include <malloc.h> 

#define S_OFF false
#define S_ON  true

// data frame
DataFrame df_rcev;
DataFrame df_send;

// read var
uint8_t rcv_data_buf[64];
int rcv_pointer;
boolean isMetStart;

// send var
uint8_t * send_data_buf;//buffer

// control var
//// PIN
const int P_led = 3;
const int P_door =  4;
const int P_relay = 5;

//// VAR
//// 's' means switch, 'st' means subType
boolean s_light        = S_OFF;      // subType 11
const int st_light     = 11;
boolean s_door         = S_OFF;      // subType 12
const int st_door      = 12;
boolean s_relay        = S_OFF;      // subType 13
const int st_relay     = 13;
boolean s_security     = S_OFF;      // subType 14
const int st_security  = 14;
boolean s_auto_ac      = S_OFF;      // subType 15
const int st_auto_ac   = 15;

int allertHigh = 30;
int allertLow = 18;

// functions declarations
void handle_new_rcevDF(DataFrame new_rcev_df);

void set_send_DF(DataFrame *_send_DF);
void send_DFdata_to_serial(DataFrame *_send_DF);
boolean ifSendBackPacket = false;

void ctrl_light(DataFrame _df);
void ctrl_door(DataFrame _df);
void ctrl_relay(DataFrame _df);
void ctrl_security(DataFrame _df);
void ctrl_auto_ac(DataFrame _df);
void ctrl_ac_cold(DataFrame _df);
void ctrl_ac_hot(DataFrame _df);
void ctrl_ac_off(DataFrame _df);
void ctrl_ac_adjust(DataFrame _df);

void final_works();

// IR vars
uint8_t addr = 0x00;
uint8_t irdata = 0x00;
const int IR_S =  8;
const int IR_SEND_COUNT = 3;

const int st_cold   = 1;
const int st_hot    = 2;
const int st_off    = 3;
const int st_adjust = 4;

int alert_low  = 18;  // init value
int alert_high = 28;  // init value

//IR funcs
void transANumberViaIR(int num);
void IR_Send38KHZ(int x,int y);
void IR_Sendcode(uint8_t x);

void setup(){ 
  
  Serial.begin(9600); 
  Serial2.begin(9600);
  // init works
  //// receive and send
  rcv_pointer = 0;
  isMetStart = false;
  
  send_data_buf = (uint8_t *)malloc(sizeof(uint8_t)*32);

  //// controls
  pinMode(13, OUTPUT); 
  digitalWrite(13, LOW);
  pinMode(P_door, OUTPUT); 
  digitalWrite(P_door, LOW);
  pinMode(P_led, OUTPUT); 
  digitalWrite(P_led, LOW);
  pinMode(P_relay, OUTPUT); 
  digitalWrite(P_relay, LOW);
  
  pinMode(IR_S, OUTPUT);//if not set will send wrong NEC
}

void loop(){
  // receive data from serial prot
  if (Serial2.available() > 0){
    uint8_t inByte = Serial2.read();
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
  }//byte handle
}

boolean blinkFlag = true; // used to show 'get data'
// handle a new data
void handle_new_rcevDF(DataFrame new_rcev_df){
  int iValue = new_rcev_df.getValueInt(3);
  if(iValue==0x66){//default data3 = 0x66
    if(blinkFlag==true){
      digitalWrite(13, HIGH);
      blinkFlag = false;
    }else{
      blinkFlag = true;
      digitalWrite(13, LOW);
    }
  }//change 13led
  // test
    Serial.print("get subtype:");
    Serial.println(new_rcev_df.sub_type);
  // end of test
  switch(new_rcev_df.sub_type){
    // IR and Aircondition
    case 1:
      ctrl_ac_cold(new_rcev_df);
      break;
    case 2:
      ctrl_ac_hot(new_rcev_df);
      break;
    case 3:
      ctrl_ac_off(new_rcev_df);
      break;
    case 4:
      ctrl_ac_adjust(new_rcev_df);
      break;
    // control
    case 11:
      ctrl_light(new_rcev_df);
      break;
    case 12:
      ctrl_door(new_rcev_df);
      break;
    case 13:
      ctrl_relay(new_rcev_df);
      break;
    case 14:
      ctrl_security(new_rcev_df);
      break;
    case 15:
      ctrl_auto_ac(new_rcev_df);
      break;
    default:
      break;
  }
  if(ifSendBackPacket==true){
    send_DFdata_to_serial(&df_send);
    ifSendBackPacket=false;
  } 
}


// final works - maybe not be executed
void final_works(){
}

// IR functions
void transDataViaIR(int addr, int data){
  addr = (uint8_t)(addr&0xff);
  irdata = (uint8_t)(data&0xff);
  
  uint8_t temp = irdata;
  uint8_t dat;
  for(int i=0;i<IR_SEND_COUNT;i++){
    IR_Send38KHZ(280,1);//发送9ms的起始码
    IR_Send38KHZ(140,0);//发送4.5ms的结果码
     
    IR_Sendcode(addr);//用户识别码
    dat=~addr;
    IR_Sendcode(dat);//用户识别码反吗
     
    IR_Sendcode(temp);//操作码
    dat=~temp;
    IR_Sendcode(dat);//操作码反码
     
    IR_Send38KHZ(21,1);//发送结束码
    delay(200);
  }  
}

// IR remote
void IR_Send38KHZ(int x,int y) //产生38KHZ红外脉冲
{ 
  for(int i=0;i<x;i++)//15=386US
  { 
    if(y==1)
    {
      digitalWrite(IR_S,1);
      delayMicroseconds(9);
      digitalWrite(IR_S,0);
      delayMicroseconds(9);
    }else{
      digitalWrite(IR_S,0);
      delayMicroseconds(20);
    }            
  }
}

void IR_Sendcode(uint8_t x)
{
  for(int i=0;i<8;i++)
  {
    if((x&0x01)==0x01)
    {
      IR_Send38KHZ(23,1);
      IR_Send38KHZ(64,0);             
    }else{
      IR_Send38KHZ(23,1);
      IR_Send38KHZ(21,0);  
    }
    x=x>>1;
  }  
}

// send back related functions
// set a data frame to send
void set_send_DF(DataFrame *_send_DF, int subType, int _status){
  (*_send_DF).setDataDF(subType, _status, 0, 0x77);
   // send back dataFrame default data3 = 0x03 
}
void set_send_DF(DataFrame *_send_DF, int subType, int _status, int data2, int data3){
  (*_send_DF).setDataDF(subType, _status, data2, data3);
   // send back dataFrame default data3 = 0x03 
}

// send work for data frame
void send_DFdata_to_serial(DataFrame *_send_DF){
  int ds_len = (*_send_DF).makeSendData(&send_data_buf);
  Serial2.write(send_data_buf, ds_len); // send
}

// control functions
void ctrl_light(DataFrame _df){
  Serial.println(">>>control led");
  int iValue = _df.getValueInt(1);
  if(iValue==1){
    s_light = S_ON;
    digitalWrite(P_led, HIGH);
  }else{
    s_light = S_OFF;
    digitalWrite(P_led, LOW);
  }
  set_send_DF(&df_send, st_light, (int)s_light);
  ifSendBackPacket = true;  
}

void ctrl_door(DataFrame _df){
  Serial.println(">>>control door");
  int iValue = _df.getValueInt(1);
  if(iValue==1){
    s_door = S_ON;
    digitalWrite(P_door, HIGH);
  }else{
    s_door = S_OFF;
    digitalWrite(P_door, LOW);
  } 
  set_send_DF(&df_send, st_door, (int)s_door);
  ifSendBackPacket = true;  
}

void ctrl_relay(DataFrame _df){
  Serial.println(">>>control relay");
  int iValue = _df.getValueInt(1);
  if(iValue==1){
    s_relay = S_ON;
    digitalWrite(P_relay, HIGH);
  }else{
    s_relay = S_OFF;
    digitalWrite(P_relay, LOW);
  }
  set_send_DF(&df_send, st_relay, (int)s_relay);
  ifSendBackPacket = true;  
}

void ctrl_security(DataFrame _df){
  Serial.println(">>>control security");
  int iValue = _df.getValueInt(1);
  if(iValue==1){
    s_security = S_ON;
    // TODO
  }else{
    s_security = S_OFF;
    // TODO
  }
  set_send_DF(&df_send, st_security, (int)s_security);
  ifSendBackPacket = true; 
}

void ctrl_auto_ac(DataFrame _df){
  Serial.println(">>>control auto ac");
  int iValue = _df.getValueInt(1);
  if(iValue==1){
    s_auto_ac = S_ON;
    // TODO  send open signal
    //// use transDataViaIR(cmd, data) function
  }else{
    s_auto_ac = S_OFF;
    // TODO
  
  }
  set_send_DF(&df_send, st_auto_ac, (int)s_auto_ac);
  ifSendBackPacket = true; 
}

void ctrl_ac_cold(DataFrame _df){
  Serial.println(">>>control ac cold");
  int iValue = _df.getValueInt(1);
  // TODO  send open signal
  //// use transDataViaIR(cmd, data) function
  set_send_DF(&df_send, st_cold, 1, iValue, 0);
  ifSendBackPacket = true; 
}
void ctrl_ac_hot(DataFrame _df){
  Serial.println(">>>control ac hot");
  int iValue = _df.getValueInt(1);
  // TODO  send open signal
  //// use transDataViaIR(cmd, data) function
  set_send_DF(&df_send, st_hot, 2, iValue, 0);
  ifSendBackPacket = true; 
}
void ctrl_ac_off(DataFrame _df){
  Serial.println(">>>control ac off");
  // TODO  send open signal
  //// use transDataViaIR(cmd, data) function
  set_send_DF(&df_send, st_off, 0);
  ifSendBackPacket = true;
}
void ctrl_ac_adjust(DataFrame _df){
  Serial.println(">>>control ac alter adjust");
  int iValue = _df.getValueInt(1);
  alert_low = iValue;
  iValue = _df.getValueInt(2);
  alert_high = iValue;
  set_send_DF(&df_send, st_adjust, 0, alert_low, alert_high);
  ifSendBackPacket = true;
}
