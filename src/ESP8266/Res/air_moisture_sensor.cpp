#include "air_moisture_sensor.h"

#define DHTTYPE DHT22 // there are multiple kinds of DHT sensors
AirMoistureSensor::AirMoistureSensor(String sName, uint8_t uiDataPin)
{
  // Standard
  m_sName = sName;
  m_uiDataPin = uiDataPin;  
  m_setup();
};
AirMoistureSensor::~AirMoistureSensor() {
  if(m_pDHT)
    delete m_pDHT;
}

void AirMoistureSensor::m_setup()
{  
  // Standard
  m_uiTimeSinceLastRead = 0;
  m_uiInterval = 5;
  m_uiIsRunning = 0;
  m_uiLastIsRunning = m_uiIsRunning;
  m_uiLastInterval = m_uiInterval;
  // Extra
  m_pDHT = new DHT(m_uiDataPin, DHTTYPE);
  m_pDHT->begin();
}

// Standard
String* AirMoistureSensor::m_getName() { return &m_sName; }

uint AirMoistureSensor::m_getInterval() { return m_uiInterval; }
void AirMoistureSensor::m_setInterval(uint uiVal)
{
  m_uiInterval = uiVal;
  if(m_uiIsRunning && uiVal != m_uiLastInterval)
    m_printStatus();
  m_uiLastInterval = m_uiInterval;
}

uint8_t AirMoistureSensor::m_getIsRunning() { return m_uiIsRunning; }
void AirMoistureSensor::m_setIsRunning(uint8_t uiVal)
{ 
  m_uiIsRunning = uiVal; 
  if(uiVal != m_uiLastIsRunning)
    m_printStatus();
  m_uiLastIsRunning = m_uiIsRunning;
}

// Extra
void AirMoistureSensor::m_printStatus() {
  Serial.println("###############################################################################");
  Serial.printf( "Sensor %s is running with the following parameters: \n\tinterval: %i -> %i\n",
   m_sName, m_uiLastInterval, m_uiInterval);  
  Serial.println("###############################################################################");  
}


// Standard
String AirMoistureSensor::m_auslesen()
{

  if (m_uiIsRunning && m_uiTimeSinceLastRead > (m_uiInterval * 100))
  {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = m_pDHT->readHumidity();
    // Read temperature as Celsius (the default)
    float t = m_pDHT->readTemperature();

    if (isnan(h) || isnan(t))
    {
      Serial.println(m_sName + " Lesefehler am Sensor");
      return "";
    }
    float hi = m_pDHT->computeHeatIndex(t, h, false);

    // Absender
    String rVal = m_sName;    
    // Payload
    rVal.concat(',');
    rVal.concat(h);

    m_uiTimeSinceLastRead = 0;
    return rVal;
  }

  m_uiTimeSinceLastRead += 100;
  return "";
}
