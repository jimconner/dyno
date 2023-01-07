/*
  Go Power Systems Dynomometer using Arduino Pro Micro (AtMega 168)

  The dynamometer has a rotation sensor and a load cell.
  The rotation sensor uses a hall sensor and will output two pulses per revolution.
  The load cell is connected to a HX711 24-bit A/D converter.

  This code reads the sensors and outputs the results over the serial port at 115200 bps.

  Jim Conner - 2023 

*/
#include <HX711.h>
#include <FreqCount.h>
#include <EEPROM.h>

#define ms_per_hour  3600000
#define ms_per_min    60000
#define ms_per_sec    1000

const int LOADCELL_SCK_PIN = 3;
const int LOADCELL_DOUT_PIN = 4;
const int RPM_PIN = 5; // This is the timer0 pin used by FreqCount to measure the RPM sensor.
long reading = 0;
long tare_offset;
float cal_mult;
float lbft = 0;
float horsepower = 0;
char incomingByte; // for incoming serial data
volatile byte rpmcount;
unsigned int rpm;
unsigned long now;

HX711 cell;

void rpm_func()
{
  rpmcount++;
}

// the setup function runs once when you press reset or power the board
void setup() {

  tare_offset = EEPROM.get(0, tare_offset); // Byte 0,1 is an int
  cal_mult = EEPROM.get(2, cal_mult); // Bytes 2-6 are the calibration multiplier (float)
  Serial.begin(115200);
  cell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  FreqCount.begin(494); // Count the pulses every half a second (-6ms to calibrate for 10Khz correctly)
}

void printDigits(int digits) {
   // utility function for digital clock display: prints preceding colon and leading 0
   Serial.print(":");
   if(digits < 10)
      Serial.print('0');
      Serial.print(digits);
}

// the loop function runs over and over again forever
void loop() {
  now = millis();
  byte hour = (now / ms_per_hour);
  now -= (hour * ms_per_hour);
  byte minute = (now / ms_per_min);
  now -= (minute * ms_per_min);
  byte second = (now / ms_per_sec);
  Serial.print(hour);
  printDigits(minute);
  printDigits(second);
  Serial.print(", ");
  

  cell.power_up(); // wake up the load cell A/D converter.
    if (FreqCount.available()) 
    {
      unsigned long fcount = FreqCount.read();
      Serial.print("RPM: ");
      rpm = fcount*30; // 1/2 second sampling with four magnets on disc.
      Serial.print(rpm); 
    }
  

  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  reading = cell.get_units(10);
  cell.power_down(); // Put the load cell A/D into low-power mode - we're not going to need it for 1/2 sec.
  long compensated_reading = (float (reading)-tare_offset);
  if (compensated_reading != 0)
    {
      lbft = (float (reading)-tare_offset)/cal_mult;
    } else {
      lbft = 0;
    }

  Serial.print(", lb-ft: ");
  Serial.print(lbft);
  horsepower = lbft * rpm / 5252;
  Serial.print(", HP: ");
  Serial.println(horsepower);

// Check for received bytes on the serial port (if the user wants us to tare or calibrate.)
if (Serial.available() > 0) 
  {
    incomingByte = Serial.read();
    if (incomingByte == 'z') // Set the offset for the zero point (tare)
    {
      tare_offset = reading;
      Serial.println("TARE");
      EEPROM.put(0, tare_offset); // Save the tare offset in eeprom so that it survives power cycles.
    }
    if (incomingByte == 'c')
    {
      long cal_weight = reading;
      Serial.println("CAL_WEIGHT (49.5lb @ 1 ft)");
      cal_mult = (float(reading)-tare_offset)/49.5;
      EEPROM.put(4, cal_mult); // Save calibration weight value eeprom so that it survives power cycles.
    }
  }
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(500);

}
