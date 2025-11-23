#include <DHT.h>

// --- Mapeamento de Pinos ---
#define DHT_PIN 12      // DHT22 Data
#define DHT_TYPE DHT22
#define POT_PIN 35      // Potentiometer
#define LDR_PIN 34      // LDR (photoresistor)
#define BTN_MOOD 5      // Button for mood
#define BTN_PAUSE 4     // Button for pause
#define LED_ALERT 15    // LED for alert

DHT dht(DHT_PIN, DHT_TYPE);

// --- Variáveis Globais ---
float temperatura = 0;
float umidade = 0;
int batimento_simulado = 0;
int luminosidade = 0;

// Estado atual dos botões (toggle)
bool moodState = false;
bool pauseState = false;
// Estado anterior para detectar borda
bool lastMoodBtn = HIGH;
bool lastPauseBtn = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(BTN_MOOD, INPUT_PULLUP);
  pinMode(BTN_PAUSE, INPUT_PULLUP);
  pinMode(LED_ALERT, OUTPUT);
  dht.begin();
  Serial.println("Sistema Monitor de Bem-Estar iniciado.");
}

void loop() {
  // Leitura dos sensores
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
  batimento_simulado = analogRead(POT_PIN);
  luminosidade = analogRead(LDR_PIN);

  // Leitura atual dos botões
  bool currentMoodBtn = digitalRead(BTN_MOOD);
  bool currentPauseBtn = digitalRead(BTN_PAUSE);

  // Alterna o estado do Mood a cada clique
  if (lastMoodBtn == HIGH && currentMoodBtn == LOW) {
    moodState = !moodState;
    Serial.print("Mood alterado para: ");
    Serial.println(moodState ? "Alerta" : "Bem-estar");
  }
  lastMoodBtn = currentMoodBtn;

  // Alterna o estado da Pausa a cada clique
  if (lastPauseBtn == HIGH && currentPauseBtn == LOW) {
    pauseState = !pauseState;
    Serial.print("Pausa alterada para: ");
    Serial.println(pauseState ? "Solicitada" : "Não solicitada");
  }
  lastPauseBtn = currentPauseBtn;

  // Lógica de alerta visual (LED acende em condição crítica)
  if (temperatura > 28 || umidade < 35 || moodState || pauseState || batimento_simulado > 2000) {
    digitalWrite(LED_ALERT, HIGH);
  } else {
    digitalWrite(LED_ALERT, LOW);
  }

  // Exibição dos dados no Serial (para dashboard ou debug)
  Serial.print("Temperatura: "); Serial.print(temperatura, 1); Serial.print(" C | ");
  Serial.print("Umidade: "); Serial.print(umidade, 1); Serial.print("% | ");
  Serial.print("Batimento (simulado): "); Serial.print(batimento_simulado); Serial.print(" | ");
  Serial.print("Luminosidade: "); Serial.print(luminosidade); Serial.print(" | ");
  Serial.print("Mood: "); Serial.print(moodState ? "Alerta" : "Bem-estar"); Serial.print(" | ");
  Serial.print("Pausa?: "); Serial.println(pauseState ? "Sim" : "Não");

  delay(1000); // Intervalo curto para resposta rápida ao clique
}
