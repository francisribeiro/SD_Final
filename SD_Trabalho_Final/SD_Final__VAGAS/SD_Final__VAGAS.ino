// Libs
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// Defines de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE  "WemosRecebeSinal"
#define TOPICO_PUBLISH    "WemosEnviaSinal"
#define ID_MQTT           "WemosHome2"

// Defines dos Pinos
#define ECHO_VAGA_1      D0
#define TRIGGER_VAGA_1   D1
#define ECHO_VAGA_2      D2
#define TRIGGER_VAGA_2   D3
#define ECHO_VAGA_3      D6
#define TRIGGER_VAGA_3   D7

long duracaoV1, distanciaV1;
long duracaoV2, distanciaV2;
long duracaoV3, distanciaV3;

// WIFI
const char* SSID = "Dioka Loka";
const char* PASSWORD = "1234567890";

// MQTT
const char* BROKER_MQTT = "iot.eclipse.org";
int BROKER_PORT = 1883;

// Variáveis e objetos globais
WiFiClient espClient;
PubSubClient MQTT(espClient);
char estadoVaga1 = '0';
char estadoVaga2 = '0';
char estadoVaga3 = '0';

// Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void initSensor();
void manageVaga1();
void manageVaga2();
void manageVaga3();

// Implementações das funções
void setup() {
  // Inicializações
  initSensor();
  initSerial();
  initWiFi();
  initMQTT();
}

// Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial o que está acontecendo.
void initSerial() {
  Serial.begin(115200);
}

// Função: inicia os sensores
void initSensor() {
  pinMode(ECHO_VAGA_1, INPUT);
  pinMode(TRIGGER_VAGA_1, OUTPUT);
  pinMode(ECHO_VAGA_2, INPUT);
  pinMode(TRIGGER_VAGA_2, OUTPUT);
  pinMode(ECHO_VAGA_3, INPUT);
  pinMode(TRIGGER_VAGA_3, OUTPUT);
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
  /*String msg;

    // obtem a string do payload recebido
    for (int i = 0; i < length; i++) {
     char c = (char)payload[i];
     msg += c;
    }

    // Abre Entrada
    if (msg.equals("V1")) {

    }*/
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
  if (estadoVaga1 == '0')
    MQTT.publish(TOPICO_PUBLISH, "1L");

  if (estadoVaga1 == '1')
    MQTT.publish(TOPICO_PUBLISH, "1O");

  if (estadoVaga2 == '0')
    MQTT.publish(TOPICO_PUBLISH, "2L");

  if (estadoVaga2 == '1')
    MQTT.publish(TOPICO_PUBLISH, "2O");

  if (estadoVaga3 == '0')
    MQTT.publish(TOPICO_PUBLISH, "3L");

  if (estadoVaga3 == '1')
    MQTT.publish(TOPICO_PUBLISH, "3O");

  delay(1000);
}

/// Vaga1 ///
void manageVaga1() {
  digitalWrite(TRIGGER_VAGA_1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_VAGA_1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_VAGA_1, LOW);
  duracaoV1 = pulseIn(ECHO_VAGA_1, HIGH);
  distanciaV1 = duracaoV1 / 58.2;
  Serial.println(distanciaV1);
  delay(50);

  if (distanciaV1 <= 4) {
    estadoVaga1 = '1';
  } else {
    estadoVaga1 = '0';
  }
}


/// Vaga2 ///
void manageVaga2() {
  digitalWrite(TRIGGER_VAGA_2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_VAGA_2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_VAGA_2, LOW);
  duracaoV2 = pulseIn(ECHO_VAGA_2, HIGH);
  distanciaV2 = duracaoV2 / 58.2;
  Serial.println(distanciaV2);
  delay(50);

  if (distanciaV2 <= 4) {
    estadoVaga2 = '1';
  } else {
    estadoVaga2 = '0';
  }
}

/// Vaga3 ///
void manageVaga3() {
  digitalWrite(TRIGGER_VAGA_3, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_VAGA_3, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_VAGA_3, LOW);
  duracaoV3 = pulseIn(ECHO_VAGA_3, HIGH);
  distanciaV3 = duracaoV3 / 58.2;
  Serial.println(distanciaV3);
  delay(50);

  if (distanciaV3 <= 4) {
    estadoVaga3 = '1';
  } else {
    estadoVaga3 = '0';
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

  manageVaga1();
  manageVaga2();
  manageVaga3();
}
