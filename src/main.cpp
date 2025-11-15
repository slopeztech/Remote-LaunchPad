#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>           // HTTP (redirects to HTTPS)
#include <ESP8266WebServerSecure.h>     // HTTPS server (BearSSL)
#include <ESP8266mDNS.h>

#include "config.h"
#include "certs.h"

ESP8266WebServerSecure serverHttps(443);
ESP8266WebServer serverHttp(80);

// ---------- FUNCTIONS ----------

String getSoftSwitchState()
{
  int buttonState = digitalRead(softSwitchPin);
  return (buttonState == LOW) ? "ON" : "OFF";
}

void activateRelay()
{
  digitalWrite(ledPin, LOW);

  if (RELAY_ACTIVE_LOW)
    digitalWrite(relayPin, LOW);
  else
    digitalWrite(relayPin, HIGH);

  delay(1000);

  if (RELAY_ACTIVE_LOW)
    digitalWrite(relayPin, HIGH);
  else
    digitalWrite(relayPin, LOW);

  digitalWrite(ledPin, HIGH);
}

void handlePing()
{
  String softSwitchState = getSoftSwitchState();
  int wifiSignal = WiFi.RSSI();
  unsigned long uptime = millis() / 1000;
  size_t freeHeap = ESP.getFreeHeap();

  String json = "{";
  json += "\"status\":\"pong\",";
  json += "\"softswitch\":\"" + softSwitchState + "\",";
  json += "\"wifi_signal_dbm\":" + String(wifiSignal) + ",";
  json += "\"free_heap_bytes\":" + String(freeHeap) + ",";
  json += "\"uptime_seconds\":" + String(uptime) + ",";
  json += "\"firmware_version\":\"1.0.0\"";
  json += "}";
  serverHttps.send(200, "application/json", json);

  tone(buzzerPin, 2000, 200);

  Serial.println("Ping received. SoftSwitch: " + softSwitchState);
}

void handleCountdown()
{
  if (!serverHttps.hasArg("seconds"))
  {
    serverHttps.send(400, "application/json", "{\"error\":\"Missing parameter 'seconds'\"}");
    return;
  }

  int seconds = serverHttps.arg("seconds").toInt();
  if (seconds <= 0 || seconds > 60000)
  { 
    serverHttps.send(400, "application/json", "{\"error\":\"Invalid countdown time\"}");
    return;
  }

  String softSwitchState = getSoftSwitchState();

  String msg = "{";
  msg += "\"countdown_started\":" + String(seconds) + ",";
  msg += "\"softswitch\":\"" + softSwitchState + "\"";
  msg += "}";

  serverHttps.send(200, "application/json", msg);

  Serial.printf("Starting countdown: %d s, SoftSwitch: %s\n", seconds, softSwitchState.c_str());

  for (int i = seconds - 1; i >= 0; i--)
  {
    tone(buzzerPin, 2000, 100);
    delay(1000);
  }

  tone(buzzerPin, 2500, 500);

  if (digitalRead(softSwitchPin) == LOW)
  {
    Serial.println("SoftSwitch ON -> Activating relay");
    activateRelay();
  }
  else
  {
    Serial.println("SoftSwitch OFF -> Relay not activated");
  }
}

void handleRootHttps()
{
  serverHttps.send(200, "text/plain", "ESP8266 HTTPS server");
}

void handleRedirect()
{
  String location = String("https://") + WiFi.softAPIP().toString();
  serverHttp.sendHeader("Location", location, true);
  serverHttp.send(301, "text/plain", "Redirecting to HTTPS");
}

void setup()
{
  digitalWrite(ledPin, HIGH);
  pinMode(ledPin, OUTPUT);

  if (RELAY_ACTIVE_LOW)
    digitalWrite(relayPin, HIGH);
  else
    digitalWrite(relayPin, LOW);
  pinMode(relayPin, OUTPUT);

  pinMode(softSwitchPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  Serial.begin(115200);
  delay(1500);

  Serial.println("Starting AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(myIP);

  // configure HTTPS handlers
  serverHttps.on("/", handleRootHttps);
  serverHttps.on("/ping", handlePing);
  serverHttps.on("/countdown", handleCountdown);
  serverHttps.onNotFound([]() {
    serverHttps.send(404, "application/json", "{\"error\":\"Not Found\"}");
  });

  serverHttps.getServer().setRSACert(
    new BearSSL::X509List(serverCert),
    new BearSSL::PrivateKey(serverKey)
  );

  serverHttps.begin();
  Serial.println("HTTPS server started on port 443");

  serverHttp.on("/", handleRedirect);
  serverHttp.begin();
  Serial.println("HTTP server (redirect) started on port 80");
}

void loop()
{
  serverHttps.handleClient();
  serverHttp.handleClient();
}
