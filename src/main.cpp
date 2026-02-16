#include <Arduino.h>
#include <Wire.h>
#include "DFRobot_INA219.h"
#include <EEPROM.h>


//amperemeter deel
/*!
   file getVoltageCurrentPower.ino
   SEN0291 Gravity: I2C Digital Wattmeter
   The module is connected in series between the power supply and the load to read the voltage, current and power
   The module has four I2C, these addresses are:
   INA219_I2C_ADDRESS1  0x40   A0 = 0  A1 = 0
   INA219_I2C_ADDRESS2  0x41   A0 = 1  A1 = 0
   INA219_I2C_ADDRESS3  0x44   A0 = 0  A1 = 1
   INA219_I2C_ADDRESS4  0x45   A0 = 1  A1 = 1
  
   Copyright    [DFRobot](https://www.dfrobot.com), 2016
   Copyright    GNU Lesser General Public License
   version  V0.1
   date  2019-2-27
*/

DFRobot_INA219_IIC     ina219(&Wire, INA219_I2C_ADDRESS4);

// voor kalibratie: meet de stroom met een multimeter en vergelijk met de waarde van de INA219. Pas deze waarden aan zodat ze overeenkomen (lineaire kalibratie).
float ina219Reading_mA = 1000;
float extMeterReading_mA = 1000;





//EEPROM-deel
// === EEPROM-adres === (permanent geheugen)
#define EEPROM_ADDR 0

// Struct voor het opslaan van cumulatieve stroomgegevens
struct PowerStats {
  uint32_t totalSeconds;
    double currentSum_mA;
};

PowerStats stats; // Variabele om de cumulatieve stroomgegevens bij te houden
// Variabelen voor timing
unsigned long lastSampleMillis = 0;
unsigned long lastEEPROMWriteMillis = 0;
// Intervallen in milliseconden
const unsigned long SAMPLE_INTERVAL = 1000;   // 1s
const unsigned long EEPROM_INTERVAL = 60000;  // 60s

// Functie om cumulatieve stroomgegevens uit EEPROM te laden (zo wordt het vorige totaal hersteld na een reset of stroomuitval)
void loadStatsFromEEPROM() {
  EEPROM.get(EEPROM_ADDR, stats);

  // Eerste keer of corrupte data
  if (stats.totalSeconds == 0xFFFFFFFF) {
    stats.totalSeconds = 0;
    stats.currentSum_mA = 0.0;
  }

  Serial.print("Herstelde runtime (s): ");
  Serial.println(stats.totalSeconds);
}

// Functie om cumulatieve stroomgegevens naar EEPROM te schrijven (zo blijft het totaal behouden bij een reset of stroomuitval)
void saveStatsToEEPROM() {
  EEPROM.put(EEPROM_ADDR, stats);
  Serial.println("Stats opgeslagen in EEPROM");
}




//schema-deel
#define R 9
#define G 10
#define B 11

// === CONSTANTEN ===
const int NUM_DAYS = 14;
const unsigned long DAY_DURATION = 24UL * 60UL * 60UL * 1000UL;

// === dagschema ===
// Tijden in 24u-formaat (HH,MM) met RGB-waarden
struct ColorTime {
  int day;     // 0–13
  int hour;
  int minute;
  int r;
  int g;
  int b;
};


ColorTime schedule[] = {
  {0, 17, 30, 255, 0, 0},   
  {1, 17, 31, 255, 0, 0},   
  {2, 17, 32, 0, 255, 0},  
  {3, 17, 33, 0, 0, 255} 
};


// Aantal gebeurtenissen in het schema
const int numEvents = sizeof(schedule)/sizeof(schedule[0]);
ColorTime currentColor;

// Variabelen voor tijdsberekening
unsigned long dayStart;
const unsigned long dayDuration = 24UL * 60UL * 60UL * 1000UL; // 24 uur in ms

int startHour = 0;
int startMinute = 0;


void setup() {
  // eventueel oude EEPROM-statistieken inladen
  loadStatsFromEEPROM();

  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  Serial.begin(9600);

  while(!Serial);
  
  Serial.println();
  // INA219 initialiseren, kijken of hij reageert, en kalibreren
  while(ina219.begin() != true) {
      Serial.println("INA219 begin faild");
      delay(2000);
  }
  ina219.linearCalibrate(ina219Reading_mA, extMeterReading_mA);
  Serial.println();

  // Stel starttijd in op compile-tijd
  String t = String(__TIME__); // bv "14:23:01"
  startHour = t.substring(0,2).toInt();
  startMinute = t.substring(3,5).toInt();

  // Bereken dagStart in milliseconden
  dayStart = (startHour * 3600UL + startMinute * 60UL) * 1000UL;

  // Print starttijd naar Serial
  Serial.print("Starttijd automatisch ingesteld op: ");
  Serial.print(startHour);
  Serial.print(":");
  Serial.println(startMinute);
  Serial.println(dayStart);

  currentColor = schedule[0]; // startkleur


}

void loop() {
  // Bereken huidige "gesimuleerde" tijd sinds dagStart
  unsigned long verlopen = millis() + dayStart;
  
  // Bereken huidige dag in het schema
  int currentDay = (verlopen / DAY_DURATION) % NUM_DAYS;
  // Zorg dat het “een dag” wordt herhaald
  unsigned long timeInDay = verlopen % dayDuration;

    // Bereken gesimuleerde uren, minuten en seconden
  unsigned long totalSeconds = timeInDay / 1000;
  int hours = totalSeconds / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = totalSeconds % 60;

  // Print tijd naar Serial
  Serial.print("Dag ");
  Serial.print(currentDay);
  Serial.print("Tijd: ");
  if(hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if(minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if(seconds < 10) Serial.print("0");
  Serial.println(seconds);

  // Check welk schema actief is
  // Loop door het schema en vind de laatste gebeurtenis die is gepasseerd
  for (int i = 0; i < numEvents; i++) {
    if (schedule[i].day == currentDay)
    {
      unsigned long eventTime = (schedule[i].hour * 3600UL + schedule[i].minute * 60UL) * 1000UL;
      if (timeInDay >= eventTime) {
        currentColor = schedule[i];
      }
    }
  }

  // Zet LED naar huidige kleur
  analogWrite(R, currentColor.r);
  analogWrite(G, currentColor.g);
  analogWrite(B, currentColor.b);

  // Print stroommetingen naar Serial, ter controle van de werking van de INA219
  Serial.print("BusVoltage:   ");
  Serial.print(ina219.getBusVoltage_V(), 2);
  Serial.println("V");
  Serial.print("ShuntVoltage: ");
  Serial.print(ina219.getShuntVoltage_mV(), 3);
  Serial.println("mV");
  Serial.print("Current:      ");
  Serial.print(ina219.getCurrent_mA(), 1);
  Serial.println("mA");
  Serial.print("Power:        ");
  Serial.print(ina219.getPower_mW(), 1);
  Serial.println("mW");
  Serial.println("");

  //EEPROM-deel
  unsigned long nowMillis = millis();

  // 1x per seconde meten en cumulatieve stroomgegevens bijwerken
  if (nowMillis - lastSampleMillis >= SAMPLE_INTERVAL) {
    lastSampleMillis = nowMillis;

    float current_mA = ina219.getCurrent_mA();

    stats.currentSum_mA += current_mA;
    stats.totalSeconds++;
  }

  // 1x per minuut cumulatieve stroomgegevens naar EEPROM schrijven
  if (nowMillis - lastEEPROMWriteMillis >= EEPROM_INTERVAL) {
    lastEEPROMWriteMillis = nowMillis;
    saveStatsToEEPROM();
  }

  // Gemiddelde stroom berekenen en printen
  if (stats.totalSeconds > 0) {
    double avgCurrent = stats.currentSum_mA / stats.totalSeconds;

    Serial.print("Gemiddelde stroom (mA): ");
    Serial.println(avgCurrent, 2);
  }

  delay(500);
}
