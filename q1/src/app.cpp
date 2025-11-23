/*
 Kaue Vinicius Samartino da Silva - 559317
 João dos Santos Cardoso de Jesus - 560400
 Davi Praxedes Santos Silva - 560719
*/

#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

// Definições de pinos
#define DHTPIN 12
#define DHTTYPE DHT22
#define POTPIN 35
#define LDRPIN 34
#define BTN_MOOD 5
#define BTN_PAUSE 4
#define LED_ALERT 15

// Config Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configuração ThingSpeak
WiFiClient client;
unsigned long channelID = SEU_CHANNEL_ID; // Substitua pelo seu Channel ID
const char* writeAPIKey = "SUA_WRITE_API_KEY"; // Substitua pela sua Write API Key


DHT dht(DHTPIN, DHTTYPE);

// Variáveis
float temperatura = 0;
float umidade = 0;
int batimento_simulado = 0;
int luminosidade = 0;
bool moodState = false;
bool pauseState = false;
bool lastMoodBtn = HIGH;
bool lastPauseBtn = HIGH;

void setup() {
  pinMode(LED_ALERT, OUTPUT);
  pinMode(BTN_MOOD, INPUT_PULLUP);
  pinMode(BTN_PAUSE, INPUT_PULLUP);
  pinMode(POTPIN, INPUT);
  pinMode(LDRPIN, INPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.print("Conectando ao WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  dht.begin();
  ThingSpeak.begin(client);
}

void loop() {
  // Leitura dos sensores
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
  batimento_simulado = map(analogRead(POTPIN), 0, 4095, 40, 180);
  luminosidade = analogRead(LDRPIN);

  // Botões toggle
  bool currentMoodBtn = digitalRead(BTN_MOOD);
  if (lastMoodBtn == HIGH && currentMoodBtn == LOW) {
    moodState = !moodState;
    Serial.print("Mood alterado para: ");
    Serial.println(moodState ? "Alerta" : "Bem-estar");
  }
  lastMoodBtn = currentMoodBtn;

  bool currentPauseBtn = digitalRead(BTN_PAUSE);
  if (lastPauseBtn == HIGH && currentPauseBtn == LOW) {
    pauseState = !pauseState;
    Serial.print("Pausa alterada para: ");
    Serial.println(pauseState ? "Solicitada" : "Não solicitada");
  }
  lastPauseBtn = currentPauseBtn;

  // LED alerta
  if (temperatura > 28 || umidade < 35 || moodState || pauseState || batimento_simulado > 120) {
    digitalWrite(LED_ALERT, HIGH);
  } else {
    digitalWrite(LED_ALERT, LOW);
  }

  // Prepara campos ThingSpeak
  ThingSpeak.setField(1, temperatura);
  ThingSpeak.setField(2, umidade);
  ThingSpeak.setField(3, batimento_simulado);
  ThingSpeak.setField(4, luminosidade);
  ThingSpeak.setField(5, moodState ? 1 : 0);
  ThingSpeak.setField(6, pauseState ? 1 : 0);

  int status = ThingSpeak.writeFields(channelID, writeAPIKey);

  if (status == 200) {
    Serial.println("Dados enviados com sucesso para o canal ThingSpeak!");
  } else {
    Serial.print("Erro ao enviar: ");
    Serial.println(status);
  }

  // Respeita limite ThingSpeak
  delay(20000); 
}
