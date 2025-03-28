#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL2l4Fv3boi"
#define BLYNK_TEMPLATE_NAME "Egg Incubator"
#define BLYNK_AUTH_TOKEN "iupLRjpY55H2gfVXAoyJIt7g7YN-UejS"

#include "DHTesp.h"
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#define LCD_COLUMNS  20 
#define LCD_LINES    4
#define HEATER_PIN 32
#define FAN_PIN 33

const int DHT_PIN = 15;

// WiFi credentials for connecting to the Wokwi Guest network
char wifiSSID[] = "Wokwi-GUEST";
char wifiPass[] = "";

DHTesp dhtSensor;

LiquidCrystal_I2C lcd1(0x27, LCD_COLUMNS, LCD_LINES);//for sensor display
LiquidCrystal_I2C lcd2(0x3F, LCD_COLUMNS, LCD_LINES);//for heating and cooling element display 

// Incubator target conditions
float targetTemp = 37.5;      // Target temperature in °C
float tempTolerance = 0.5;    // Acceptable temperature deviation

// (Optional): For humidity control
float targetHumidity = 55.0;      // Target humidity in %
float humidityTolerance = 5.0;    // Allowable range

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);

  Blynk.begin(BLYNK_AUTH_TOKEN, wifiSSID, wifiPass);
  Serial.println("Attempting to connect to Blynk via WiFi...");

  unsigned long startTime = millis();
  while (!Blynk.connected() && millis() - startTime < 10000) {
    Serial.println("Waiting for Blynk connection...");
    delay(500);
  }
  if (Blynk.connected()) {
    Serial.println("Blynk connected successfully!");
  } else {
    Serial.println("Blynk connection failed.");
  }
 
}
void loop() {

  Blynk.run();
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  Serial.println("Temp: " + String(data.temperature, 1) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
  Serial.println("---");
  
  lcd1.setCursor(0, 0);
  lcd1.print("  Temp: " + String(data.temperature, 1) + "\xDF"+"C  ");
  lcd1.setCursor(0, 1);
  lcd1.print(" Humidity: " + String(data.humidity, 1) + "% ");
  lcd1.print("Wokwi Online IoT");

  // Send sensor data to Blynk virtual pins:
  // V1: Temperature, V2: Humidity
  Blynk.virtualWrite(V1, data.temperature);
  Blynk.virtualWrite(V2, data.humidity);

   // Temperature control logic
  if (data.temperature < targetTemp - tempTolerance) {
    // Temperature is too low – turn on heater
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(FAN_PIN, LOW);
    lcd2.setCursor(0, 0);
    lcd2.print("Heater ON                ");
    lcd2.setCursor(0, 1);
    lcd2.print("            ");
    Serial.println("Heater ON");
  } else if ( data.temperature > targetTemp + tempTolerance) {
    // Temperature is too high – activate cooling fan
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(FAN_PIN, HIGH);
    lcd2.setCursor(0, 0);
    lcd2.print("Cooling Fan ON        ");
    lcd2.setCursor(0, 1);
    lcd2.print("         ");
    Serial.println("Cooling Fan ON");
  } else {
    // Temperature is within the target range – turn off both
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);
    lcd2.setCursor(0, 0);
    lcd2.print("Temperature        ");
    lcd2.setCursor(0, 1);
    lcd2.print("Stable      ");
    Serial.println("Temperature Stable");
  }

  delay(1000);
}