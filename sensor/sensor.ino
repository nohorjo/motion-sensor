#include <WebSocketsClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#define THRESHOLD 1500

// MPU6050 few configuration register addresses
#define MPU6050_REGISTER_SMPLRT_DIV        0x19
#define MPU6050_REGISTER_USER_CTRL         0x6A
#define MPU6050_REGISTER_PWR_MGMT_1        0x6B
#define MPU6050_REGISTER_PWR_MGMT_2        0x6C
#define MPU6050_REGISTER_CONFIG            0x1A
#define MPU6050_REGISTER_GYRO_CONFIG       0x1B
#define MPU6050_REGISTER_ACCEL_CONFIG      0x1C
#define MPU6050_REGISTER_FIFO_EN           0x23
#define MPU6050_REGISTER_INT_ENABLE        0x38
#define MPU6050_REGISTER_ACCEL_XOUT_H      0x3B
#define MPU6050_REGISTER_SIGNAL_PATH_RESET 0x68

int16_t Gx, Gy, Gz;

WebSocketsClient webSocket;

const char *ssid =  "";     // Enter your WiFi Name
const char *password =  ""; // Enter your WiFi Password

const uint8_t MPU6050SlaveAddress = 0x68;

void setup() {
    Wire.begin(D7, D6);
    MPU6050_Init();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    delay(5000);

    webSocket.begin("192.168.1.15", 4457, "/"); // Your local IP here
}

void I2C_Write(uint8_t regAddress, uint8_t data){
    Wire.beginTransmission(MPU6050SlaveAddress);
    Wire.write(regAddress);
    Wire.write(data);
    Wire.endTransmission();
}

// read all 14 register
void Get_Data(){
    Wire.beginTransmission(MPU6050SlaveAddress);
    Wire.write(MPU6050_REGISTER_ACCEL_XOUT_H);
    Wire.endTransmission();
    Wire.requestFrom(MPU6050SlaveAddress, (uint8_t)14, true);

    Wire.read(); Wire.read(); // skip ax
    Wire.read(); Wire.read(); // skip ay
    Wire.read(); Wire.read(); // skip az
    Wire.read(); Wire.read(); // skip temp

    Gx = ((int16_t)Wire.read()<<8) | Wire.read();
    Gy = ((int16_t)Wire.read()<<8) | Wire.read();
    Gz = ((int16_t)Wire.read()<<8) | Wire.read();

    Gx = abs(Gx);
    Gy = abs(Gy);
    Gz = abs(Gz);

}

//configure MPU6050
void MPU6050_Init(){
    delay(150);
    I2C_Write(MPU6050_REGISTER_SMPLRT_DIV, 0x07);
    I2C_Write(MPU6050_REGISTER_PWR_MGMT_1, 0x01);
    I2C_Write(MPU6050_REGISTER_PWR_MGMT_2, 0x00);
    I2C_Write(MPU6050_REGISTER_CONFIG, 0x00);
    I2C_Write(MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
    I2C_Write(MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
    I2C_Write(MPU6050_REGISTER_FIFO_EN, 0x00);
    I2C_Write(MPU6050_REGISTER_INT_ENABLE, 0x01);
    I2C_Write(MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
    I2C_Write(MPU6050_REGISTER_USER_CTRL, 0x00);
}

void loop() {
    Get_Data();

    webSocket.loop();
    
    if (
        Gx > THRESHOLD
        || Gy > THRESHOLD
        || Gz > THRESHOLD
    ) {
        char m[100];
        sprintf(m, "%d,%d,%d", Gx, Gy, Gz);
        webSocket.sendTXT(m);
    }

    delay(100);
}
