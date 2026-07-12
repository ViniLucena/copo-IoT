#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <certificados.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <VL53L0X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

#define ID_COPO 1 //Só trocar aqui pra mudar o copo

VL53L0X tof;
OneWire oneWire(4);
DallasTemperature tempSensor(&oneWire);
Adafruit_NeoPixel leds(30, 18, NEO_GRB + NEO_KHZ800);
Adafruit_INA219 ina219;
WiFiClientSecure conexaoSegura;
unsigned long instanteAnterior = 0;
MQTTClient mqtt(1000);
JsonDocument nivel;
String textoNivel;

unsigned long tLeitura = 0;
unsigned long tMqtt = 0;
unsigned long tAnim = 0;
unsigned long inicioGol = 0;

int distanciaMm = 0;
float quantidadeMl = 0;
float temperaturaC = 0;

bool golAtivo = false;
String timeGol = "";
int frame = 0;

struct TimeInfo {
  const char* nome;
  const char* aliases;
  uint8_t cor[3][3];
};

TimeInfo times[] = {
  {"mexico", "mexico,méxico", {{0,160,70},{255,255,255},{220,0,0}}},
  {"south_africa", "africa do sul,áfrica do sul,south africa", {{0,150,70},{255,220,0},{220,0,0}}},
  {"south_korea", "coreia,south korea,korea", {{255,255,255},{220,0,0},{0,0,220}}},
  {"czechia", "tchequia,czech,czechia", {{255,255,255},{220,0,0},{0,0,220}}},
  {"canada", "canada,canadá", {{255,0,0},{255,255,255},{255,0,0}}},
  {"bosnia", "bosnia,bósnia", {{0,60,255},{255,220,0},{255,255,255}}},
  {"qatar", "qatar,catar", {{120,0,35},{255,255,255},{120,0,35}}},
  {"switzerland", "switzerland,suica,suíça", {{255,0,0},{255,255,255},{255,0,0}}},
  {"brazil", "brasil,brazil", {{0,180,0},{255,220,0},{0,0,255}}},
  {"morocco", "marrocos,morocco", {{220,0,0},{0,160,70},{220,0,0}}},
  {"haiti", "haiti,haití", {{0,0,220},{220,0,0},{255,255,255}}},
  {"scotland", "escocia,escócia,scotland", {{0,80,255},{255,255,255},{0,80,255}}},
  {"usa", "usa,estados unidos,united states", {{220,0,0},{255,255,255},{0,0,220}}},
  {"paraguay", "paraguai,paraguay", {{220,0,0},{255,255,255},{0,0,220}}},
  {"australia", "australia,austrália", {{0,0,220},{255,255,255},{220,0,0}}},
  {"turkey", "turquia,turkey", {{255,0,0},{255,255,255},{255,0,0}}},
  {"germany", "alemanha,germany", {{20,20,20},{220,0,0},{255,200,0}}},
  {"curacao", "curacao,curaçao", {{0,80,255},{255,220,0},{255,255,255}}},
  {"ivory_coast", "costa do marfim,ivory coast", {{255,120,0},{255,255,255},{0,160,70}}},
  {"ecuador", "equador,ecuador", {{255,220,0},{0,0,220},{220,0,0}}},
  {"netherlands", "holanda,netherlands", {{255,90,0},{255,255,255},{0,0,220}}},
  {"japan", "japao,japão,japan", {{255,255,255},{220,0,0},{255,255,255}}},
  {"sweden", "suecia,suécia,sweden", {{0,80,255},{255,220,0},{0,80,255}}},
  {"tunisia", "tunisia,tunísia", {{220,0,0},{255,255,255},{220,0,0}}},
  {"belgium", "belgica,bélgica,belgium", {{20,20,20},{255,220,0},{220,0,0}}},
  {"egypt", "egito,egypt", {{220,0,0},{255,255,255},{20,20,20}}},
  {"iran", "iran,irã", {{0,160,70},{255,255,255},{220,0,0}}},
  {"new_zealand", "nova zelandia,nova zelândia,new zealand", {{0,0,160},{220,0,0},{255,255,255}}},
  {"spain", "espanha,spain", {{220,0,0},{255,220,0},{220,0,0}}},
  {"cape_verde", "cabo verde,cape verde", {{0,0,180},{255,255,255},{220,0,0}}},
  {"saudi_arabia", "arabia saudita,arábia saudita,saudi", {{0,160,70},{255,255,255},{0,160,70}}},
  {"uruguay", "uruguai,uruguay", {{0,100,255},{255,255,255},{255,220,0}}},
  {"france", "franca,frança,france", {{0,0,255},{255,255,255},{255,0,0}}},
  {"senegal", "senegal", {{0,170,70},{255,220,0},{220,0,0}}},
  {"iraq", "iraque,iraq", {{220,0,0},{255,255,255},{20,20,20}}},
  {"norway", "noruega,norway", {{255,0,0},{255,255,255},{0,0,255}}},
  {"argentina", "argentina", {{0,150,255},{255,255,255},{255,220,0}}},
  {"algeria", "argelia,argélia,algeria", {{0,160,70},{255,255,255},{220,0,0}}},
  {"austria", "austria,áustria", {{220,0,0},{255,255,255},{220,0,0}}},
  {"jordan", "jordania,jordânia,jordan", {{220,0,0},{20,20,20},{0,150,70}}},
  {"portugal", "portugal", {{0,160,70},{220,0,0},{255,220,0}}},
  {"dr_congo", "congo,dr congo", {{0,160,255},{255,220,0},{220,0,0}}},
  {"uzbekistan", "uzbequistao,uzbequistão,uzbekistan", {{0,150,255},{255,255,255},{0,170,70}}},
  {"colombia", "colombia,colômbia", {{255,220,0},{0,0,220},{220,0,0}}},
  {"england", "inglaterra,england", {{255,255,255},{220,0,0},{255,255,255}}},
  {"croatia", "croacia,croácia,croatia", {{220,0,0},{255,255,255},{0,0,220}}},
  {"ghana", "ghana,gana", {{220,0,0},{255,220,0},{0,150,70}}},
  {"panama", "panama,panamá", {{255,255,255},{0,0,220},{220,0,0}}}
};


void setLed(int i, uint8_t r, uint8_t g, uint8_t b) {
  leds.setPixelColor(i, leds.Color(r, g, b));
}

void todos(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < 30; i++) setLed(i, r, g, b);
  leds.show();
}

void mostrarNivel() {
  int n = map((int)quantidadeMl, 0, 600, 0, 30);

  for (int i = 0; i < 30; i++) {
    if (i < n) setLed(i, 0, 180, 0);
    else setLed(i, 0, 0, 0);
  }

  leds.show();
}

void lerSensores() {
  int leitura = tof.readRangeSingleMillimeters();

  if (!tof.timeoutOccurred() && leitura > 0 && leitura < 1000) {
    distanciaMm = leitura;
    float pct = 100.0 * (200 - distanciaMm) / (200 - 60);
    pct = constrain(pct, 0.0, 100.0);
    quantidadeMl = 600.0 * pct / 100.0;
  } else {
    distanciaMm = leitura;
    quantidadeMl = 0;
  }

  tempSensor.requestTemperatures();
  temperaturaC = tempSensor.getTempCByIndex(0);
}

int acharTime(String msg) {
  for (int i = 0; i < (int)(sizeof(times) / sizeof(times[0])); i++) {
    String lista = times[i].aliases;
    int pos = 0;

    while (pos >= 0) {
      int fim = lista.indexOf(',', pos);
      String alias = fim < 0 ? lista.substring(pos) : lista.substring(pos, fim);

      if (alias.length() > 0 && msg.indexOf(alias) >= 0) return i;

      pos = fim < 0 ? -1 : fim + 1;
    }
  }

  return -1;
}

void animarGol() {
  int idx = acharTime(timeGol);

  if (idx < 0) {
    todos(255, 180, 0);
  } else {
    int deslocamento = frame;

    if (String(times[idx].nome) == "france") deslocamento = frame / 2;

    for (int i = 0; i < 30; i++) {
      int c = (i + deslocamento) % 3;
      setLed(i, times[idx].cor[c][0], times[idx].cor[c][1], times[idx].cor[c][2]);
    }

    leds.show();
  }

  frame++;
}

String mensagemRecebida(String payload) {
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, payload);

  if (!erro) {
    String m = doc["mensagem"] | "";
    String t = doc["time"] | "";
    payload = m + " " + t;
  }

  payload.toLowerCase();
  return payload;
}

void receberMqtt(String &topico, String &payload) {
  if (topico == "copoAlertas") {
    String msg = mensagemRecebida(payload);

    if (msg.indexOf("parar") >= 0 || msg.indexOf("stop") >= 0 || msg.indexOf("normal") >= 0) {
      golAtivo = false;
      todos(0, 0, 0);
    } else if (msg.indexOf("gol") >= 0) {
      golAtivo = true;
      timeGol = msg;
      inicioGol = millis();
      frame = 0;
    }
  }
}

void publicarDados() {
  JsonDocument dados;
  dados["id_copo"] = ID_COPO;
  dados["quantidade_ml"] = quantidadeMl;
  dados["temperatura_c"] = temperaturaC;

  String json;
  serializeJson(dados, json);

  String topicoDados = "copo/" + String(ID_COPO) + "/dados"; //Mudei aqui para criar o nome do topico com o número do copo Victor

  mqtt.publish(topicoDados.c_str(), json);
}

void reconectarWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin("LabIoT", "4n1m4l5@))!!");
    Serial.print("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(1000);
    }
    Serial.print("conectado!\nEndereço IP: ");
    Serial.println(WiFi.localIP());
  }
}


void reconectarMQTT() {


  if (!mqtt.connected()) {
    Serial.print("Conectando MQTT...");
    while(!mqtt.connected()) {
      mqtt.connect("Copo", "aula", "zowmad-tavQez");
      Serial.print(".");
      delay(1000);
    }
    Serial.println(" conectado!");
   
    mqtt.subscribe("copoAlertas");    
  }
}


void setup() {
 
  Serial.println("========== Leitura da Bateria do copo ==========");


  Serial.begin(115200);
  delay(500);
  reconectarWiFi();
  conexaoSegura.setCACert(certificado1);


  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.onMessage(receberMqtt);
  mqtt.setKeepAlive(10);
  mqtt.setWill("tópico da desconexão", "conteúdo");




  // Inicializa os pinos I2C uma única vez para TODO MUNDO
  Wire.begin(8, 9);  // SDA no 8, SCL no 9




  // Inicializa o medidor de corrente (ele busca automaticamente o endereço 0x40)
  if (!ina219.begin()) {

    Serial.println("Failed to find INA219 chip");
  }
  ina219.setCalibration_16V_400mA();


  leds.begin();
  leds.setBrightness(40);
  todos(0, 0, 0);

  tof.setTimeout(500);
  if (tof.init()) {
    Serial.println("VL53L0X (Distância) OK!");
    tof.setSignalRateLimit(0.1);
    tof.setMeasurementTimingBudget(200000);
  } else {
    Serial.println("AVISO: VL53L0X não encontrado. Ignorando config...");
  }

  tempSensor.begin();



  reconectarMQTT();
}


void loop() {


  reconectarWiFi();
  reconectarMQTT();
  mqtt.loop();


  unsigned long instanteAtual = millis();
  if (instanteAtual > instanteAnterior + 10000) { //  A cada 10 segundos
   
    float corrente_mA = ina219.getCurrent_mA();
    float tensao_V = ina219.getBusVoltage_V();
    float tensao_resistorShunt_mV = ina219.getShuntVoltage_mV();
    float tensao_real = tensao_V + (tensao_resistorShunt_mV / 1000);
    float power_mW = tensao_V * corrente_mA;


    //String T = "Tensão Real = " + String(tensao_real);
   
    int porcentagem = (tensao_real- 3.0)/(1.2) *100;


    String porcentagem_txt = (String(porcentagem)+"%");


    String T_porcento = "Tensão Real = " + String(porcentagem_txt);


    Serial.println("====================");
    Serial.println(T_porcento);
    Serial.println("====================");


    nivel["id_copo"] = ID_COPO;
    nivel["valor"] = porcentagem;
    serializeJson(nivel, textoNivel);
    //Serial.println(textoNivel);
    mqtt.publish("nivel_bat", textoNivel);
    instanteAnterior = instanteAtual;
 
    instanteAnterior = instanteAtual;
  }


  if (millis() - tLeitura > 1000) {
    lerSensores();

    Serial.print("Distancia: ");
    Serial.print(distanciaMm);
    Serial.print(" mm | Quantidade: ");
    Serial.print(quantidadeMl);
    Serial.print(" mL | Temperatura: ");
    Serial.println(temperaturaC);

    if (!golAtivo) mostrarNivel();

    tLeitura = millis();
  }

  if (golAtivo && millis() - inicioGol > 10000) golAtivo = false;

  if (golAtivo && millis() - tAnim > 100) {
    animarGol();
    tAnim = millis();
  }

  if (millis() - tMqtt > 5000) {
    publicarDados();
    tMqtt = millis();
  }
}