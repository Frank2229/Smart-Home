/*
  Author: Frank Glaser
  Device: Average Desk Lamp

  This program will control when my study lamp will turn on.
  When triggered, it is time to get my butt in gear and hit the books.
  Attached is an mini OLED display to keep track of the time.
*/

#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "time.h"

#define LED 4
#define RELAY 13
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// VARIABLES DETERMINED BY THE MASTER
const int gmtOffsetMult = -5;
const int daylightOffsetMult = 1;
const int studyTimeStart_Hour = 18;
const int studyTimeStart_Minute = 0;
const int studyTimeEnd_Hour = 20;
const int studyTimeEnd_Minute = 59;

// INITIALIZATION OF TOUCH SENSOR VARIABLES.
const int TOUCHSENSOR_MANUAL = 15;
const int TOUCHSENSOR_TOGGLE = 2;
bool isManualPressed = false;
bool isManualMode = false;
bool isTogglePressed = false;
bool isToggleOn = false;

// WIFI VARIABLES
const char* ssid = "Stevens-IoT";
const char* password = "jPT563VQM3";
const char* ntpServer = "in.pool.ntp.org";

// INITIALIZATION OF TIME VARIABLES
const long secondsPerHour = 3600;
bool isStudyTime = false;
int hour = 0;
int minute = 0;
char* dayPart;
int secondIncrementor = 0;

// SETUP OLED DISPLAY
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  // Initialization of the LED.
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Initialization of the lamp relay.
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  // Initialization of the touch sensors.
  pinMode(TOUCHSENSOR_MANUAL, INPUT);
  pinMode(TOUCHSENSOR_TOGGLE, INPUT);

  // Initialization of the OLED w/ introduction message.
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Smart Lamp");
  display.println("By: Frank");
  display.println("    Glaser");
  display.display();

  // Connect to internet to retrieve world date and time.
  // Adjust time based on local time zone and daylight savings.
  // After date and time is found and stored, sever wifi connection.
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  configTime((secondsPerHour * gmtOffsetMult), (secondsPerHour * daylightOffsetMult), ntpServer);
  assessStudyTime();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  if (digitalRead(TOUCHSENSOR_MANUAL) == HIGH) isManualPressed = true;

  if (isManualPressed && digitalRead(TOUCHSENSOR_MANUAL) == LOW) {
    if (isManualMode) {
      isManualMode = false;
      digitalWrite(LED, LOW);
    }
    else {
      isManualMode = true;
      digitalWrite(LED, HIGH);
    }
    isManualPressed = false;
  }

  if (isManualMode) {
    if (digitalRead(TOUCHSENSOR_TOGGLE) == HIGH) isTogglePressed = true;

    Serial.println("MANUAL MODE");

    if (isTogglePressed && digitalRead(TOUCHSENSOR_TOGGLE) == LOW) {
      if (isToggleOn) {
        isToggleOn = false;
        digitalWrite(RELAY, LOW);
      }
      else {
        isToggleOn = true;
        digitalWrite(RELAY, HIGH);
      }
      isTogglePressed = false;
    }
  }

  if (secondIncrementor == 200000) {
    assessStudyTime();

    if (!isManualMode) {
      if (isStudyTime) digitalWrite(RELAY, HIGH);
      else digitalWrite(RELAY, LOW);
    }

    // Format to normal am/pm time.
    if (hour > 12) {
      hour -= 12;
      dayPart = "pm";
    }
    else if (hour == 0) {
      dayPart = "am";
      hour = 12;
    }
    else if (hour == 12) dayPart = "pm";
    else dayPart = "am";
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(8, 7);
    display.print(hour);
    display.print(":");
    if (minute < 10) display.print("0");
    display.print(minute);
    display.print(dayPart);
    display.display();
  
    secondIncrementor = 0;
  }
  else secondIncrementor++;
}

// This function will initially set the current time retrieved from the internet.
// Afterwards, it will update every two seconds from the stored time and date data.
// If the time is within the study interval, return true.
void assessStudyTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  if (hour >= studyTimeStart_Hour && minute >= studyTimeStart_Minute && hour <= studyTimeEnd_Hour && minute <= studyTimeEnd_Minute) isStudyTime = true;
  else isStudyTime = false;
}
