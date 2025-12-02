#include <Arduino.h>

#define R 9
#define G 10
#define B 11


// === dagschema ===
// Tijden in 24u-formaat (HH,MM) met RGB-waarden
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


// Aantal gebeurtenissen in het schema
const int numEvents = sizeof(schedule)/sizeof(schedule[0]);
ColorTime currentColor;

// Variabelen voor tijdsberekening
unsigned long dayStart;
const unsigned long dayDuration = 24UL * 60UL * 60UL * 1000UL; // 24 uur in ms

int startHour = 0;
int startMinute = 0;


void setup() {
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  Serial.begin(9600);

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
  // Loop door het schema en vind de laatste gebeurtenis die is gepasseerd
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
