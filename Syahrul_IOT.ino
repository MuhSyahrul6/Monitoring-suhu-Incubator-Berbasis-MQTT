#include <ESP32Servo.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

// Konfigurasi DHT22
#define DHT_PIN 4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Konfigurasi Relay
#define RELAY_PIN 13

// Konfigurasi Micro Servo
#define SERVO_PIN 18
Servo servo;

int humidity;  //Stores humidity value
int temperature; //Stores temperature value

// Konfigurasi WiFi
const char* WIFI_SSID = "A";
const char* WIFI_PASSWORD = "";

// Konfigurasi waktu penggerakkan servo (30 menit = 1800000 ms)
const unsigned long SERVO_INTERVAL = 15000;
unsigned long lastServoMoveTime = 0;

// Konfigurasi posisi servo
const int SERVO_START_POSITION = 0; // Posisi awal rak
const int SERVO_ROTATE_POSITION = 120; // Posisi rotasi rak

char temperatureStr[10]; // Array karakter untuk menyimpan suhu dalam bentuk string
char humidityStr[10]; // Array karakter untuk menyimpan kelembaban dalam bentuk string

// Konfigurasi MQTT
const char* mqttServer = "free.mqtt.iyoti.id";
const int mqttPort = 1883;
const char* mqttClientId = "Syahrulgtg";
const char* mqttTopicTemperature = "Syahrulgtg/temperature";
const char* mqttTopicHumidity = "Syahrulgtg/humidity";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(9600);

  // Menghubungkan ke jaringan WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Menginisialisasi DHT11
  dht.begin();

  // Menginisialisasi Micro Servo
  servo.attach(SERVO_PIN);

  // Menginisialisasi Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Menghubungkan ke broker MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqttClientId)) {
      Serial.println("Connected to MQTT broker");
      mqttClient.subscribe(mqttTopicTemperature);
      mqttClient.subscribe(mqttTopicHumidity);
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(500);
    }
  }
}

void loop() {
  // Membaca suhu dan kelembaban dari DHT22
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Menampilkan suhu dan kelembaban pada Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" Derajat C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // Mengubah suhu dan kelembaban menjadi string menggunakan snprintf
  snprintf(temperatureStr, sizeof(temperatureStr), "%d", temperature);
  snprintf(humidityStr, sizeof(humidityStr), "%d", humidity);

  // Mengirim data suhu dan kelembaban melalui MQTT
  mqttClient.publish(mqttTopicTemperature, temperatureStr);
  mqttClient.publish(mqttTopicHumidity, humidityStr);

  // Menggerakkan Micro Servo setiap satu jam
  unsigned long currentMillis = millis();
  if (currentMillis - lastServoMoveTime >= SERVO_INTERVAL) {
    moveServo();
    lastServoMoveTime = currentMillis;
  }

  // Mengontrol Relay berdasarkan suhu
  if (temperature > 37) {
    digitalWrite(RELAY_PIN, HIGH); // Hidupkan pemanas
  } else {
    digitalWrite(RELAY_PIN, LOW); // Matikan pemanas
  }

  // Proses pesan MQTT yang diterima
  mqttClient.loop();

  delay(1000); // Jeda 1 detik antara pembacaan suhu dan kelembaban
}

void moveServo() {
  // Menggerakkan servo ke posisi awal
  for (int pos = SERVO_START_POSITION; pos <= SERVO_ROTATE_POSITION; pos++) {
    servo.write(pos);
    delay(10); // Menunggu 10 milidetik sebelum perubahan posisi
  }
  delay(2000); // Menunggu 2 detik untuk stabilisasi

  // Menggerakkan servo untuk memutar telur
  for (int pos = SERVO_ROTATE_POSITION; pos >= SERVO_START_POSITION; pos--) {
    servo.write(pos);
    delay(10); // Menunggu 10 milidetik sebelum perubahan posisi
  }
  delay(2000); // Menunggu 2 detik untuk stabilisasi
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Lakukan sesuatu dengan pesan yang diterima
  if (strcmp(topic, mqttTopicTemperature) == 0) {
    // Pesan terkait suhu diterima
    // Lakukan sesuatu dengan nilai suhu yang diterima
  } else if (strcmp(topic, mqttTopicHumidity) == 0) {
    // Pesan terkait kelembaban diterima
    // Lakukan sesuatu dengan nilai kelembaban yang diterima
  }
}
