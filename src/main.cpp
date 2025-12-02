#include <Arduino.h>

#define R 9
#define G 10
#define B 11

// void setup(){
//   	pinMode(R, OUTPUT);
//   	pinMode(G, OUTPUT);
//   	pinMode(B, OUTPUT);
// }

// void loop(){
//   	analogWrite(R, random(255)); 
//     analogWrite(G, random(255));
// 	  analogWrite(B, random(255));
//     delay(200);	
//     // analogWrite(R, 0);
//     // analogWrite(G, 255);
//     // analogWrite(B, 0);
//     // delay(1000);
// }
struct ColorTime {
  int hour;
  int minute;
  int r;
  int g;
  int b;
};


ColorTime schedule[] = {
  {17, 30, 255, 0, 0},   
  {17, 31, 0, 255, 0},   
  {17, 32, 0, 0, 255},  
  {17, 33, 255, 255, 0} 
};

const int numEvents = sizeof(schedule)/sizeof(schedule[0]);
ColorTime currentColor;

unsigned long dayStart;
const unsigned long dayDuration = 24UL * 60UL * 60UL * 1000UL; // 24 uur in ms

int startHour = 0;
int startMinute = 0;


void setup() {
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  Serial.begin(9600);

  String t = String(__TIME__); // bv "14:23:01"
  startHour = t.substring(0,2).toInt();
  startMinute = t.substring(3,5).toInt();

  dayStart = (startHour * 3600UL + startMinute * 60UL) * 1000UL;

  Serial.print("Starttijd automatisch ingesteld op: ");
  Serial.print(startHour);
  Serial.print(":");
  Serial.println(startMinute);
  Serial.println(dayStart);

  
  

  currentColor = schedule[0]; // startkleur
}

void loop() {
  unsigned long now = millis() + dayStart;
  
  // Zorg dat het “een dag” wordt herhaald
  unsigned long timeInDay = now % dayDuration;

    // Bereken gesimuleerde uren, minuten en seconden
  unsigned long totalSeconds = timeInDay / 1000;
  int hours = totalSeconds / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = totalSeconds % 60;

  // Print tijd naar Serial
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
  for (int i = 0; i < numEvents; i++) {
    unsigned long eventTime = (schedule[i].hour * 3600UL + schedule[i].minute * 60UL) * 1000UL;
    if (timeInDay >= eventTime) {
      currentColor = schedule[i];
    }
  }

  // Zet LED naar huidige kleur
  analogWrite(R, currentColor.r);
  analogWrite(G, currentColor.g);
  analogWrite(B, currentColor.b);

  delay(1000); // check elke seconde
}











// #include <Arduino.h>
// #include <Wire.h>
// #include "RTClib.h"
// #include <SPI.h>

// RTC_DS3231 rtc;
// #define rood 9
// #define groen 10
// #define blauw 11

// // === Jouw dagschema ===
// // Tijden in 24u-formaat (HH,MM) met RGB-waarden
// struct KleurMoment {
//   byte uur;
//   byte minuut;
//   byte r;
//   byte g;
//   byte b;
// };

// // Pas dit aan aan jouw schema (max. 10 momenten)
// KleurMoment schema[] = {
//   {15,  47, 255, 180, 100},
//   {15, 48, 255, 255, 255},
//   {15, 49, 255, 120, 80},
//   {15, 50, 100, 60, 40}
// };
// const int AANTAL = sizeof(schema) / sizeof(schema[0]);

// int huidigMoment = -1;

// void setup() {
//   Serial.begin(9600);
//   pinMode(rood, OUTPUT);
//   pinMode(groen, OUTPUT);
//   pinMode(blauw, OUTPUT);

//   if (!rtc.begin()) {
//     Serial.println("RTC niet gevonden!");
//     while (1);
//   }
//   if (rtc.lostPower()) {
//     Serial.println("RTC stroom verloren, tijd instellen!");
//     // Stel tijd in op compile-tijd (eenmalig bij eerste upload)
//     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//   }

//   Serial.println("Dagschema gestart.");
// }

// void stelKleurIn(byte r, byte g, byte b) {
//   analogWrite(rood, r);
//   analogWrite(groen, g);
//   analogWrite(blauw, b);
//   Serial.print("Nieuwe kleur: ");
//   Serial.print(r); Serial.print(", ");
//   Serial.print(g); Serial.print(", ");
//   Serial.println(b);
// }

// void loop() {
//   DateTime nu = rtc.now();
//   Serial.print("Huidige tijd: ");
//   Serial.print(nu.hour()); Serial.print(":");
//   Serial.print(nu.minute()); Serial.print(":");
//   Serial.println(nu.second());
//   // Check of huidige tijd overeenkomt met een schema-tijd
//   for (int i = 0; i < AANTAL; i++) {
//     if (nu.hour() == schema[i].uur && nu.minute() == schema[i].minuut) {
//       if (huidigMoment != i) { // alleen veranderen bij nieuw moment
//         huidigMoment = i;
//         stelKleurIn(schema[i].r, schema[i].g, schema[i].b);
//       }
//     }
//   }

//   delay(5000); // controleer elke 5s
// }

