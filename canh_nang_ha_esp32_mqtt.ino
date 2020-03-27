#include "canh_nang_ha_esp32_mqtt.h"


void getStatus(){
    String datasend = "{\"deviceid\" : \"";
    datasend += String(deviceId);
    datasend += "\", \"devicetype\" : \"motor\", \"typecontrol\" : \"getstatus\",  \"number_disconnect\" : \"";
    datasend += String(countDisconnectToServer - 1);
    datasend += "\", \"all_time\" : \"";
    datasend += String(sum_time_disconnect_to_sever);
    datasend +=  "\", \"status\" : \"";
    if(forward == true){
        datasend += "open\"}";
        client.publish(topicsendStatus, datasend.c_str());
    }else{
        datasend += "close\"}";
        client.publish(topicsendStatus, datasend.c_str());
    }
    ECHOLN("getstatus");
}

void SendStatusReconnect(){
    const char* willTopic = "CabinetAvy/HPT/LWT";
    String ReconnectMessage = "{\"devicetype\" : \"";
    ReconnectMessage += m_Typedevice;
    ReconnectMessage += "\", \"deviceid\" : \"";
    ReconnectMessage += String(deviceId);
    ReconnectMessage += "\", \"status\" : \"ok\"}";
    client.publish(willTopic, ReconnectMessage.c_str());
    ECHOLN("-------Reconnect-------");
}

void Close(){
    ECHOLN("Close!");

    

    String datasend = "{\"deviceid\" : \"";
    datasend += String(deviceId); 
    datasend += "\", \"devicetype\" : \"motor\", \"typecontrol\" : \"control\",  \"number_disconnect\" : \"";
    datasend += String(countDisconnectToServer - 1);
    datasend += "\", \"all time\" : \"";
    datasend += String(sum_time_disconnect_to_sever);
    datasend += "\", \"status\" : \"close\"}";
    client.publish(topicsendStatus, datasend.c_str());


    digitalWrite(PIN_LOA, HIGH);
    digitalWrite(L2, LOW);
    // digitalWrite(R1, LOW);
    delay(200);

    
    digitalWrite(L1, HIGH);
    // digitalWrite(R2, HIGH);
    delay(300);
    digitalWrite(PIN_LOA, LOW);
    forward = false;
    statusStop = false;
    statusLed = false;

    tickerSetPwmLedLightOn.stop();
    tickerSetPwmLedLightOff.start();
    
}

void Open(){
    ECHOLN("Open!");
    
    

    String datasend = "{\"deviceid\" : \"";
    datasend += String(deviceId); 
    datasend += "\", \"devicetype\" : \"motor\", \"typecontrol\" : \"control\",  \"number_disconnect\" : \"";
    datasend += String(countDisconnectToServer - 1);
    datasend += "\", \"all_time\" : \"";
    datasend += String(sum_time_disconnect_to_sever);
    datasend += "\", \"status\" : \"open\"}";
    client.publish(topicsendStatus, datasend.c_str());
    digitalWrite(PIN_LOA, HIGH);
    digitalWrite(L1, LOW);
    // digitalWrite(R2, LOW);
    delay(200);
    digitalWrite(L2, HIGH);
    // digitalWrite(R1, HIGH);
    
    delay(300);
    digitalWrite(PIN_LOA, LOW);
    forward = true;
    statusStop = false;
    statusLed = true;

    tickerSetPwmLedLightOff.stop();
    tickerSetPwmLedLightOn.start();
}

void Stop(){
    String datasend = "{\"deviceid\" : \"";
    datasend += String(deviceId); 
    datasend += "\", \"devicetype\" : \"motor\", \"typecontrol\" : \"control\",  \"number_disconnect\" : \"";
    datasend += String(countDisconnectToServer - 1);
    datasend += "\", \"all_time\" : \"";
    datasend += String(sum_time_disconnect_to_sever);
    datasend += "\", \"status\" : \"stop\"}";
    client.publish(topicsendStatus, datasend.c_str());

    digitalWrite(PIN_LOA, HIGH);
    digitalWrite(L1, LOW);
    // digitalWrite(L1, HIGH);
    digitalWrite(R2, LOW);
    digitalWrite(L2, LOW);
    digitalWrite(R1, LOW);
    // digitalWrite(R1, HIGH);
    ECHOLN("Stop");
    statusStop = true;
    delay(500);
    digitalWrite(PIN_LOA, LOW);
    //tickerSetMotor.stop();
}



void clearEeprom(){
    ECHOLN("clearing eeprom");
    for (int i = 0; i < EEPROM_WIFI_MAX_CLEAR; ++i){
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    ECHOLN("Done writing!");
}


void ConfigMode(){
    StaticJsonBuffer<RESPONSE_LENGTH> jsonBuffer;
    ECHOLN(server.arg("plain"));
    JsonObject& rootData = jsonBuffer.parseObject(server.arg("plain"));
    ECHOLN("--------------");
    tickerSetApMode.stop();
    digitalWrite(LED_TEST_AP, LOW);
    if (rootData.success()) {
        server.sendHeader("Access-Control-Allow-Headers", "*");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\"}");
        //server.stop();
        String nssid = rootData["ssid"];
        String npass = rootData["password"];
        String nid = rootData["deviceid"];
        String nserver = rootData["server"];


        esid = nssid;
        epass = npass;
        deviceId = nid.toInt();
        serverMqtt = nserver;

        ECHOLN("clearing eeprom");
        for (int i = 0; i <= EEPROM_WIFI_SERVER_END; i++){ 
            EEPROM.write(i, 0); 
        }
        ECHOLN("writing eeprom ssid:");
        ECHO("Wrote: ");
        for (int i = 0; i < nssid.length(); ++i){
            EEPROM.write(i+EEPROM_WIFI_SSID_START, nssid[i]);             
            ECHO(nssid[i]);
        }
        ECHOLN("");
        ECHOLN("writing eeprom pass:"); 
        ECHO("Wrote: ");
        for (int i = 0; i < npass.length(); ++i){
            EEPROM.write(i+EEPROM_WIFI_PASS_START, npass[i]);
            ECHO(npass[i]);
        }
        ECHOLN("");
        ECHOLN("writing eeprom device id:"); 
        ECHO("Wrote: ");
        EEPROM.write(EEPROM_WIFI_DEVICE_ID, deviceId);
        ECHOLN(deviceId);

        ECHOLN("writing eeprom server:"); 
        ECHO("Wrote: ");
        for (int i = 0; i < nserver.length(); ++i){
            EEPROM.write(i+EEPROM_WIFI_SERVER_START, nserver[i]);
            ECHO(nserver[i]);
        }
        ECHOLN("");

        EEPROM.commit();
        ECHOLN("Done writing!");

        if (testWifi(nssid, npass)) {

            // ConnecttoMqttServer();
            Flag_Normal_Mode = true;
            return;
        }
        tickerSetApMode.start();
        ECHOLN("Wrong wifi!!!");
        SetupConfigMode();
        StartConfigServer();
        return;
    }
    ECHOLN("Wrong data!!!");
}

bool connectToWifi(String nssid, String npass) {
    ECHOLN("Open STA....");
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(100);
    // setupIP();
    //WiFi.begin(nssid.c_str(), npass.c_str());

    if (testWifi(nssid, npass)) {
        return true;
    }

    return false;
}

bool testWifi(String esid, String epass) {
    ECHO("Connecting to: ");
    ECHOLN(esid);
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    server.close();
    ECHO("delay: ");
    ECHOLN((deviceId-1)*3000 + 1000);
    for(int i = 0; i < 100; i++){
        if(digitalRead(PIN_CONFIG) == LOW){
            break;
        }
        delay(((deviceId-1)*3000 + 1000)/100);
    }

    WiFi.mode(WIFI_STA);        //bat che do station
    WiFi.begin(esid.c_str(), epass.c_str());
    int c = 0;
    ECHOLN("Waiting for Wifi to connect");
    while (c < 20) {
        if (WiFi.status() == WL_CONNECTED) {
            ECHOLN("\rWifi connected!");
            ECHO("Local IP: ");
            ECHOLN(WiFi.localIP());
            // digitalWrite(LED_TEST_AP, HIGH);
            ConnecttoMqttServer();
            return true;
        }
        delay(500);
        ECHO(".");
        c++;
        if(digitalRead(PIN_CONFIG) == LOW){
            break;
        }
    }
    ECHOLN("");
    ECHOLN("Connect timed out");
    return false;
}

void ConnecttoMqttServer(){
    client.setServer(serverMqtt.c_str(), MQTT_PORT);
    client.setCallback(callbackMqttBroker);
    reconnect();
}


void callbackMqttBroker(char* topic, byte* payload, unsigned int length){
    String Topic = String(topic);
    ECHO("TOPIC: ");
    ECHOLN(Topic);

    String data;
    for (int i = 0; i < length; i++) {
        data += char(payload[i]);
    }
    ECHO("DATA: ");
    ECHOLN(data);
    StaticJsonBuffer<RESPONSE_LENGTH> jsonBuffer;
    JsonObject& rootData = jsonBuffer.parseObject(data);



    if(Topic.indexOf(m_Controlhand) > 0){
        if (rootData.success()){
            if(rootData["typedevice"] == m_Typedevice){
                int arraySize = rootData["deviceid"].size();   //get size of JSON Array
                int sensorValue[arraySize];
                bool isTrueControl = false;
                for (int i = 0; i < arraySize; i++) { //Iterate through results
                    sensorValue[i] = rootData["deviceid"][i];  //Implicit cast
                    // ECHOLN(sensorValue[i]);
                    if(sensorValue[i] == deviceId){
                        isTrueControl = true;
                        break;
                    }
                }
                if(isTrueControl == true){
                    String dataType = rootData["typecontrol"];
                    //---------control color------------------
                    if(dataType == "controlled"){
                        // timeToSaveData = millis();
                        // flagIsHaveChangeColorHand = true;
                        
                        if(forward == true){
                            int controlled[3];
                            for (int i = 0; i < 3; i++) { //Iterate through results
                                controlled[i] = rootData["data"][i];  //Implicit cast
                            }
                            countLedLightRed_After = controlled[0];
                            countLedLightGreen_After = controlled[1];
                            countLedLightBlue_After = controlled[2];
                            ledcWrite(LED_CHANNEL_R, uint8_t(countLedLightRed_After));
                            ledcWrite(LED_CHANNEL_G, uint8_t(countLedLightGreen_After));
                            ledcWrite(LED_CHANNEL_B, uint8_t(countLedLightBlue_After));
                            countLedLightRed_Before = countLedLightRed_After;
                            countLedLightGreen_Before = countLedLightGreen_After;
                            countLedLightBlue_Before = countLedLightBlue_After;
                            ECHOLN("---------");
                        }
                        

                    }
                }
            }
        }
    }

    else if(Topic.indexOf(m_Control) > 0){
        if (rootData.success()){
            //--------------getstatus-------------
            if(rootData["typedevice"] == m_Typedevice){
                int arraySize = rootData["deviceid"].size();   //get size of JSON Array
                int sensorValue[arraySize];
                bool isTrueControl = false;
                for (int i = 0; i < arraySize; i++) { //Iterate through results
                    sensorValue[i] = rootData["deviceid"][i];  //Implicit cast
                    // ECHOLN(sensorValue[i]);
                    if(sensorValue[i] == deviceId){
                        isTrueControl = true;
                        break;
                    }
                }
                if(isTrueControl == true){
                    String dataType = rootData["typecontrol"];
                    //---------control------------------
                    if(dataType == "getstatus"){
                        getStatus();
                    }
                    else if(dataType == "control"){
                        String dataControl = rootData["data"];
                        if(dataControl == "open"){
                            Open();
                        }else if(dataControl == "close"){
                            Close();
                        }
                        else if(dataControl == "stop"){
                            Stop();
                        }
                    }
                    else if(dataType == "controlled"){
                        if(forward == true){
                            int controlled[3];
                            for (int i = 0; i < 3; i++) { //Iterate through results
                                controlled[i] = rootData["data"][i];  //Implicit cast
                            }
                            countLedLightRed_After = controlled[0];
                            countLedLightGreen_After = controlled[1];
                            countLedLightBlue_After = controlled[2];
                            // ECHO("Writed: ");
                            // ECHO(countLedLightRed_After);
                            // ECHO(",");
                            // ECHO(countLedLightGreen_After);
                            // ECHO(",");
                            // ECHOLN(countLedLightBlue_After);
                            tickerSetPwmLedLightOn.stop();
                            tickerSetPwmLedLightOff.stop();
                            tickerSetPwmLedLightChange.start();
                            // EEPROM.write(EEPROM_WIFI_LED_RED, char(countLedLightRed_After));
                            // EEPROM.write(EEPROM_WIFI_LED_GREEN, char(countLedLightGreen_After));
                            // EEPROM.write(EEPROM_WIFI_LED_BLUE, char(countLedLightBlue_After));
                            // EEPROM.commit();
                        }
                    }
                }
            }
        }
    }

    
}

void reconnect() {
    // Loop until we're reconnected
    ECHO("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Avy-";
    // clientId += GetFullSSID;
    clientId += String(random(0xffffff), HEX);
    const char* willTopic = "CabinetAvy/HPT/LWT";
    uint8_t willQos = 0;
    boolean willRetain = false;
    String willMessage = "{\"devicetype\" : \"";
    willMessage += m_Typedevice;
    willMessage += "\", \"deviceid\" : \"";
    willMessage += String(deviceId);
    willMessage += "\", \"status\" : \"error\"}";
    // Attempt to connect
    if (client.connect(clientId.c_str(), willTopic, willQos, willRetain, willMessage.c_str())) {
        ECHO("connected with id: ");
        ECHOLN(clientId);
        // Once connected, publish an announcement...
        // client.publish("outTopic", "hello world");
        // ... and resubscribe
        String topicControl = m_Pretopic + m_Control;
        String topicControlhand = m_Pretopic + m_Controlhand;

        // char topicGetstatusArray[topicGetstatus.length() + 1];
        // char topicControlArray[topicControl.length() + 1];

        // topicGetstatus.toCharArray(topicGetstatusArray, topicGetstatus.length() + 1);
        // topicControl.toCharArray(topicControlArray, topicControl.length() + 1);
        
        // client.subscribe(topicGetstatus.c_str());
        client.subscribe(topicControl.c_str());
        client.subscribe(topicControlhand.c_str());
        ECHO("Done Subscribe Channel: ");
        // ECHO(topicGetstatus);
        // ECHO("  +  ");
        ECHO(topicControl);
        ECHO(", ");
        ECHOLN(topicControlhand);
        countDisconnectToServer++;
        if(flag_disconnect_to_sever == true){
            sum_time_disconnect_to_sever += millis() - count_time_disconnect_to_sever;
            flag_disconnect_to_sever = false;
        }
        digitalWrite(LED_TEST_AP, HIGH);

        getStatus();
        SendStatusReconnect();
    } else {
        ECHO("failed, rc=");
        ECHO(client.state());
        ECHOLN(" try again in 2 seconds");
        // Wait 2 seconds before retrying
        delay(2000);
    }
}




void setLedApMode() {
    digitalWrite(LED_TEST_AP, !digitalRead(LED_TEST_AP));
}



String GetFullSSID() {
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    String macID;
    WiFi.mode(WIFI_AP);
    WiFi.softAPmacAddress(mac);
    macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) + String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    ECHO("[Helper][getIdentify] Identify: ");
    ECHO(SSID_PRE_AP_MODE);
    ECHOLN(macID);
    return macID;
}

void checkButtonConfigClick(){
    if (digitalRead(PIN_CONFIG) == LOW && (ConfigTimeout + CONFIG_HOLD_TIME) <= millis()) { // Khi an nut
        ConfigTimeout = millis();
        //tickerSetMotor.attach(0.2, setLedApMode);  //every 0.2s
        Flag_Normal_Mode = false;
        tickerSetApMode.start();
        SetupConfigMode();
        StartConfigServer();
    } else if(digitalRead(PIN_CONFIG) == HIGH) {
        ConfigTimeout = millis();
    }
}


void SetupConfigMode(){
    ECHOLN("[WifiService][setupAP] Open AP....");
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    server.close();
    delay(1000);
    WiFi.mode(WIFI_AP_STA);
    IPAddress APIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    String SSID_AP_MODE = SSID_PRE_AP_MODE + GetFullSSID();
    WiFi.softAP(SSID_AP_MODE.c_str(), PASSWORD_AP_MODE);

    WiFi.softAPConfig(APIP, gateway, subnet);
    ECHOLN(SSID_AP_MODE);

    ECHOLN("[WifiService][setupAP] Softap is running!");
    IPAddress myIP = WiFi.softAPIP();
    ECHO("[WifiService][setupAP] IP address: ");
    ECHOLN(myIP);
}
void StartConfigServer(){    
    ECHOLN("[HttpServerH][startConfigServer] Begin create new server...");
//    server = new ESP8266WebServer(HTTP_PORT);
    server.on("/config", HTTP_POST, ConfigMode);
    server.begin();
    ECHOLN("[HttpServerH][startConfigServer] HTTP server started");
}





void SetupNetwork() {
    ECHOLN("Reading EEPROM ssid");
    esid = "";
    for (int i = EEPROM_WIFI_SSID_START; i < EEPROM_WIFI_SSID_END; ++i){
        esid += char(EEPROM.read(i));
    }
    ECHO("SSID: ");
    ECHOLN(esid);
    ECHOLN("Reading EEPROM pass");
    epass = "";
    for (int i = EEPROM_WIFI_PASS_START; i < EEPROM_WIFI_PASS_END; ++i){
        epass += char(EEPROM.read(i));
    }
    ECHO("PASS: ");
    ECHOLN(epass);

    ECHOLN("Reading EEPROM Device ID");
    deviceId = EEPROM.read(EEPROM_WIFI_DEVICE_ID);
    ECHO("ID: ");
    ECHOLN(deviceId);

    ECHOLN("Reading EEPROM server");
    serverMqtt = "";
    for (int i = EEPROM_WIFI_SERVER_START; i < EEPROM_WIFI_SERVER_END; ++i){
        serverMqtt += char(EEPROM.read(i));
    }
    ECHO("SERVER: ");
    ECHOLN(serverMqtt);
    
    testWifi(esid, epass);
}

// void StartNormalSever(){
//     server.on("/", HTTP_GET, handleRoot);
//     server.on("/getstatus", HTTP_GET, getStatus);
//     server.on("/close", HTTP_GET, Close);
//     server.on("/open", HTTP_GET, Open);
//     server.on("/stop", HTTP_GET, Stop);
//     server.on("/", HTTP_OPTIONS, handleOk);
//     server.on("/getstatus", HTTP_OPTIONS, getStatus);
//     server.on("/close", HTTP_OPTIONS, handleOk);
//     server.on("/open", HTTP_OPTIONS, handleOk);
//     server.on("/stop", HTTP_OPTIONS, handleOk);
//     server.begin();
//     ECHOLN("HTTP server started");
// }

void buttonClick(){  
    if(statusStop == false){
        Stop();
    }
    else if(forward == false ){
      Open();
    }else {
      Close();
    }
    delay(500);
}

void tickerupdate(){
    tickerSetApMode.update();
    tickerSetPwmLedLightOn.update();
    tickerSetPwmLedLightOff.update();
    tickerSetPwmLedLightChange.update();
}


void setPwmLedLighton(){

    countChangeLed++;
    float out_led_red, out_led_green, out_led_blue;
    out_led_red = (float)0 + ((float)((float)countLedLightRed_After - (float)0)/255)*countChangeLed;
    out_led_red = abs(out_led_red);
    ledcWrite(LED_CHANNEL_R, uint8_t(out_led_red*AlphaLed));

    out_led_green = (float)0 + ((float)((float)countLedLightGreen_After - (float)0)/255)*countChangeLed;
    out_led_green = abs(out_led_green);
    ledcWrite(LED_CHANNEL_G, uint8_t(out_led_green*AlphaLed));

    out_led_blue = (float)0 + ((float)((float)countLedLightBlue_After - (float)0)/255)*countChangeLed;
    out_led_blue = abs(out_led_blue);
    ledcWrite(LED_CHANNEL_B, uint8_t(out_led_blue*AlphaLed));

    if(countChangeLed == 255){
        ECHOLN("On Led");
		countChangeLed = 0;
        tickerSetPwmLedLightOn.stop();
	}
}

void setPwmLedLightoff(){
    countChangeLed++;
	float out_led_red, out_led_green, out_led_blue;
    out_led_red = (float)countLedLightRed_Before + ((float)((float)0 - (float)countLedLightRed_Before)/255)*countChangeLed;
    out_led_red = abs(out_led_red);
	ledcWrite(LED_CHANNEL_R, uint8_t(out_led_red*AlphaLed));
    
    out_led_green = (float)countLedLightGreen_Before + ((float)((float)0 - (float)countLedLightGreen_Before)/255)*countChangeLed;
    out_led_green = abs(out_led_green);
	ledcWrite(LED_CHANNEL_G, uint8_t(out_led_green*AlphaLed));
    
    out_led_blue = (float)countLedLightBlue_Before + ((float)((float)0 - (float)countLedLightBlue_Before)/255)*countChangeLed;
    out_led_blue = abs(out_led_blue);
	ledcWrite(LED_CHANNEL_B, uint8_t(out_led_blue*AlphaLed));
    
    if(countChangeLed == 255){
        ECHOLN("Off Led");
		countChangeLed = 0;
        tickerSetPwmLedLightOff.stop();
	}

}

void setPwmLedLightChange(){
    countChangeLed++;
    float out_led_red, out_led_green, out_led_blue;
    out_led_red = (float)countLedLightRed_Before + (((float)countLedLightRed_After - (float)countLedLightRed_Before)/255)*countChangeLed;
    out_led_red = abs(out_led_red);
    ledcWrite(LED_CHANNEL_R, uint8_t(out_led_red*AlphaLed));

    out_led_green = (float)countLedLightGreen_Before + (((float)countLedLightGreen_After - (float)countLedLightGreen_Before)/255)*countChangeLed;
    out_led_green = abs(out_led_green);
    ledcWrite(LED_CHANNEL_G, uint8_t(out_led_green*AlphaLed));

    out_led_blue = (float)countLedLightBlue_Before + (((float)countLedLightBlue_After - (float)countLedLightBlue_Before)/255)*countChangeLed;
    out_led_blue = abs(out_led_blue);
    ledcWrite(LED_CHANNEL_B, uint8_t(out_led_blue*AlphaLed));

    // ECHO(uint8_t(out_led_red));
    // ECHO("-----");
    // ECHO(uint8_t(out_led_green));
    // ECHO("-----");
    // ECHOLN(uint8_t(out_led_blue));
    if(countChangeLed == 255){
        ECHOLN("Change Led");
		countChangeLed = 0;
        countLedLightRed_Before = countLedLightRed_After;
        countLedLightGreen_Before = countLedLightGreen_After;
        countLedLightBlue_Before = countLedLightBlue_After;
        tickerSetPwmLedLightChange.stop();
	}

}


void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

    ledcSetup(LED_CHANNEL_R, 1000, 8); // 1 kHz PWM, 8-bit resolution
    ledcSetup(LED_CHANNEL_G, 1000, 8); // 1 kHz PWM, 8-bit resolution
    ledcSetup(LED_CHANNEL_B, 1000, 8); // 1 kHz PWM, 8-bit resolution

    ledcAttachPin(PIN_LED_LIGHT_R, LED_CHANNEL_R); // analog pin to channel led_R
    ledcAttachPin(PIN_LED_LIGHT_G, LED_CHANNEL_G); // analog pin to channel led_G
    ledcAttachPin(PIN_LED_LIGHT_B, LED_CHANNEL_B); // analog pin to channel led_B


    pinMode(LED_TEST_AP, OUTPUT);
    pinMode(PIN_CONFIG, INPUT);
    pinMode(PIN_LOA, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(R1, OUTPUT);
    pinMode(R2, OUTPUT);
    pinMode(L1, OUTPUT);
    pinMode(L2, OUTPUT);
    // pinMode(LEDTEST, OUTPUT);
    delay(10);
    digitalWrite(L1, LOW);
    // digitalWrite(L1, HIGH);
    digitalWrite(R2, LOW);
    digitalWrite(L2, LOW);
    digitalWrite(R1, LOW);
    // digitalWrite(R1, HIGH);
    digitalWrite(LED_TEST_AP, LOW);

    ledcWrite(LED_CHANNEL_R, countLedLightRed_After);
    ledcWrite(LED_CHANNEL_G, countLedLightGreen_After);
    ledcWrite(LED_CHANNEL_B, countLedLightBlue_After);


    // pinMode(PIN_LOA, INPUT_PULLUP);
    Serial.begin(115200);
    EEPROM.begin(512);

    SetupNetwork();     //khi hoat dong binh thuong
    Close();
    //external interrupt doc tin hieu encoder

}


void loop() {
    if (Flag_Normal_Mode == true && WiFi.status() != WL_CONNECTED){
        // digitalWrite(LED_TEST_AP, LOW);
        if(flag_disconnect_to_sever == false){
            count_time_disconnect_to_sever = millis();
            flag_disconnect_to_sever = true;
        }
        testWifi(esid, epass); 
    } 
    // if(WiFi.status() == WL_CONNECTED){
    //     if (!client.connected()) {
    //         if(flag_disconnect_to_sever == false){
    //             count_time_disconnect_to_sever = millis();
    //             flag_disconnect_to_sever = true;
    //         }
            
    //         reconnect();
    //     }
    //     client.loop();
    // }


    if(WiFi.status() == WL_CONNECTED){
        if (!client.connected()) {
            digitalWrite(LED_TEST_AP, LOW);
            if(flag_disconnect_to_sever == false){
                count_time_disconnect_to_sever = millis();
                lastReconnectAttempt = millis();
                flag_disconnect_to_sever = true;

                // HTTPClient httpclient;

                // String ipsend = "http://";
                // ipsend += serverMqtt;
                // ipsend += ":8888/{\"deviceid\" : \"";
                // ipsend += String(deviceId);
                // ipsend += "\", \"devicetype\" : \"motor\", \"status\" : \"error\"}";
                // httpclient.setTimeout(2000);
                // httpclient.begin(ipsend); //HTTP
                // ECHOLN(ipsend);
                // int httpCode = httpclient.GET();

                // // httpCode will be negative on error
                // if(httpCode > 0) {
                //     // HTTP header has been send and Server response header has been handled
                //     ECHO("[HTTP] GET... code: ");
                //     ECHOLN(httpCode);

                //     // file found at server
                //     if(httpCode == HTTP_CODE_OK) {
                //         String payload = httpclient.getString();
                //         ECHOLN(payload);
                //     }
                // } else {
                //     ECHO("[HTTP] GET... failed, error: ");
                //     ECHOLN(httpclient.errorToString(httpCode).c_str());
                // }

                // httpclient.end();
            }
            unsigned long nowReconnectAttempt = millis();

            if (abs(nowReconnectAttempt - lastReconnectAttempt) > 10000) {
                lastReconnectAttempt = nowReconnectAttempt;
                reconnect();
            }
        }else{
            client.loop();
        }
        
    }




    checkButtonConfigClick();
    tickerupdate();
    server.handleClient();
}
