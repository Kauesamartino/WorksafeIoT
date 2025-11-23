#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ---------- CONFIGURAÇÃO WIFI ----------
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// ---------- CONFIGURAÇÃO MQTT ----------
const char* mqtt_server = "SEU_BROKER_MQTT"; // Ex: "192.168.1.100" ou IP do Mosquitto/Node-RED
WiFiClient espClient;
PubSubClient client(espClient);

// ---------- MAPEAMENTO DE PINOS ----------
#define DHT_PIN 12      
#define DHT_TYPE DHT22
#define POT_PIN 35      
#define LDR_PIN 34      
#define BTN_MOOD 5      
#define BTN_PAUSE 4     
#define LED_ALERT 15    

DHT dht(DHT_PIN, DHT_TYPE);

// ---------- VARIÁVEIS GLOBAIS ----------
float temperatura = 0;
float umidade = 0;
int batimento_simulado = 0;
int luminosidade = 0;
bool moodState = false;
bool pauseState = false;
bool lastMoodBtn = HIGH;
bool lastPauseBtn = HIGH;

// ---------- Funções de conexão ----------
void setupWifi() {
  delay(10);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("conectado!");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 3 segundos.");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BTN_MOOD, INPUT_PULLUP);
  pinMode(BTN_PAUSE, INPUT_PULLUP);
  pinMode(LED_ALERT, OUTPUT);
  dht.begin();
  setupWifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("Sistema Monitor de Bem-Estar iniciado.");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // ------ Leitura dos sensores ------
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
  batimento_simulado = analogRead(POT_PIN);
  luminosidade = analogRead(LDR_PIN);

  // ------ Botão Mood toggle ------
  bool currentMoodBtn = digitalRead(BTN_MOOD);
  if (lastMoodBtn == HIGH && currentMoodBtn == LOW) {
    moodState = !moodState;
    Serial.print("Mood alterado para: ");
    Serial.println(moodState ? "Alerta" : "Bem-estar");
  }
  lastMoodBtn = currentMoodBtn;

  // ------ Botão Pausa toggle ------
  bool currentPauseBtn = digitalRead(BTN_PAUSE);
  if (lastPauseBtn == HIGH && currentPauseBtn == LOW) {
    pauseState = !pauseState;
    Serial.print("Pausa alterada para: ");
    Serial.println(pauseState ? "Solicitada" : "Não solicitada");
  }
  lastPauseBtn = currentPauseBtn;

  // ------ LED alerta ------
  if (temperatura > 28 || umidade < 35 || moodState || pauseState || batimento_simulado > 2000) {
    digitalWrite(LED_ALERT, HIGH);
  } else {
    digitalWrite(LED_ALERT, LOW);
  }

  // ------ Monta e envia JSON via MQTT ------
  StaticJsonDocument<256> doc;
  doc["temperatura"] = temperatura;
  doc["umidade"] = umidade;
  doc["batimento"] = batimento_simulado;
  doc["luminosidade"] = luminosidade;
  doc["mood"] = moodState ? "Alerta" : "Bem-estar";
  doc["pausa"] = pauseState ? "Sim" : "Não";
  char payload[256];
  serializeJson(doc, payload);
  client.publish("worksafeiot/sensores", payload);

  // ------ Debug Serial ------
  Serial.println(payload);

  delay(1000);
}
