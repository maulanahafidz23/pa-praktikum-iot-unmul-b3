// Import ESP dan Publisher-Subscriber
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Import Komponen Intensitas Cahaya
#include <Wire.h>
#include <BH1750.h>

// Telegram
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Atur WIFI
const char* ssid = "Xiaomi 12T";
const char* password = "sebentar";
const char* mqtt_server = "broker.hivemq.com";

// Sensor Intensitas Cahaya
BH1750 lightMeter;
float lux;
int batas_intensitas1 = 10;
int batas_intensitas2 = 300;
int batas_intensitas3 = 600;
bool masuk = false;

// Settings Publisher
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Settings Telegram
#define BOT_TOKEN "7161930840:AAGZcr7hSuYgxxtU7ZL57PXrabzZGiWnFQ0" // ID Bot Token Telegram
const unsigned long BOT_MTBS = 1000; // mean time between scan messages
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

// Settings Pesan Masuk Telegram
void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    int spaceIndex = text.indexOf(' ');
    String command = text.substring(0, spaceIndex);
    String argument = text.substring(spaceIndex + 1, spaceIndex + 2);
    String argument2 = text.substring(spaceIndex + 2);
    
    if (command == "/LihatIntensitas") {
      String message = "";
      message += "Jumlah Intensitas : "+ String(lux) + " Lux";
      bot.sendMessage(chat_id, message, "");
    }

    if (command == "/LihatStatusLED") {
      String message = "LED Status: \nLed Merah (Led 1) = ";
      if(lux < batas_intensitas1) {
        message += "ON";
      } else {
        message += "OFF";
      }
      message += "\nLed Kuning (Led 2) = ";
      if(lux < batas_intensitas2) {
        message += "ON";
      } else {
        message += "OFF";
      }
      message += "\nLed Hijau (Led 3) = ";
      if(lux < batas_intensitas3) {
        message += "ON";
      } else {
        message += "OFF";
      }
      bot.sendMessage(chat_id, message, "");
    }

    if(command == "/AturBatasIntensitas"){
      String tex = "Ketik '/input [NomorLampu] [JumlahIntensitas]' untuk memilih lampu dan atur batas intensitas nya\nContoh: /input 1 250";
      bot.sendMessage(chat_id,tex,"");
      masuk = true;
    }
    if (command == "/input" && masuk){
      Serial.println(argument);
      Serial.println(argument2);
      if (argument == "1"){
        batas_intensitas1 = argument2.toInt();
        String teks = "Berhasil Diubah!\n Batas Intensitas LED " + argument + " = " + argument2;
        bot.sendMessage(chat_id,teks,"");
        client.publish("IOTB/3/INPUTAN", argument.c_str());
        Serial.println(argument);
        masuk = false;
      } else if (argument == "2"){
        batas_intensitas2 = argument2.toInt();
        String teks = "Berhasil Diubah!\n Batas Intensitas LED " + argument + " = " + argument2;
        bot.sendMessage(chat_id,teks,"");
        client.publish("IOTB/3/INPUTAN", argument.c_str());
        Serial.println(argument);
        masuk = false;
      } else if (argument == "3"){
        batas_intensitas3 = argument2.toInt();
        String teks = "Berhasil Diubah!\n Batas Intensitas LED " + argument + " = " + argument2;
        bot.sendMessage(chat_id,teks,"");
        client.publish("IOTB/3/INPUTAN", argument.c_str());
        Serial.println(argument);
        masuk = false;
      } else {
        String teks = "Inputan Tidak Valid";
        bot.sendMessage(chat_id,teks,"");
      }
    } 
    else if (command == "/input"){
      String teks = "Harap masuk ke menu /AturBatasIntensitas terlebih dahulu";
      bot.sendMessage(chat_id,teks,"");
    }

    if (command == "/LihatBatasIntensitasLED") {
      String message = "Batas Intensitas LED: ";
      message += "\nLed Merah (Led 1) = " + String (batas_intensitas1);
      message += "\nLed Kuning (Led 2) = " + String (batas_intensitas2);
      message += "\nLed Hijau (Led 3) = " + String (batas_intensitas3);
      bot.sendMessage(chat_id, message, "");
    }

    if (command == "/start") {
      String welcome = "Selamat Datang di Grup IoT Telegram Bot B3, " + from_name + ".\n\n";
      welcome += "Daftar Perintah:\n";
      welcome += "/LihatIntensitas : Untuk Melihat Jumlah Intensitas Cahaya Saat Ini\n";
      welcome += "/LihatStatusLED : Untuk Melihat Status LED\n";
      welcome += "/LihatBatasIntensitasLED : Untuk Melihat Batas Intensitas Setiap LED\n";
      welcome += "/AturBatasIntensitas : Untuk Melihat Status LED\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

// Atur Konfigurasi WIFI
void setup_wifi() {
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  randomSeed(micros());
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

// fungsi untuk menghubungkan ke broker
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
      client.subscribe("IOTB/3/SENSOR"); // DAPAT DARI TOPIC NYA
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
  // Atur Komponen Intensitas Cahaya
  Wire.begin();
  lightMeter.begin();

  // Atur MQTT
  client.setServer(mqtt_server, 1883);

  // Atur Serial Monitor dan WIFI
  Serial.begin(115200);
  setup_wifi();
}

void loop() {
  // Loop MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(2000);
  lux = lightMeter.readLightLevel();
  snprintf (msg, MSG_BUFFER_SIZE, "%.1f", lux);
  
  client.publish("IOTB/3/SENSOR", msg); // fungsi untuk publish ke broker

  // Loop Telegram
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("Got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}