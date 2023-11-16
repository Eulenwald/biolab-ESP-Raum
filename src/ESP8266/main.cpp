/*********
  main raumsensor esp001
  Sensoren und Aktoren befinden sich verteilt im Raum
*********/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <time.h>

// inklude der 3 Sensorklassem
#include "Res/room_light_sensor.h"
#include "Res/air_temperature_sensor.h"
#include "Res/air_moisture_sensor.h"

const char* NTP_SERVER = "de.pool.ntp.org";
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

const char *WIFI_SSID = "WifiSSID";
const char *WIFI_PASSWORD = "WIFIPW";

const char *MQTT_HOST_ONLINE = "HOSTIPONLIE";
const char *MQTT_HOST_PI = "192.168.178.26";

const int MQTT_PORT = 1883;
const char *MQTT_CLIENT_ID = "esp001";
const char *MQTT_PASSWORD = "sdfgdsfgdsgdfsgdg";
const char *MQTT_USER = "eule";

const char *TOPIC_VALUES = "values/sensors";
const char *TOPIC_PARAM = "params/esp001";
const char *MESSAGE = "Esp8266_1 sendet Hallo Welt!";
const char *CONFIG = "esp001,config";

const char *MQTT_HOST = MQTT_HOST_PI;
const char *VERSION = "esp001 v 1.1 23/08/14";
// ############################################################################
/**
 * wifiSetup
 * mqttSetup
 */
unsigned long delayTime;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


/**
 * 4 Temperatursenosren im Raum verteilt
 * Sie bekommen ihre Kennung und eine PIN zum arbeiten
 */
AirTemperatureSensor atsCorner1("ats001", D3);
AirTemperatureSensor atsCorner2("ats002", D3);
AirTemperatureSensor *a_pAirTemperatureSensorList[] = {&atsCorner1, &atsCorner2};
AirMoistureSensor amsCorner1("ams001", D3);
AirMoistureSensor amsCorner2("ams002", D3);
AirMoistureSensor *a_pAirMoistureSensorList[] = {&amsCorner1, &amsCorner2};
/**
 * 2 Lichtsensoren im Raum verteilt
 * Sie bekommen ihre Kennung und eine PIN zum arbeiten
 * zusätzlich ein pin für die Relais-Simulation
 */
RoomLightSensor rlsCorner1("rls001", A0);
RoomLightSensor rlsCorner2("rls002", A0);
RoomLightSensor *m_pRoomLightSensorList[] = {&rlsCorner1, &rlsCorner2};


// ############################################################################
/**
 * vorwaertsdeklaration
*/
// ############################################################################
void publish(String *sVal);
void config(void);
void mqttConnect(void);
void wifiSetup(void);
void mqttSetup(void);
void reconnect(void);
void DDebug(String name);
void showYourConfig(void);
// ############################################################################
/**
 * Stinknormale arduino schleifen
 * setup
 * loop
 */
// ############################################################################
void setup()
{

  Serial.begin(9600);
  while (!Serial) {}
  
  rlsCorner1.m_setRelaisPin(D5);
  rlsCorner2.m_setRelaisPin(D6);

  wifiSetup();
  mqttSetup();

  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);

  config();  
}

int publishKennung;
float sensorVal;
int publishPayload;
void loop()
{
  if (!wifiClient.connected())
  {
    mqttConnect();
  }
  delay(100);

  if (!mqttClient.connected()) {
    reconnect();
  }

  mqttClient.loop();
  delay(10);

  // Erstellen des JSON-Objekts
  DynamicJsonDocument jsonDocument(1024); 
  // Hinzufügen des ESP32-Namens
  jsonDocument["espName"] = MQTT_CLIENT_ID;
  jsonDocument["config"] = false;

  // Erstellen eines JSON-Arrays für die Sensoren
  JsonArray sensorsArray = jsonDocument.createNestedArray("sensors");

  // Erstellen und Hinzufügen des ersten Sensors
  JsonObject sensor;

  publishPayload = 0;  
  
  for (int i = 0; i < 2; i++)
  {

    // TODO Exception abfangen    
    sensorVal = a_pAirTemperatureSensorList[i]->m_getValue();
    if (sensorVal > 0.0)
    {     
      sensor = sensorsArray.createNestedObject();

      publishPayload = 1;      
      sensor["sensorName"] = *a_pAirTemperatureSensorList[i]->m_getName();
      sensor["sensorValue"] = (int)sensorVal;
      sensorVal = 0.0;     
    }
    delay(10);
  }

  for (int i = 0; i < 2; i++)
  {
    // TODO Exception abfangen    
    sensorVal = a_pAirMoistureSensorList[i]->m_getValue();
    if (sensorVal > 0.0)
    {      
      // Es gibt einen Messewert
      sensor = sensorsArray.createNestedObject();

      publishPayload = 1;      
      sensor["sensorName"] = *a_pAirMoistureSensorList[i]->m_getName();;
      sensor["sensorValue"] = (int)sensorVal;     
      sensorVal = 0.0;      
    }
    delay(10);
  }

  for (int i = 0; i < 2; i++)
  {
    // TODO Exception abfangen
    //sensorVal = m_pRoomLightSensorList[i]->m_auslesen();
    sensorVal = m_pRoomLightSensorList[i]->m_getValue();
    if (sensorVal > 0.0)
    {
      // Es gibt einen Messewert
      sensor = sensorsArray.createNestedObject();

      publishPayload = 1;      
      sensor["sensorName"] = *m_pRoomLightSensorList[i]->m_getName();
      sensor["sensorValue"] = (int)sensorVal;   
      sensorVal = 0.0;        

      //publishPayload.concat('\n');      publishPayload.concat(sensorVal);      
    }
    delay(10);
  }

  // Habe ich Sensorwerte
  if(publishPayload == 1) {
    
    // JSON-Daten als String konvertieren
    String jsonStr;
    serializeJson(jsonDocument, jsonStr); // JSON-Dokument in die Datei schreiben
    publish(&jsonStr);    
  }  

  delay(delayTime);
}

// ############################################################################

/**
 * Hilfsfunktionen
 * publish
 * mqttConnect
 * callback
 */
/**
 * puplish
 *
 * String * sVal => nimmt eine normalen String engegen.
 * void <= gibt nichts zurueck
 * Weil die publish/Methode von Mqtt nur ein char-array verarbeiten kann wird der String
 * in ein char-array umgewandelt.
 */
void publish(String *sVal)
{
  int n = sVal->length();
  char char_array[n + 1];
  sVal->toCharArray(char_array, n + 1);
  Serial.printf("values -- Wert: \n%s\n", char_array);
  mqttClient.publish(TOPIC_VALUES, char_array);
  delay(100);
}

/**
 * config
 * Sendet via Mqtt eine Anfrage mit der bitte um configurationsdaten
*/
void config() {
  
  // Erstellen des JSON-Objekts
  DynamicJsonDocument jsonDocument(100); 
  // Hinzufügen des ESP32-Namens
  jsonDocument["espName"] = MQTT_CLIENT_ID;
  jsonDocument["config"] = true;

  String jsonStr;
  serializeJson(jsonDocument, jsonStr); // JSON-Dokument in die Datei schreiben
  publish(&jsonStr);

  //Serial.println(doc);
  Serial.println( *MQTT_CLIENT_ID + " meldet eine config anfrage");
}

/**
 * mqttConnect
 *
 * => nimmt nicht entgegen <= gibt void zurueck
 * Dies Funktion wird zuerst im Mqtt-Setup aufgerufen.
 * Im spaeteren Verlauf aber auch bei einem Verbindungsverlust.
 */
void mqttConnect()
{

  while (!wifiClient.connected())
  {

    Serial.println("Versuche Mqtt-Verbindungsaufbau ...");

    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
    {
      Serial.print("Mit MQTT verbunden");
      mqttClient.subscribe(TOPIC_PARAM, 1);
      Serial.printf(" und mit %s angemeldet\n", TOPIC_PARAM);
      Serial.println();
    }
    else
    {
      Serial.println("MQTT-Verbindung schlug fehl, versuche es erneut!");
      delay(500);
    }
  } // End of while
}

/**
 * callback zur Aufnahme der Konfigurationdaten via Mqtt-Daten
 *
 */
void callback(char *topic, byte *payload, unsigned int length)
{
  String message;
  String sSensorKennung;
  uint iRunning = 0;  
  String sRunning;
  String sPayloadIntervall;
  String sPayloadThresold;
  String sPayloadExtra;
  String sStartTime;
  String sEndTime;

  // 01234567890123456789
  // esp001##1_1234_1234
  for (uint i = 0; i < length; i++) {
   
    message += (char)payload[i];
    if(i < 6)
      sSensorKennung += (char)payload[i];
    else if(i == 8) {
      iRunning += (uint)payload[i];
      sRunning += (char)payload[i];
    }
    else if(i > 9 && i < 13)
      sPayloadIntervall += (char)payload[i];
    else if (i > 13 && i < 18)
      sPayloadThresold += (char)payload[i];
    else if (i > 18 && i < 23)
      sPayloadExtra += (char)payload[i];
    else if (i > 23 && i < 29)
      sStartTime += (char)payload[i];
    else if (i > 29 && i < 35)
      sEndTime += (char)payload[i];
   }
  
  /**
   * Zuerst vergleiche die 3 Als (Air light sensors) sensoren
   * Die haben eine Schwellwert 000 - 999 (wann soll Lich dayu kommen)
   * und einen Intervall 00 - 99
   */
  for (int i = 0; i < 2; i++)
  {
    if (*m_pRoomLightSensorList[i]->m_getName() == sSensorKennung)
    {      
      m_pRoomLightSensorList[i]->m_setInterval(sPayloadIntervall.toInt());
      m_pRoomLightSensorList[i]->m_setThreshold(sPayloadThresold.toInt());
      m_pRoomLightSensorList[i]->m_setNegativeThreshold(sPayloadExtra.toInt());
      m_pRoomLightSensorList[i]->m_setTimeTable(sStartTime, sEndTime);
      m_pRoomLightSensorList[i]->m_setIsRunning(sRunning.toInt());
    }
  }  
  /**
   * Als 2tes die 2 Ats (air temperature sensosrs)
   * Diese haben zur Zeit keine Schwellwerte
   */
  for (int i = 0; i < 2; i++)
  {
    if (*a_pAirTemperatureSensorList[i]->m_getName() == sSensorKennung)
    {       
      a_pAirTemperatureSensorList[i]->m_setInterval(sPayloadIntervall.toInt());
      // sPayloadExtra hat er nicht
      a_pAirTemperatureSensorList[i]->m_setIsRunning(sRunning.toInt());
    }
  }

  /**
   * Als 3tes die 2 Ams (air moisture sensosrs)
   * Diese haben zur  Zeit keine Schwellwerte
   */
  for (int i = 0; i < 2; i++)
  {
    if (*a_pAirMoistureSensorList[i]->m_getName() == sSensorKennung)
    {       
      a_pAirMoistureSensorList[i]->m_setInterval(sPayloadIntervall.toInt());
      // sPayloadExtra hat er nicht
      a_pAirMoistureSensorList[i]->m_setIsRunning(sRunning.toInt());
    }
  }
  showYourConfig();
}
// ############################################################################

void showYourConfig() {
  // im späteren Verlauf, Ausgabe via Mqtt
  Serial.println("######################## ESP Status ###########################################");
  Serial.println(VERSION);
  Serial.print("MQTT_HOST :");
  Serial.println(MQTT_HOST);
  
  for (int i = 0; i < 2; i++) {
    Serial.print("Sensor: ");
    Serial.println(*m_pRoomLightSensorList[i]->m_getName());
    Serial.print("\tintervall: ");
    Serial.print(m_pRoomLightSensorList[i]->m_getInterval());
    Serial.print(" isRuning: ");
    Serial.print(m_pRoomLightSensorList[i]->m_getIsRunning());
    Serial.print(" threshold: ");
    Serial.print(m_pRoomLightSensorList[i]->m_getThreshold());
    Serial.print(" threshold (neg): ");
    Serial.println(m_pRoomLightSensorList[i]->m_getNegativeThreshold());    

    Serial.print("\tstartTime: ");
    Serial.print(*m_pRoomLightSensorList[i]->m_getStartTime());    
    Serial.print(" - endtime: ");
    Serial.println(*m_pRoomLightSensorList[i]->m_getEndTime());    
  }
  
  for (int i = 0; i < 2; i++) {
    Serial.print("Sensor: ");
    Serial.print(*a_pAirTemperatureSensorList[i]->m_getName());
    Serial.print(" intervall: ");
    Serial.print(a_pAirTemperatureSensorList[i]->m_getInterval());
    Serial.print(" isRuning: ");
    Serial.println(a_pAirTemperatureSensorList[i]->m_getIsRunning());
  }

  for (int i = 0; i < 2; i++) {
    Serial.print("Sensor: ");
    Serial.print(*a_pAirMoistureSensorList[i]->m_getName());
    Serial.print(" intervall: ");
    Serial.print(a_pAirMoistureSensorList[i]->m_getInterval());
    Serial.print(" isRuning: ");
    Serial.println(a_pAirMoistureSensorList[i]->m_getIsRunning());
  }
  Serial.println("###############################################################################");
}


/**
 * Ausgelagerte Setup-funktionen
 * wifiSetup
 * mqttSetup
 */
void wifiSetup()
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Verbindungsaufbau zum Wifi ...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println(".");
  Serial.println(WiFi.localIP());

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void mqttSetup()
{
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(callback);
  mqttConnect();
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Verbindung zum MQTT-Broker wird wiederhergestellt...");
    
    if (mqttClient.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Verbunden mit dem MQTT-Broker");
      mqttClient.subscribe(TOPIC_PARAM);
    } else {
      Serial.print("Fehlgeschlagen, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Neuer Versuch in 5 Sekunden...");
      delay(5000);
    }
  }
}

// ############################################################################
void DDebug(String name) {
  Serial.println(name + " wurde ausgefuert!");
}