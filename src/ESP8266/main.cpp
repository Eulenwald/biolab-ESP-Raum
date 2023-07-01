/*********
  main raumsensor esp001
  Sensoren und Aktoren befinden sich verteilt im Raum
*********/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Arduino.h>
#include <Wire.h>

#include "Res/room_light_sensor.h"
#include "Res/air_temperature_sensor.h"
#include "Res/air_moisture_sensor.h"

const char *WIFI_SSID = "TheBigOne";
const char *WIFI_PASSWORD = "Rab2l1st1234";

const char *MQTT_HOST_ONLINE = "85.214.44.154";
const char *MQTT_HOST_PI = "192.168.178.26";
// const char *MQTT_HOST_ONLINE = "eulenwald-online-services.eu";
const int MQTT_PORT = 1883;
const char *MQTT_CLIENT_ID = "esp001";
// mosquitto_passwd -b -c passwd eule _dfg34%dsg456
const char *MQTT_PASSWORD = "_dfg34%dsg456";
const char *MQTT_USER = "eule";

const char *TOPIC_VALUES = "values/sensors";
const char *TOPIC_PARAM = "params/esp001";
const char *MESSAGE = "Esp8266_1 sendet Hallo Welt!";
const char *CONFIG = "esp001,config";

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
 * 3 Lichtsensoren im Raum verteilt
 * Sie bekommen ihre Kennung und eine PIN zum arbeiten
 * zusätzlich ein pin für die Relais-Simulation
 */
RoomLightSensor rlsCorner1("rls001", A0);
RoomLightSensor rlsCorner2("rls002", A0);
RoomLightSensor rlsCorner3("rls003", A0);
RoomLightSensor *m_pRoomLightSensorList[] = {&rlsCorner1, &rlsCorner2, &rlsCorner3};



// ############################################################################
/**
 * vorweartsdeklaration
*/
// ############################################################################
void publish(String *sVal);
void config(void);
void mqttConnect(void);
void wifiSetup(void);
void mqttSetup(void);
void reconnect(void);
void DDebug(String name);
// ############################################################################
/**
 * Stinknormale arduino schleifen
 * setup
 * loop
 */
void setup()
{

  Serial.begin(9600);
  delay(100);

  while (!Serial) {}
  delay(100);

  atsCorner1.m_setInterval(10);
  atsCorner2.m_setInterval(10);
  amsCorner1.m_setInterval(10);
  amsCorner2.m_setInterval(10);

  rlsCorner1.m_setRelaisPin(D5);
  rlsCorner2.m_setRelaisPin(D6);
  rlsCorner3.m_setRelaisPin(D7);

  wifiSetup();
  mqttSetup();

  config();

  //  pinMode(LED_BUILTIN, OUTPUT);
}

String rVal;
String sensorVal;
String publishVal;
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
  publishVal = "";
  String firstSet = "name,wert\n";
  // for(int i=0; i < (int)sizeof(*mAtsList); i++) {
  for (int i = 0; i < 2; i++)
  {

    // TODO Exception abfangen
    sensorVal = a_pAirTemperatureSensorList[i]->m_auslesen();
    if (sensorVal != "")
    {      
      publishVal.concat('\n');
      publishVal.concat(firstSet);
      publishVal.concat(sensorVal);      
      firstSet = "";
    }
    delay(100);
  }

  if(publishVal != "") {
    rVal = MQTT_CLIENT_ID;
    rVal.concat(",values");
    rVal.concat(publishVal);
    publish(&rVal);
  }
  publishVal = "";

  firstSet = "name,wert\n";
  // for(int i=0; i < (int)sizeof(*mAtsList); i++) {
  for (int i = 0; i < 2; i++)
  {

    // TODO Exception abfangen
    sensorVal = a_pAirMoistureSensorList[i]->m_auslesen();
    if (sensorVal != "")
    {      
      publishVal.concat('\n');
      publishVal.concat(firstSet);
      publishVal.concat(sensorVal);      
      firstSet = "";
    }
    delay(100);
  }

  if(publishVal != "") {
    rVal = MQTT_CLIENT_ID;
    rVal.concat(",values");
    rVal.concat(publishVal);
    publish(&rVal);
  }
  publishVal = "";

  firstSet = "name,wert\n";
  // for(int i=0; i < (int)sizeof(*mAlsList); i++) {
  for (int i = 0; i < 3; i++)
  {
    // TODO Exception abfangen
    sensorVal = m_pRoomLightSensorList[i]->m_auslesen();
    if (sensorVal != "")
    {
      publishVal.concat('\n');
      publishVal.concat(firstSet);
      publishVal.concat(sensorVal);      
      firstSet = "";
    }
    delay(100);
  }

  if(publishVal != "") {
    rVal = MQTT_CLIENT_ID;
    rVal.concat(",values");
    rVal.concat(publishVal);
    publish(&rVal);
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
 * void <= gibt nicht zurueck
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
  mqttClient.publish(TOPIC_VALUES, CONFIG);
  Serial.println("esp001 meldet eine config anfrage");
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
  String sPayload1;
  String sPayload2;

  // 01234567890123456789
  // esp001##1_123_123
  for (uint i = 0; i < length; i++) {
    message += (char)payload[i];
    if(i < 6)
      sSensorKennung += (char)payload[i];
    else if(i == 8) {
      iRunning += (uint)payload[i];
      sRunning += (char)payload[i];
    }
    else if(i > 9 && i < 13)
      sPayload1 += (char)payload[i];
    else if(i > 13)
      sPayload2 += (char)payload[i];
  }

  /**
   * Fuer welchen Sensor sind die Parameter
   */
  //String sSensorKennung = message.substring(0, 6);
  //uint iRunning = message.substring(8, 9).toInt();  
  //String sPayload = message.substring(10, (int)length);
  //Serial.println("config -- name: " + sSensorKennung + " payload 1: " + sPayload1  + " payload 2 -- " + sPayload2);
  /**
   * Zuerst vergleiche die 3 Als (Air light sensors) sensoren
   * Die haben eine Schwellwert 000 - 999 (wann soll Lich dayu kommen)
   * und einen Intervall 00 - 99
   */
  // sizeof scheint fehlerhaft implementiert zu sein
  // for(int i=0; i < (int)sizeof(*mAlsList); i++) {
  for (int i = 0; i < 3; i++)
  {
    if (*m_pRoomLightSensorList[i]->m_getName() == sSensorKennung)
    {      
      m_pRoomLightSensorList[i]->m_setInterval(sPayload1.toInt());
      m_pRoomLightSensorList[i]->m_setThreshold(sPayload2.toInt());
      m_pRoomLightSensorList[i]->m_setIsRunning(sRunning.toInt());
      return;
    }
  }
  delay(100);
  /**
   * Als 2tes die 2 Ats (air temperature sensosrs)
   * Diese haben zur  Zeit keine Schwellwerte
   */
  for (int i = 0; i < 2; i++)
  {
    if (*a_pAirTemperatureSensorList[i]->m_getName() == sSensorKennung)
    {       
      a_pAirTemperatureSensorList[i]->m_setInterval(sPayload1.toInt());
      //a_pAirTemperatureSensorList[i]->m_setIsRunning(iRunning);
      a_pAirTemperatureSensorList[i]->m_setIsRunning(sRunning.toInt());
      return;
    }
  }
  delay(100);


  /**
   * Als 3tes die 2 Ams (air moisture sensosrs)
   * Diese haben zur  Zeit keine Schwellwerte
   */
  for (int i = 0; i < 2; i++)
  {
    if (*a_pAirMoistureSensorList[i]->m_getName() == sSensorKennung)
    {       
      a_pAirMoistureSensorList[i]->m_setInterval(sPayload1.toInt());
      //a_pAirTemperatureSensorList[i]->m_setIsRunning(iRunning);
      a_pAirMoistureSensorList[i]->m_setIsRunning(sRunning.toInt());
      return;
    }
  }
  delay(100);
}
// ############################################################################

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
  mqttClient.setServer(MQTT_HOST_ONLINE, MQTT_PORT);
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