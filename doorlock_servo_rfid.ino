#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MFRC522.h>
#include <Servo.h>

// WiFi
const char* ssid = "JTI-POLINEMA";
const char* password = "jtifast!";

// MQTT
const char* mqtt_server = "broker.hivemq.com";
const char* mqttTopic = "smart/door";
bool isMqttConnected = false;

// Pin
#define SS_PIN D4
#define RST_PIN D2
#define LED_PIN D0
// #define DOOR_LOCK_PIN D1

Servo doorServo;

WiFiClient espClient;
PubSubClient client(espClient);

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      isMqttConnected = true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("MQTT Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(LED_PIN, OUTPUT); // Inisialisasi LED mati
  Serial.println("Scan RFID card to get UID...");
  doorServo.attach(D3); // Ganti D0 dengan pin yang kamu pilih
  doorServo.write(0);  // Set posisi awal servo (tertutup)
}

void loop() {
  if (!client.connected() && !isMqttConnected) {
    reconnect();
  }
  if (!client.loop()) {
    client.connect("ESP8266Client");
  }
  
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    cardUID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println("Card UID: " + cardUID);
  if (cardUID == "adb7a082") {
    digitalWrite(LED_PIN, HIGH); // Nyalakan LED
    doorServo.write(90); // Set posisi servo (terbuka)
    client.publish(mqttTopic,"Continue");
    delay(1000); // Tahan posisi terbuka selama 1 detik
    doorServo.write(0); // Kembali ke posisi awal (tertutup)
    Serial.println("Door is Open");
    
    
  }if (cardUID  == "bcd4653b") {
       digitalWrite(LED_PIN, HIGH); // Nyalakan LED
    doorServo.write(90); // Set posisi servo (terbuka)
    client.publish(mqttTopic,"Continue");
    delay(1000); // Tahan posisi terbuka selama 1 detik
    doorServo.write(0); // Kembali ke posisi awal (tertutup)
    Serial.println("Door is Open");
  }
  // } else {
  //   digitalWrite(LED_PIN, LOW); // Matikan LED
  //   Serial.println("Access Denied");
  //   client.publish(mqttTopic,"Cancel");
  // }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}