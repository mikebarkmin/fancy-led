#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "main.hpp"

#define PIN D1
#define NUMBER_OF_PIXEL 10

const char ssid[] = "*";
const char password[] = "*";

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_PIXEL, PIN, NEO_GRB + NEO_KHZ800);

/*
 * 0: Standard-Modus (Farbe und Helligkeit)
 * 1: Pulsieren (Farbe und Helligkeit)
 *
 */
 enum operationModes {
  STANDARD,
  PULSE
 };

 int operationMode = STANDARD;

 struct RGB {
  int red;
  int green;
  int blue;
 };

 struct StandardMode {
  RGB color;
  int brightness;
 };

 struct PulseMode {
  RGB color;
  int minBrightness;
  int maxBrightness;
  int currentBrightness;
  bool up;
  int delayMs;
  int iterations;
 };

StandardMode standardMode;
PulseMode pulseMode;


void setup() {
  // put your setup code here, to run once
  strip.begin();
  strip.show();

  connect();

  RGB color = { 255, 255, 255 };
  standardMode = { color, 255};
  pulseMode = { color, 0, 255, 0, true, 10, 0};

  server.on("/operation", handleOperation);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(1);
  switch(operationMode) {
    case PULSE:
      doOperationPulse();
      break;
    default:
      doOperationStandard();
      break;
  }
  strip.show();
}


void doOperationPulse() {
  int currentBrightness = pulseMode.currentBrightness;
  int maxBrightness = pulseMode.maxBrightness;
  int minBrightness = pulseMode.minBrightness;
  bool up = pulseMode.up;

  if(pulseMode.iterations < pulseMode.delayMs - 1) {
    pulseMode.iterations += 1;
    return;
  }
  pulseMode.iterations = 0;

  if (up && currentBrightness < maxBrightness) {
    pulseMode.currentBrightness = ++currentBrightness;
  }
  if (!up && currentBrightness > minBrightness) {
    pulseMode.currentBrightness = --currentBrightness;
  }

  if (currentBrightness == maxBrightness || currentBrightness == minBrightness) {
    pulseMode.up = !up;
  }

  int red = pulseMode.color.red;
  int green = pulseMode.color.green;
  int blue = pulseMode.color.blue;

  for (int i=0; i < NUMBER_OF_PIXEL; i++) {
    strip.setPixelColor(i, red, green, blue);
  }
  strip.setBrightness(currentBrightness);
}

void doOperationStandard() {
  int brightness = standardMode.brightness;
  int red = standardMode.color.red;
  int green = standardMode.color.green;
  int blue = standardMode.color.blue;

  Serial.println("doOperation");
  Serial.printf("%d - %d - %d", red, green, blue);

  for (int i=0; i < NUMBER_OF_PIXEL; i++) {
    strip.setPixelColor(i, red, green, blue);
  }
  strip.setBrightness(brightness);
  strip.show();
}

void handleOperation() {
  String body = server.arg("plain");
  Serial.print(body);

  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& operation = jsonBuffer.parseObject(body);
  operationMode = operation.get<int>("mode");

  int red = operation["settings"]["color"][0];
  int green = operation["settings"]["color"][1];
  int blue = operation["settings"]["color"][2];

  switch (operationMode) {
    case PULSE: {
      int minBrightness = operation["settings"]["minBrightness"];
      int maxBrightness = operation["settings"]["maxBrightness"];
      int delayMs = operation["settings"]["delayMs"];

      pulseMode.color.red = red;
      pulseMode.color.green = green;
      pulseMode.color.blue = blue;
      pulseMode.minBrightness = minBrightness;
      pulseMode.maxBrightness = maxBrightness;
      pulseMode.currentBrightness = minBrightness;
      pulseMode.up = true;
      pulseMode.delayMs = delayMs;
      pulseMode.iterations = 0;
      break;
    }
    default: {
      int brightness = operation["settings"]["brightness"];
      standardMode.color.red = red;
      standardMode.color.green = green;
      standardMode.color.blue = blue;
      standardMode.brightness = brightness;

      Serial.printf("%d - %d - %d", red, green, blue);;
      break;
    }
  }

  Serial.println(operation.get<int>("mode"));

  server.send(200, "text/plain", "OK");
}

void connect() {
  Serial.begin(9600);
  // Setup wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print Wifi Status
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
