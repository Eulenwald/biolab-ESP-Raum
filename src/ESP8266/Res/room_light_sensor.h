#ifndef RoomLightSensor_h
#define RoomLightSensor_h

#include <Arduino.h>
#include <string.h>

class RoomLightSensor
{
public:
  RoomLightSensor(String sName, uint8_t uiDataPin);
  ~RoomLightSensor() { ; }
private:
  // Standard
  String m_sName;
  uint8_t m_uiDataPin;

  uint m_uiTimeSinceLastRead;
  uint m_uiInterval;  
  uint8_t m_uiIsRunning;

  uint m_uiThreshold;
  uint m_uiLastThreshold;
  uint8_t m_uiRelaisPin;

  // Standard
  void m_setup();
  void m_printStatus();
public:
  // Standard
  
  void m_setRelaisPin(uint8_t uiRelaisPin);

  // Extra
  // Schwellwerte
  void m_setThreshold(uint uiVal);
  uint m_getThreshold();

  uint m_uiLastIsRunning;
  uint m_uiLastInterval;
public:    
  String *m_getName();
  void m_setInterval(uint uiVal);
  uint m_getInterval();

  uint8_t m_getIsRunning();
  void m_setIsRunning(uint8_t uiVal);
  String m_auslesen();
};

#endif