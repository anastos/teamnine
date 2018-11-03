#include <Wire.h>

#define OV7670_I2C_ADDRESS  0x21 /*TODO: write this in hex (eg. 0xAB) */
#define COM7  0x12
#define CLKRC 0x11
//#define COM9  0x14
#define COM3  0x0c
#define MVFP  0x1E
//#define COM17 0x42
#define COM15 0x40

///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  // TODO: READ KEY REGISTERS
  
  // delay(100);
  
  // TODO: WRITE KEY REGISTERS
  OV7670_write_register(COM7, 0x80); //reset registers, color bar, rgb
  delay(100);
  OV7670_write_register(COM7, 0x06); //reset registers, color bar, rgb
  //OV7670_write_register(COM9, 0xc0); //set gain ceiling
  OV7670_write_register(CLKRC, 0xc0); //use external clock
  OV7670_write_register(COM3, 0x08); //enable scaling
  OV7670_write_register(MVFP, 0xc0); //vertical flip
  OV7670_write_register(COM15, 0xd0); //rgb 565 enable
  read_key_registers();
  set_color_matrix();
}

void loop(){
  
 }


///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  Serial.println("COM7");
  Serial.println(read_register_value(COM7));  
  Serial.println("CLKRC");
  Serial.println(read_register_value(CLKRC));
  //Serial.println("COM9");
  //Serial.println(read_register_value(COM9));
  Serial.println("COM3");
  Serial.println(read_register_value(COM3));
  //Serial.println("COM17");
  //Serial.println(read_register_value(COM17));
  Serial.println("COM15");
  Serial.println(read_register_value(COM15));
  Serial.println("MVFP");
  Serial.println(read_register_value(MVFP));
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  //Serial.println("in read function");
  while(Wire.available()<1){
      //Serial.println("stuck");
    }
  data = Wire.read();
  //Serial.println("passed while loop");
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
