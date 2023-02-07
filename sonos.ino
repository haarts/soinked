#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "driver/adc.h"
#include "config.h"
#include <Paperdink.h>

PAPERDINK_DEVICE Paperdink;
WiFiUDP Udp;

char packetBuffer[255];

// Valid till 07 Jul 2023
static const char *cert = "-----BEGIN CERTIFICATE-----\n"
                          "MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n"
                          "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
                          "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
                          "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n"
                          "MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n"
                          "BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n"
                          "aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n"
                          "dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n"
                          "AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n"
                          "3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n"
                          "tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n"
                          "Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n"
                          "VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n"
                          "79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n"
                          "c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n"
                          "Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n"
                          "c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n"
                          "UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n"
                          "Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n"
                          "BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n"
                          "A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n"
                          "Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n"
                          "VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n"
                          "ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n"
                          "8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n"
                          "iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n"
                          "Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n"
                          "XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n"
                          "qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n"
                          "VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n"
                          "L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n"
                          "jjxDah2nGN59PRbxYvnKkKj9\n"
                          "-----END CERTIFICATE-----\n";


void setup() {
  Serial.begin(115200);
  Paperdink.begin();
  Paperdink.enable_display();
  /* Clear the background */
  Paperdink.epd.fillScreen(GxEPD_WHITE);
  Paperdink.epd.setTextColor(GxEPD_BLACK);

  if (connectToNetwork(SSID, PASSWORD) != 0) {
    Paperdink.epd.setCursor(0, 10);
    Paperdink.epd.print("No internet connection");
    Paperdink.epd.display();
    Paperdink.deep_sleep_button_wakeup(BUTTON_1_PIN);
  }

  Serial.println(WiFi.localIP());

  if (Udp.begin(LOCALPORT) == 0) {
    Paperdink.epd.setCursor(0, 10);
    Paperdink.epd.print("Couldn't start UDP server");
    Paperdink.epd.display();
    Paperdink.deep_sleep_button_wakeup(BUTTON_1_PIN);
  }
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    readPacket(packetSize);
    display(packetBuffer);
  }

  if (runningTimeExceeded()) {
    Paperdink.epd.setCursor(0, 40);
    Paperdink.epd.print("Sleeping");
    Paperdink.epd.display();
    Paperdink.deep_sleep_button_wakeup(BUTTON_1_PIN);
    Paperdink.disable_everything();
  }
}

uint runningTimeExceeded() {
  if (millis() > 1000 * 60 * 60) {
    return 1;
  }

  return 0;
}

void display(char *content) {
  Paperdink.epd.setCursor(0, 40);
  char *part = strtok(content, ";");
  while (part != NULL) {
    Paperdink.epd.print(part);
    Paperdink.epd.setCursor(0, 60);
    part = strtok(NULL, ";");
  }
  Paperdink.epd.display();
}

void displayBatteryStatus(GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_DRIVER_CLASS::HEIGHT>* display) {
  uint8_t not_charging = digitalRead(CHARGING_PIN);
  adc_power_acquire();
  delay(10);
  adc1_config_width(ADC_WIDTH_BIT_12);
  int batt_adc = adc1_get_raw(BATTERY_VOLTAGE_ADC);
  adc_power_release();
  pcf8574.digitalWrite(BATT_EN, HIGH);
  float batt_voltage = (float)((batt_adc/4095.0)*4.2);
  display->print(batt_voltage);  
}

void readPacket(uint packetSize) {
  Serial.print("Received packet of size ");
  Serial.println(packetSize);
  Serial.print("From ");
  IPAddress remoteIp = Udp.remoteIP();
  Serial.print(remoteIp);
  Serial.print(", port ");
  Serial.println(Udp.remotePort());

  // read the packet into packetBufffer
  int len = Udp.read(packetBuffer, 255);
  // C strings always end with a zero byte
  if (len > 0) {
    packetBuffer[len] = 0;
  }

  Serial.println("Contents:");
  Serial.println(packetBuffer);
}

int connectToNetwork(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);
  int attempts = 40;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (!attempts--) {
      Serial.println("NOT Connected");
      return -1;
    }
  }
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
  return 0;
}

void gatherWeatherData() {
  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client->setCACert(cert);
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
      HTTPClient https;

      Serial.print("[HTTPS] begin...\n");
      if (https.begin(*client, "https://jigsaw.w3.org/HTTP/connection.html")) {  // HTTPS
        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect\n");
      }
      // End extra scoping block
    }
    delete client;
  } else {
    Serial.println("Unable to create client");
  }
}