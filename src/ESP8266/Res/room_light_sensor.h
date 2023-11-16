#ifndef RoomLightSensor_h
#define RoomLightSensor_h

#include <Arduino.h>
#include <string.h>
#include <time.h>

//const char* NTP_SERVER = "de.pool.ntp.org"; const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

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
  uint m_uiNegativeThreshold;

  uint8_t m_uiRelaisPin;

  uint m_uiLastIsRunning;
  uint m_uiLastInterval;
  uint m_uiLastThreshold;
  uint m_uiLastNegativeThreshold;

  time_t now;
  tm localTime;

  bool bTimeTableAktiv;
  uint m_uiStartTimeInMinutes;
  uint m_uiEndTimeInMinutes;
  String sStartTime;
  String sEndTime;
  // Standard
  void m_setup();
  void m_printStatus();

public:
  // Standard

  void m_setRelaisPin(uint8_t uiRelaisPin);

  void m_setThreshold(uint uiVal);
  uint m_getThreshold();

  void m_setNegativeThreshold(uint uiVal);
  uint m_getNegativeThreshold();

  String *m_getName();
  void m_setInterval(uint uiVal);
  uint m_getInterval();

  uint8_t m_getIsRunning();
  void m_setIsRunning(uint8_t uiVal);
  float m_getValue();  

  String* m_getStartTime() { return &sStartTime; }
  String* m_getEndTime() { return &sEndTime; }
  void m_setTimeTable(String startTime, String endTime);
private:
  void m_setRelais(float actValue);
  bool m_isTimeTable();
};

#endif