#include "room_light_sensor.h"

// Konstruktor
RoomLightSensor::RoomLightSensor(String sName, uint8_t uiDataPin)
{
  // Standard
  m_sName = sName;
  m_uiDataPin = uiDataPin;
  m_setup();
};

void RoomLightSensor::m_setup()
{
  // Standard
  m_uiTimeSinceLastRead = millis();
  m_uiInterval = 1;
  m_uiIsRunning = 0;

  m_uiThreshold = 0;
  m_uiNegativeThreshold = 0;

  bTimeTableAktiv = false;
  m_uiStartTimeInMinutes = 0;
  m_uiEndTimeInMinutes = 0;
  sStartTime = "00:00";
  sEndTime = "00:00";
  
  m_uiLastIsRunning = m_uiIsRunning;
  m_uiLastInterval = m_uiInterval;
  m_uiLastThreshold = m_uiThreshold;
  m_uiLastNegativeThreshold = m_uiNegativeThreshold;

}

uint RoomLightSensor::m_getThreshold() { return m_uiThreshold; }
void RoomLightSensor::m_setThreshold(uint uiVal)
{
  m_uiThreshold = uiVal;
  if (m_uiIsRunning && uiVal != m_uiLastThreshold)
    m_printStatus();
  m_uiLastThreshold = m_uiThreshold;
}

uint RoomLightSensor::m_getNegativeThreshold() { return m_uiNegativeThreshold; }
void RoomLightSensor::m_setNegativeThreshold(uint uiVal)
{
  m_uiNegativeThreshold = uiVal;

  if (m_uiIsRunning && uiVal != m_uiLastNegativeThreshold)
    m_printStatus();
  m_uiLastNegativeThreshold = m_uiNegativeThreshold;
}

// Standard
String *RoomLightSensor::m_getName() { return &m_sName; }

uint RoomLightSensor::m_getInterval() { return m_uiInterval; }
void RoomLightSensor::m_setInterval(uint uiVal)
{
  m_uiInterval = uiVal;
  if (m_uiIsRunning && uiVal != m_uiLastInterval)
    m_printStatus();
  m_uiLastInterval = m_uiInterval;
}

uint8_t RoomLightSensor::m_getIsRunning() { return m_uiIsRunning; }
void RoomLightSensor::m_setIsRunning(uint8_t uiVal)
{
  m_uiIsRunning = uiVal;
  if (uiVal != m_uiLastIsRunning)
  {
    m_printStatus();
  }
  m_uiLastIsRunning = m_uiIsRunning;
}

void RoomLightSensor::m_setRelaisPin(uint8_t uiRelaisPin)
{
  m_uiRelaisPin = uiRelaisPin;
  pinMode(m_uiRelaisPin, OUTPUT);
}

// Extra
void RoomLightSensor::m_printStatus()
{
  Serial.println("######################## RLS Status ###########################################");
  Serial.printf("Sensor \"%s\" status changed. Now it's running with the following parameters: \n", m_sName);
  Serial.printf("\tisRunning:  %i -> %i \n", m_uiLastIsRunning, m_uiIsRunning);
  Serial.printf("\tinterval:  %i -> %i \n", m_uiLastInterval, m_uiInterval);
  Serial.printf("\tthreshold: %i -> %i\n", m_uiLastThreshold, m_uiThreshold);
  Serial.printf("\tthreshold (neg): %i -> %i\n", m_uiNegativeThreshold, m_uiLastNegativeThreshold);
  Serial.println("###############################################################################");
}

// Standard

/**
 * rVal wird ein Wert zwisch 0 - 1024
 * von hell - dunckel
 */
float RoomLightSensor::m_getValue()
{

  float fFactor = 100.0 / 1024.0;
  float rVal = 0;
  int newVal = 0;

  /**
   * Sensor ist aktiv und im Interval dran
   */
  if (m_uiIsRunning && (millis() - m_uiTimeSinceLastRead > (m_uiInterval * 60000)))
  {
    newVal = analogRead(m_uiDataPin);
    if (isnan(newVal))
    {
      Serial.println(m_sName + " Lesefehler am Sensor");
      return 0;
    }

    if (newVal > 0)
    {
      // change to %-Value
      rVal = newVal * fFactor;
      // logical change
      rVal = 100.0 - rVal;
      
      m_setRelais(rVal);

      m_uiTimeSinceLastRead = millis();
    }

    return rVal;
  }  
  return 0.0;
}

void RoomLightSensor::m_setTimeTable(String startTime, String endTime) {
  
  sStartTime = startTime;
  sEndTime = endTime;
  
  m_uiStartTimeInMinutes = (startTime.substring(0,2).toInt() * 60) + startTime.substring(3,5).toInt();
  m_uiEndTimeInMinutes = (endTime.substring(0,2).toInt() * 60) + endTime.substring(3,5).toInt();
  
  if(m_uiStartTimeInMinutes + m_uiEndTimeInMinutes > 0)
    bTimeTableAktiv = true;
  else bTimeTableAktiv = false;

}

/**
 * Hilfsfunktion
 * m_setRelais(float actValue, tm* localTime)
 * @actValue aktueller Messwert/Auslöser
 * @localTime aktuelle Uhrzeit
 * 
 * Das Auslöosen des Relais hängt von 3 Faktoren ab.
 * 1. Gibt es keinen Schwellenwert (0) so gibt es auch keine Aktion.
 * 2. Ist der der Aktuelle Wert >= dem Schwellenwert so gibt es 
 *    auch keine Aktion. 
 * 3. Ist die aktuelle Uhrzeit nicht im Zeitfensetr so gibt es
 *    ebenfalls keine Aktion
*/
void RoomLightSensor::m_setRelais(float actValue) {
  
  
  // Gibt es einen Schwellwert
  if (m_uiThreshold > 0)
  {
    if (actValue < m_uiThreshold && m_isTimeTable())
    {
      // ist der Schwellwert unterschritten,
      // soll dass Licht angehen. z. B. < 40
      digitalWrite(m_uiRelaisPin, HIGH);
      Serial.println("######################## Licht an #############################################");
    }
    else if (actValue > m_uiNegativeThreshold)
    {
      // Schwellwert zum auschalten der Lampe!
      // Damit es aber nicht an dauernd an und  wieder aus geht,
      // sollte der Wert etwas angepasst werden. Also etwas hoeher
      // als der Schwellwert zum Einschalten. z. B. > 45
      digitalWrite(m_uiRelaisPin, LOW);
      Serial.println("######################## Licht aus ############################################");
    }
  }
  else
  {
    // gibt es kein Schwellwert so soll die Lampe auf jedenfall
    // aus sein/gehen!
    digitalWrite(m_uiRelaisPin, LOW);
    Serial.println("######################## Licht aus ############################################");
  }
}

bool RoomLightSensor::m_isTimeTable() {
  
  // Wenn beide auf 00:00 Uhr stehen, spielt der TimeTable kein Rolle
  if(!bTimeTableAktiv) {
    Serial.println("TimeTable is nicht Aktiv");
    return true;
  }

  time(&now);                       // Liest die aktuelle Zeit
  localtime_r(&now, &localTime);    // und schreibt sie in Localtime

  int localTimeInMinutes = (localTime.tm_hour * 60) + localTime.tm_min;
  Serial.println("local Time in Minutes: " + localTimeInMinutes);
  if(localTimeInMinutes >= m_uiStartTimeInMinutes && localTimeInMinutes <= m_uiEndTimeInMinutes) {
    Serial.print("Starminuten: " + m_uiStartTimeInMinutes );
    Serial.println(" Endminuten: " + m_uiEndTimeInMinutes);
    return true;
  }

  
  Serial.printf(" \tTest Uhrzeit: %02d:%02d:%02d \n", localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
  return false;
}