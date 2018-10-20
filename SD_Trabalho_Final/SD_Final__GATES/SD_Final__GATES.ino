// Libs
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// Definição de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE  "WemosRecebeSinal"
#define TOPICO_PUBLISH    "WemosEnviaSinal"
#define ID_MQTT           "WemosHome"

// Defines dos Pinos
#define ECHO_ENTRADA      D0
#define ECHO_SAIDA        D2
#define TRIGGER_ENTRADA   D1
#define TRIGGER_SAIDA     D3
#define VERDE             D4
#define VERMELHO          D5
#define PIN_SAIDA         D6
#define PIN_ENTRADA       D7
#define PIN_BUZZER        D8

long duracaoE, distanciaE;
long duracaoS, distanciaS;

int controleEntrada = 0, manualE = 0;
int controleSaida = 0, manualS = 0;
int estadoGlobal = 0;

int v1 = 0, v2 = 0, v3 = 0;

// WIFI
const char* SSID = "Dioka Loka";
const char* PASSWORD = "1234567890";

// MQTT
const char* BROKER_MQTT = "iot.eclipse.org";
int BROKER_PORT = 1883;

// Variáveis e objetos globais
WiFiClient espClient;
PubSubClient MQTT(espClient);
char estadoEntrada = '0', estadoSaida = '0';
Servo entrada, saida;

// Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void initServo();
void manageEntradaSaida(Servo motor, char estado);
void initSensor();
void manageSaida();
void manageEntrada();
void initLeds();
void leds();
void initBuzzer();

// Implementações das funções
void setup() {
  // Inicializações
  initServo();
  initSensor();
  initLeds();
  initBuzzer();
  initSerial();
  initWiFi();
  initMQTT();
}

// Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial o que está acontecendo.
void initSerial() {
  Serial.begin(115200);
}

void initBuzzer() {
  pinMode(PIN_BUZZER, OUTPUT);
}

// Função: define as entradas para os motores servo e os inicializana posição inicial
void initServo() {
  entrada.attach(PIN_ENTRADA);
  entrada.write(0);
  saida.attach(PIN_SAIDA);
  saida.write(0);
}

// Função: inicia os sensores
void initSensor() {
  pinMode(ECHO_ENTRADA, INPUT);
  pinMode(TRIGGER_ENTRADA, OUTPUT);
  pinMode(ECHO_SAIDA, INPUT);
  pinMode(TRIGGER_SAIDA, OUTPUT);
}

void initLeds() {
  pinMode(VERDE, OUTPUT);
  pinMode(VERMELHO, OUTPUT);
}

// Função: inicializa e conecta-se na rede WI-FI desejada
void initWiFi() {
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconectWiFi();
}

// Função: inicializa parâmetros de conexão MQTT(endereço do broker, porta e seta função de callback)
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

// Função: função de callback esta função é chamada toda vez que uma informação de um dos tópicos subescritos chega)
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;

  // obtem a string do payload recebido
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.println(msg);

  // Abre Entrada
  if (msg.equals("EO")) {
    estadoEntrada = '1';
    manualE = 1;
    manageEntradaSaida(entrada, estadoEntrada);
  }

  // Fecha Entrada
  if (msg.equals("EC")) {
    estadoEntrada = '0';
    manualE = 0;
    manageEntradaSaida(entrada, estadoEntrada);
  }

  // Abre Saida
  if (msg.equals("SO")) {
    estadoSaida = '1';
    manualS = 1;
    manageEntradaSaida(saida, estadoSaida);
  }

  // Fecha Saida
  if (msg.equals("SC")) {
    estadoSaida = '0';
    manualS = 0;
    manageEntradaSaida(saida, estadoSaida);
  }

  // vaga1 ocupada
  if (msg.equals("1O")) {
    v1 = 1;
  }

  // vaga1 livre
  if (msg.equals("1L")) {
    v1 = 0;
  }

  // vaga2 ocupada
  if (msg.equals("2O")) {
    v2 = 1;
  }

  // vaga2 livre
  if (msg.equals("2L")) {
    v2 = 0;
  }

    // vaga3 ocupada
  if (msg.equals("3O")) {
    v3 = 1;
  }

  // vaga2 livre
  if (msg.equals("3L")) {
    v3 = 0;
  }
}

// Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//         em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }
}

// Função: reconecta-se ao WiFi
void reconectWiFi() {
  // Se já está conectado a rede WI-FI, nada é feito.
  // Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

// Função: Verifica o estado das conexões WiFI e ao broker MQTT.
//         Em caso de desconexão (qualquer uma das duas), a conexão é refeita.
void VerificaConexoesWiFIEMQTT(void) {
  if (!MQTT.connected())
    reconnectMQTT();

  reconectWiFi();
}

// Função: envia ao Broker o estado atual do output
void EnviaEstadoOutputMQTT(void) {

  // Abre Entrada
  if (estadoEntrada == '0')
    MQTT.publish(TOPICO_PUBLISH, "EC");

  // Fecha Entrada
  if (estadoEntrada == '1')
    MQTT.publish(TOPICO_PUBLISH, "EO");

  // Abre Saida
  if (estadoSaida == '0')
    MQTT.publish(TOPICO_PUBLISH, "SC");

  // Fecha Saida
  if (estadoSaida == '1')
    MQTT.publish(TOPICO_PUBLISH, "SO");

  delay(1000);
}

// Controle de entrada de veículos
void manageEntradaSaida(Servo motor, char estado) {

  // Abre
  if (estado == '1') {
    for (int pos = 0; pos < 90; pos++)  {
      motor.write(pos);
      delay(15);
    }

    // Fecha
  } else if (estado == '0') {
    for (int pos = 90; pos >= 0; pos--)  {
      motor.write(pos);
      delay(15);
    }
  }

  if (estado == '1')
    estadoGlobal = 1;
  else
    estadoGlobal = 0;
}

/// ENTRADA ///
void manageEntrada() {
  digitalWrite(TRIGGER_ENTRADA, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_ENTRADA, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_ENTRADA, LOW);
  duracaoE = pulseIn(ECHO_ENTRADA, HIGH);
  distanciaE = duracaoE / 58.2;
  //Serial.println(distanciaE);
  delay(50);

  Serial.println(v1);
  Serial.println(v2);
  Serial.println(v3);
  
  // Abre
  if (distanciaE <= 4 && estadoEntrada == '0' && manualE == 0 && (v1 + v2 + v3) < 3) {
    estadoEntrada = '1';
    manageEntradaSaida(entrada, estadoEntrada);
    delay(1000);
  }

  // Fecha
  if (distanciaE > 4 && estadoEntrada == '1' && manualE == 0) {
    estadoEntrada = '0';
    manageEntradaSaida(entrada, estadoEntrada);
    delay(1000);
  }
}

/// SAIDA ///
void manageSaida() {
  digitalWrite(TRIGGER_SAIDA, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_SAIDA, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_SAIDA, LOW);
  duracaoS = pulseIn(ECHO_SAIDA, HIGH);
  distanciaS = duracaoS / 58.2;
  // Serial.println(distanciaS);
  delay(50);

  // Abre
  if (distanciaS <= 4 && estadoSaida == '0' && manualS == 0) {
    estadoSaida = '1';
    manageEntradaSaida(saida, estadoSaida);
    delay(1000);
  }

  // Fecha
  if (distanciaS > 4 && estadoSaida == '1' && manualS == 0) {
    estadoSaida = '0';
    manageEntradaSaida(saida, estadoSaida);
    delay(1000);
  }
}
void leds() {
  if (estadoEntrada == 1 || estadoSaida == 1 || estadoGlobal == 1) {
    digitalWrite(VERMELHO, LOW);  // desligar o led vermelho
    digitalWrite(VERDE, HIGH);  // ligar o led verde
    tone(PIN_BUZZER, 1000);
    delay(300);
    digitalWrite(VERMELHO, HIGH);  // ligar o led vermelho
    digitalWrite(VERDE, LOW);  // desligar o led verde
    tone(PIN_BUZZER, 1500);
    delay(400);
  } else {
    digitalWrite(VERMELHO, HIGH);  // ligar o led vermelho
    digitalWrite(VERDE, LOW);  // ligar o led verde
    tone(PIN_BUZZER, 0);
    delay(1);
  }
}

// Programa principal
void loop() {
  // Garante funcionamento das conexões WiFi e ao broker MQTT
  VerificaConexoesWiFIEMQTT();

  // Envia o status de todos os outputs para o Broker no protocolo esperado
  EnviaEstadoOutputMQTT();

  // Keep-alive da comunicação com broker MQTT
  MQTT.loop();

  manageSaida();
  manageEntrada();

  leds();
}
