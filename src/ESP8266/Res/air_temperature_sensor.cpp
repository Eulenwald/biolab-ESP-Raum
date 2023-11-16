#include "air_temperature_sensor.h"

#define DHTTYPE DHT22 // there are multiple kinds of DHT sensors
AirTemperatureSensor::AirTemperatureSensor(String sName, uint8_t uiDataPin)
{
  // Standard
  m_sName = sName;
  m_uiDataPin = uiDataPin;  
  m_setup();
};
AirTemperatureSensor::~AirTemperatureSensor() {
  if(m_pDHT)
    delete m_pDHT;
}

void AirTemperatureSensor::m_setup()
{  
  // Standard
  m_uiTimeSinceLastRead = millis();
  m_uiInterval = 1;
  m_uiIsRunning = 0;
  m_uiLastIsRunning = m_uiIsRunning;
  m_uiLastInterval = m_uiInterval;
  // Extra
  m_pDHT = new DHT(m_uiDataPin, DHTTYPE);
  m_pDHT->begin();
}

// Standard
String* AirTemperatureSensor::m_getName() { return &m_sName; }

uint AirTemperatureSensor::m_getInterval() { return m_uiInterval; }
void AirTemperatureSensor::m_setInterval(uint uiVal)
{
  m_uiInterval = uiVal;
  if(m_uiIsRunning && uiVal != m_uiLastInterval)
    m_printStatus();
  m_uiLastInterval = m_uiInterval;
}

uint8_t AirTemperatureSensor::m_getIsRunning() { return m_uiIsRunning; }
void AirTemperatureSensor::m_setIsRunning(uint8_t uiVal)
{ 
  m_uiIsRunning = uiVal; 
  if(uiVal != m_uiLastIsRunning)
    m_printStatus();
  m_uiLastIsRunning = m_uiIsRunning;
}

// Extra
void AirTemperatureSensor::m_printStatus() {
  Serial.println("###############################################################################");
  Serial.printf( "Sensor %s is running with the following parameters: \n\tinterval: %i -> %i\n",
   m_sName, m_uiLastInterval, m_uiInterval);  
  Serial.println("###############################################################################");  
}


// Standard
float AirTemperatureSensor::m_getValue() {
  
  if (m_uiIsRunning && (millis() - m_uiTimeSinceLastRead > (m_uiInterval * 60000)))  
  {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    // Read temperature as Celsius (the default)
    float rVal = m_pDHT->readTemperature();

    if (isnan(rVal))
    {
      Serial.println(m_sName + " Lesefehler am Sensor");
      return 0;
    }

    // Absender
    m_uiTimeSinceLastRead = millis();
    return rVal;
  }
  
  return 0;
}