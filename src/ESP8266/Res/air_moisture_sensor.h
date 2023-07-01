#ifndef AirMoistureSensor_h
#define AirMoistureSensor_h

#include <Arduino.h>
#include <DHT.h>

class AirMoistureSensor
{
public:
  // Standard
  AirMoistureSensor(String sName, uint8_t uiDataPin);
  ~AirMoistureSensor();
private:
  // Standard
  String m_sName;
  uint8_t m_uiDataPin;

  uint m_uiTimeSinceLastRead;
  uint m_uiInterval;  
  uint8_t m_uiIsRunning;


  DHT *m_pDHT;

  // Standard
  void m_setup();
  
  uint m_uiLastIsRunning;
  uint m_uiLastInterval;
public:    
  String *m_getName();
  void m_setInterval(uint uiVal);
  uint m_getInterval();

  uint8_t m_getIsRunning();
  void m_setIsRunning(uint8_t uiVal); 
  void m_printStatus();
  String m_auslesen();
};

#endif