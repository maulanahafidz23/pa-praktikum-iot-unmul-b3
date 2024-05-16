// Import ESP dan Publisher-Subscriber
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "Xiaomi 12T";
const char* password = "sebentar"; 
const char* mqtt_server = "broker.hivemq.com"; // broker gratisan

// LED pin
#define LED1 D1
#define LED2 D2
#define LED3 D3

// Batas Intensitas
int batas_intensitas1 = 10;
int batas_intensitas2 = 300;
int batas_intensitas3 = 600;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menerima data
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("] ");
  String data = ""; // variabel untuk menyimpan data yang berbentuk array char
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i]; // menyimpan kumpulan char kedalam string
  }

  if (strcmp(topic, "IOTB/3/SENSOR") == 0) {
    Serial.println(" Lux");
    int intensitasCahaya = data.toInt(); // konvert string ke int
    if (intensitasCahaya > batas_intensitas1) { // pengkondisian
      digitalWrite(LED1, LOW);
    } else if (intensitasCahaya < batas_intensitas1) { // pengkondisian
      digitalWrite(LED1, HIGH);
    }

    if (intensitasCahaya > batas_intensitas2) { // pengkondisian
      digitalWrite(LED2, LOW);
    } else if (intensitasCahaya < batas_intensitas2) { // pengkondisian
      digitalWrite(LED2, HIGH);
    }

    if (intensitasCahaya > batas_intensitas3) { // pengkondisian
      digitalWrite(LED3, LOW);
    } else if (intensitasCahaya < batas_intensitas3) { // pengkondisian
      digitalWrite(LED3, HIGH);
    }

  } else if (strcmp(topic, "IOTB/3/INPUTAN") == 0) {
    int spaceIndex = data.indexOf(' ');
    String NomorLED = data.substring(0, spaceIndex);
    String Intensitas = data.substring(spaceIndex + 1);
    if (NomorLED == "1"){
      batas_intensitas1 = Intensitas.toInt();
    } else if (NomorLED == "2"){
      batas_intensitas2 = Intensitas.toInt();
    } else if (NomorLED == "3"){
      batas_intensitas3 = Intensitas.toInt();
    }
  }
}

// fungsi untuk mengubungkan ke broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("IOTB/3/SENSOR");
      client.subscribe("IOTB/3/INPUTAN");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // setup awal ke server mqtt
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}