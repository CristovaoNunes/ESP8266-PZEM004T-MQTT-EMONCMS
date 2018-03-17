//MQTT
#include <PubSubClient.h>
//ESP
#include <ESP8266WiFi.h>

#define MQTT_AUTH false
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

#include <SoftwareSerial.h>
//https://github.com/olehs/PZEM004T
#include <PZEM004T.h>

PZEM004T pzem(D2, D1);
IPAddress ip(192, 168, 1, 1);

//Constantes
const String HOSTNAME  = "pzem";

const char* ssid = "SSID";

const char* password = "PASSWORD";

const char* host = "emoncms.org";

//MQTT BROKERS GRATUITOS PARA TESTES https://github.com/mqtt/mqtt.github.io/wiki/public_brokers
const char* MQTT_SERVER = "192.168.1.150";

WiFiClient wclient;
PubSubClient client(MQTT_SERVER, 1883, wclient);


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.println(".");
  // Aguarda até estar ligado ao Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Ligado a ");
  Serial.println(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  pzem.setAddress(ip);
}

bool checkMqttConnection() {
  if (!client.connected()) {
    if (MQTT_AUTH ? client.connect(HOSTNAME.c_str(), MQTT_USERNAME, MQTT_PASSWORD) : client.connect(HOSTNAME.c_str())) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED");
    }
  }
  return client.connected();
}



void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()) {
      client.loop();

      
      float v = pzem.voltage(ip);
      String voltagem;
      voltagem = String(v);
      if (v < 0.0) v = 0.0;
      Serial.print(v); Serial.print("V; ");
      client.publish("/Casa/PZEM/V", voltagem.c_str());

      float i = pzem.current(ip);
      String amperagem;
      amperagem = String(i);
      if (i >= 0.0) {
        Serial.print(i);
        Serial.print("A; ");
      }
      client.publish("/Casa/PZEM/A", amperagem.c_str());

      float p = pzem.power(ip);
      String potencia;
      potencia = String(p);
      if (p >= 0.0) {
        Serial.print(p);
        Serial.print("W; ");
      }
      client.publish("/Casa/PZEM/P", potencia.c_str());


      float e = pzem.energy(ip);
      String energia;
      energia = String(e);
      if (e >= 0.0) {
        Serial.print(e);
        Serial.print("Wh; ");
      }
      client.publish("/Casa/PZEM/E", energia.c_str());



      WiFiClient client;
      const int httpPort = 80;
      if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
      }


      // We now create a URI for the request
      //Substituir a apikey                    ********************************
      String url = "/api/post?node=pzem&apikey=71128d82c7c98335d201076767320f73&json={voltagem:" + String(v) + ",amperagem:" + String(i) + ",potencia:" + String(p) + ",energia:" + String(e)+ "}";
      
      Serial.print("Requesting URL: ");
      Serial.println(url);

      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
        }
      }

    }
  }
}
