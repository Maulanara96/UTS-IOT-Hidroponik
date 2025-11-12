// 1. Sertakan Library
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// 2. Konfigurasi Jaringan & MQTT
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";        
const char* mqtt_server = "9697015f3b5d4b9dbcbaaa9789835176.s1.eu.hivemq.cloud"; // Broker publik
const int   mqtt_port = 8883;
const char* mqtt_username = "esp32_hidroponik";
const char* mqtt_password = "Sayamaul1234";

// Topik MQTT
#define TOPIC_DATA_SENSOR "hidroponik/data/sensor"
#define TOPIC_KONTROL_POMPA "hidroponik/kontrol/pompa"

// 3. Konfigurasi Hardware (PIN)
// Sensor (1b)
#define DHT_PIN 27
#define DHT_TYPE DHT22 // DHT22 lebih baik untuk simulasi
// Aktuator (1b)
#define GREEN_LED_PIN 15
#define YELLOW_LED_PIN 4
#define RED_LED_PIN 2
#define BUZZER_PIN 13
// Aktuator (2b)
#define RELAY_PIN 17

// 4. Inisialisasi Objek
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Variabel timer
long lastMsg = 0;
#define MSG_INTERVAL 5000 // Interval 5 detik

// --- FUNGSI SETUP ---
void setup() {
  Serial.begin(115200);
  
  // Inisialisasi pin aktuator (OUTPUT)
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Matikan semua saat awal
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();
  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback); // Fungsi saat terima pesan
}

// --- FUNGSI KONEKSI WIFI ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung!");
}

// --- FUNGSI CALLBACK MQTT
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  if (strcmp(topic, TOPIC_KONTROL_POMPA) == 0) {
    if (message == "ON") {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Pompa DINYALAKAN");
    } else if (message == "OFF") {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Pompa DIMATIKAN");
    }
  }
}

// --- FUNGSI REKONEK MQTT ---
void mqtt_reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba terhubung ke MQTT...");
    
    if (client.connect("esp32HidroponikWokwiClient", mqtt_username, mqtt_password)) {
      
      Serial.println("terhubung!");
      // Subscribe ke topik kontrol pompa
      client.subscribe(TOPIC_KONTROL_POMPA);
      Serial.print("Subscribe ke: ");
      Serial.println(TOPIC_KONTROL_POMPA);
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      // Cek error. Error state 4 = Bad Username/Password
      // Error state 5 = Not Authorized
      delay(5000);
    }
  }
}

// --- FUNGSI LOGIKA SENSOR (Soal 1b) ---
void jalankanLogikaSuhu(float t) {
  // Matikan semua dulu
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Terapkan kondisi
  if (t > 35) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  } else if (t >= 30 && t <= 35) {
    digitalWrite(YELLOW_LED_PIN, HIGH);
  } else if (t < 30) {
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
}

// --- FUNGSI LOOP UTAMA ---
void loop() {
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop(); // Wajib untuk MQTT

  long now = millis();
  if (now - lastMsg > MSG_INTERVAL) { // Jalankan setiap 5 detik
    lastMsg = now;

    // Baca sensor
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Gagal membaca dari sensor DHT!");
      return;
    }

    Serial.print("Suhu: ");
    Serial.print(t);
    Serial.print(" *C, Kelembapan: ");
    Serial.print(h);
    Serial.println(" %");

    // --- Menjalankan Logika Soal 1b ---
    jalankanLogikaSuhu(t);

    // --- Menjalankan Streaming Data Soal 2b ---
    char jsonBuffer[100];
    snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"suhu\": %.2f, \"kelembapan\": %.2f}", t, h);
    
    client.publish(TOPIC_DATA_SENSOR, jsonBuffer);
    Serial.print("Data terkirim ke MQTT: ");
    Serial.println(jsonBuffer);
  }

}
