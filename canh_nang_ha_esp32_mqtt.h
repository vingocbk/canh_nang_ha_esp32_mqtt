#include "AppDebug.h"
#include <Arduino.h>
#include "WiFi.h"
#include "HTTPClient.h"
#include "WebServer.h"
#include "ESPmDNS.h"
#include "ArduinoJson.h"
#include "Ticker.h"
#include "EEPROM.h"
#include <PubSubClient.h>

#include "soc/soc.h"  //Brownout detector was triggered
#include "soc/rtc_cntl_reg.h"

#define LED_TEST_AP  27 // D4 onchip GPIO2
#define PIN_CONFIG 16       // D3 flash GPIO0
#define PIN_LOA 13 //1 TX    //D6 GPIO12 
#define PIN_BUTTON_UP 4
#define PIN_BUTTON_DOWN 14

#define sample_time 25      //thoi gian lay mau la 25 ms de tinh van toc
#define SPEED_DEFAUT 120    //van toc dong co khi hoat dong binh thuong
#define ERROR_SPEED 50      //sai so cho phep cua van toc, neu nam ngoai khoang nay thi coi nhu co vat can

#define L1 17   //D0
#define L2 18   //D5
#define R1 19    //D1
#define R2 31    //D2
// #define LEDTEST 27  //led on chip
#define PIN_LED_LIGHT_R 23
#define PIN_LED_LIGHT_G 25
#define PIN_LED_LIGHT_B 22
#define BUTTON 12


#define LED_CHANNEL_R 1
#define LED_CHANNEL_G 2
#define LED_CHANNEL_B 3
#define AlphaLed 1

#define RESPONSE_LENGTH 2048     //do dai data nhan ve tu tablet
#define EEPROM_WIFI_SSID_START 0
#define EEPROM_WIFI_SSID_END 32
#define EEPROM_WIFI_PASS_START 33
#define EEPROM_WIFI_PASS_END 64
#define EEPROM_WIFI_DEVICE_ID 65
#define EEPROM_WIFI_SERVER_START 66
#define EEPROM_WIFI_SERVER_END 128
#define EEPROM_WIFI_MAX_CLEAR 512


#define SSID_PRE_AP_MODE "AvyInterior-"
#define PASSWORD_AP_MODE "123456789"

#define m_Getstatus "/getstatus"
#define m_Control "/control"
#define m_Controlhand "/controlhand"
#define m_Resetdistant "/resetdistant"
#define m_Ledrgb "/ledrgb"
#define m_Setmoderun "/setmoderun"
#define m_Settimereturn "/settimereturn"
#define m_Setlowspeed "/setlowspeed"
#define m_Typedevice  "motor"



#define HTTP_PORT 80
#define MQTT_PORT 1883
#define WL_MAC_ADDR_LENGTH 6

#define CONFIG_HOLD_TIME 5000

// const char* mqtt_server = "test.mosquitto.org";
//const char* mqtt_server = "iot.eclipse.org";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "3.3.0.120";

const char* topicsendStatus = "CabinetAvy/HPT/deviceStatus";
const char* m_userNameServer = "avyinterial";
const char* m_passSever = "avylady123";
String m_Pretopic = "CabinetAvy/HPT";
WiFiClient espClient;
PubSubClient client(espClient);

WebServer server(HTTP_PORT);





//normal mode
void getStatus();
void SendStatusReconnect();
void GoUp();
void GoDown();
void Stop();
void clearEeprom();
void SetupNetwork();
// void StartNormalSever();
void tickerupdate();
void buttonClick();
void setPwmLedLighton();
void setPwmLedLightoff();
void setPwmLedLightChange();
void ConnecttoMqttServer();
void callbackMqttBroker(char* topic, byte* payload, unsigned int length);
void checkButtonUpDownClick();
void reconnect();


//Config Mode
void checkButtonConfigClick();      //kiem tra button
void SetupConfigMode();             //phat wifi
void StartConfigServer();           //thiet lap sever
void ConfigMode();                  //nhan data tu app
void setLedApMode();                //hieu ung led
String GetFullSSID();
bool connectToWifi(String nssid, String npass);
bool testWifi(String esid, String epass);
unsigned long ConfigTimeout;

String esid, epass, serverMqtt;
int deviceId;
uint32_t countDisconnectToServer = 0;
unsigned long count_time_disconnect_to_sever = 0;
bool flag_disconnect_to_sever = false;
unsigned long sum_time_disconnect_to_sever = 0;
unsigned long lastReconnectAttempt = 0;

bool Flag_Normal_Mode = true;

bool forward = true;
bool statusStop = true;
bool clickbutton = false;
bool statusLed = false;

// unsigned long Pul_Motor;
// unsigned long test_time, time_start_speed;

bool isLedOn = false;
int countLedLightRed_After = 255, countLedLightGreen_After = 255, countLedLightBlue_After = 255;						//bien cho den sang tu tu khi mo tu
int countLedLightRed_Before = 255, countLedLightGreen_Before = 255, countLedLightBlue_Before = 255;
int countChangeLed = 0;

Ticker tickerSetApMode(setLedApMode, 200, 0);   //every 200ms
Ticker tickerSetPwmLedLightOn(setPwmLedLighton, 20, 260);	//every 20ms
Ticker tickerSetPwmLedLightOff(setPwmLedLightoff, 20, 260);
Ticker tickerSetPwmLedLightChange(setPwmLedLightChange, 10, 260);
