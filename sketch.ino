#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Wokwi ofrece esta red virtual sin contrasena.
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";

// EdgeService desplegado para la demo academica de OnControl.
constexpr char API_URL[] =
    "https://edgeservice.onrender.com/OnControl/parameters";
constexpr char DEVICE_ID[] = "smart-band-001";
constexpr char API_KEY[] = "oncontrol-grupo2-demo-key";

constexpr int BPM_PIN = 34;
constexpr int SPO2_PIN = 35;
constexpr int TEMPERATURE_PIN = 32;
constexpr int SEND_BUTTON_PIN = 4;
constexpr int STATUS_LED_PIN = 2;

bool initialMeasurementSent = false;
bool previousButtonState = HIGH;

float mapFloat(int value, int inputMin, int inputMax, float outputMin,
               float outputMax) {
  return outputMin +
         (static_cast<float>(value - inputMin) * (outputMax - outputMin) /
          static_cast<float>(inputMax - inputMin));
}

void connectToWiFi() {
  Serial.print("Conectando a ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }

  Serial.println();
  Serial.print("WiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

void readVitals(float &bpm, float &spo2, float &temperature) {
  long bpmRaw = 0;
  long spo2Raw = 0;
  long temperatureRaw = 0;

  for (int sample = 0; sample < 20; sample++) {
    bpmRaw += analogRead(BPM_PIN);
    spo2Raw += analogRead(SPO2_PIN);
    temperatureRaw += analogRead(TEMPERATURE_PIN);
    delay(10);
  }

  bpm = mapFloat(bpmRaw / 20, 0, 4095, 45.0, 150.0);
  spo2 = mapFloat(spo2Raw / 20, 0, 4095, 82.0, 100.0);
  temperature = mapFloat(temperatureRaw / 20, 0, 4095, 34.0, 40.5);
}

void sendMeasurement() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  float bpm;
  float spo2;
  float temperature;
  readVitals(bpm, spo2, temperature);

  String payload = "{";
  payload += "\"device_id\":\"" + String(DEVICE_ID) + "\",";
  payload += "\"bpm\":" + String(bpm, 1) + ',';
  payload += "\"temp\":" + String(temperature, 2) + ',';
  payload += "\"spo2\":" + String(spo2, 1);
  payload += '}';

  Serial.println();
  Serial.println("Enviando medicion a OnControl:");
  Serial.println(payload);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, API_URL)) {
    Serial.println("No se pudo iniciar la conexion HTTPS");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);

  digitalWrite(STATUS_LED_PIN, HIGH);
  const int statusCode = http.POST(payload);
  const String response = http.getString();
  digitalWrite(STATUS_LED_PIN, LOW);

  Serial.print("Respuesta HTTP: ");
  Serial.println(statusCode);
  Serial.println(response);

  if (statusCode == HTTP_CODE_CREATED) {
    Serial.println("Medicion registrada correctamente.");
  } else {
    Serial.println("La medicion no pudo registrarse.");
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(BPM_PIN, INPUT);
  pinMode(SPO2_PIN, INPUT);
  pinMode(TEMPERATURE_PIN, INPUT);
  pinMode(SEND_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  Serial.println("OnControl - Simulador de banda IoT");
  Serial.println("Los potenciometros representan BPM, SpO2 y temperatura.");
  connectToWiFi();

  // Se envia una sola lectura al iniciar para validar la integracion completa.
  delay(1500);
  sendMeasurement();
  initialMeasurementSent = true;
  Serial.println("Pulsa ENVIAR para registrar otra medicion.");
}

void loop() {
  const bool currentButtonState = digitalRead(SEND_BUTTON_PIN);

  if (initialMeasurementSent && previousButtonState == HIGH &&
      currentButtonState == LOW) {
    sendMeasurement();
  }

  previousButtonState = currentButtonState;
  delay(50);
}
