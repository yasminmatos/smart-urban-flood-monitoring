#include <WiFi.h>
#include <PubSubClient.h>

const char* SSID = "Wokwi-GUEST"; 
const char* PASSWORD = "";
const char* MQTT_BROKER = "broker.hivemq.com"; 
const int MQTT_PORT = 1883;

const char* TOPIC_LEVEL = "mackenzie/urban/flood/level";
const char* TOPIC_STATUS = "mackenzie/urban/flood/status";

const int PIN_TRIG = 5;  
const int PIN_ECHO = 18; 
const int PIN_LED = 2;   

const int LIMIAR_CRITICO = 42; 
const float MARGEM_HISTERESE = 1.05; 
const int TAMANHO_FILTRO = 10;
int leituras[TAMANHO_FILTRO];
int indiceFiltro = 0;

bool alertaAtivo = false;
long ultimaPublicacao = 0;
const int INTERVALO_PUBLICACAO = 5000; 

WiFiClient espClient;
PubSubClient mqttClient(espClient); 

int aplicarMediaMovel(int novaLeitura) {
  leituras[indiceFiltro] = novaLeitura;
  indiceFiltro = (indiceFiltro + 1) % TAMANHO_FILTRO;
  
  long soma = 0;
  for (int i = 0; i < TAMANHO_FILTRO; i++) {
    soma += leituras[i];
  }
  return soma / TAMANHO_FILTRO; 
}

void conectarWiFi() {
  Serial.print("Conectando-se à rede Wi-Fi: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado com sucesso!");
}

void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Tentando estabelecer conexão com o Broker MQTT...");
    
    String clientId = "ESP32Client-Resilience-" + String(random(0, 1000));
    
    if (mqttClient.connect(clientId.c_str(), TOPIC_STATUS, 1, true, "Offline")) {
      Serial.println("Conectado ao Broker HiveMQ!");
      mqttClient.publish(TOPIC_STATUS, "Online", true); 
    } else {
      Serial.print("Falha na conexão. Código: ");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  
  for (int i = 0; i < TAMANHO_FILTRO; i++) leituras[i] = 0;
  
  conectarWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  long duracaoPulse = pulseIn(PIN_ECHO, HIGH);
  int distanciaBruta = duracaoPulse * 0.034 / 2; 
  
  int distanciaFiltrada = aplicarMediaMovel(distanciaBruta);

  if (distanciaFiltrada <= LIMIAR_CRITICO) {
    if (!alertaAtivo) {
      alertaAtivo = true;
      digitalWrite(PIN_LED, HIGH); 
      Serial.println(" [ALERTA] Nível crítico de água detectado!");
    }
  } else if (distanciaFiltrada > (LIMIAR_CRITICO * MARGEM_HISTERESE)) {
    if (alertaAtivo) {
      alertaAtivo = false;
      digitalWrite(PIN_LED, LOW); 
      Serial.println(" [INFO] Nível hídrico estabilizado. Alerta removido.");
    }
  }

  long agora = millis();
  if (agora - ultimaPublicacao > INTERVALO_PUBLICACAO) {
    ultimaPublicacao = agora;
    
    String payload = "{\"distancia\":" + String(distanciaFiltrada) + ",\"alerta\":" + String(alertaAtivo) + "}";
    Serial.print("Publicando telemetria: ");
    Serial.println(payload);
    
    mqttClient.publish(TOPIC_LEVEL, payload.c_str()); 
  }
  
  delay(100); 
}
