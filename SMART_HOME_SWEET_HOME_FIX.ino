#define BLYNK_TEMPLATE_ID "TMPL60XX9UBQi"
#define BLYNK_TEMPLATE_NAME "DHT11blynk"
#define BLYNK_AUTH_TOKEN "n5IFNAH0e13jRMkcqCB1fJeRz0dZ4nQh" // Ganti dengan Auth Token Anda dari Blynk

#define BLYNK_PRINT Serial
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <DHT.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "KAMPUS_4@POLIJE";  // Ganti dengan nama WiFi Anda
char pass[] = "@polije.ac.id";  // Ganti dengan password WiFi Anda

// NOTIFIKASI TELEGRAM BOT
#define BOTtoken "6919132219:AAFSmtxCWcOYenViGfMUaRhd5fd97zVFKYs"  // Token Bot Anda dari Botfather

// Ganti dengan ID chat Anda menggunakan @myidbot
#define CHAT_ID "6444737129"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = D3; // PIR Motion Sensor
bool motionDetected = false;

// Indikasi ketika gerakan terdeteksi
void ICACHE_RAM_ATTR detectsMovement() {
  motionDetected = true;
}

#define DHTPIN 4          // Pin digital tempat Anda menghubungkan sensor DHT11
#define DHTTYPE DHT11     // Tipe sensor DHT 11  
#define pirPin D3
#define relayPin D5
#define relayPin2 D6
#define servoPin D7
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
Servo myServo;

void setup(){
  Serial.begin(115200);
  
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht.begin();
  pinMode(pirPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  myServo.attach(servoPin);
  timer.setInterval(2500L, sendSensor);

  // Inisialisasi koneksi WiFi
  #ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");      // Dapatkan waktu UTC melalui NTP
  client.setTrustAnchors(&cert); // Tambahkan sertifikat root untuk api.telegram.org
  #endif

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Tambahkan sertifikat root untuk api.telegram.org
  #endif
  
  // Mode PIR Motion Sensor sebagai INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Atur pin motionSensor sebagai interrupt, beri fungsi interrupt, dan set mode RISING
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Mencoba terhubung ke jaringan Wifi:
  Serial.print("Menghubungkan Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi terhubung");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot mulai berjalan", "");
}

void loop(){
  int pirValue = digitalRead(pirPin);
  Blynk.run();
  timer.run();

  // NOTIFIKASI TELEGRAM BOT
  if(motionDetected){
    bot.sendMessage(CHAT_ID, "Ada Tamu Diluar !!!", "");
    Serial.println("Ada Tamu Diluar !!!");
    motionDetected = false;
  }

  // Pemberitahuan pada Serial Monitor
  if(pirValue == HIGH){
    Serial.println("Sensor PIR mendeteksi gerakan!");
    delay(2000);
  }
}

void sendSensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca sensor DHT!");
    return;
  }
  
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  
  Serial.print("Suhu : ");
  Serial.print(t);
  Serial.print("|| Kelembapan : ");
  Serial.println(h);
}

// Tombol Lampu BLYNK
BLYNK_WRITE(V2) {
  int pinValue = param.asInt();
  
  if (pinValue == 0) {
    digitalWrite(relayPin, HIGH);
    Serial.println("Lampu mati");
  } else {
    digitalWrite(relayPin, LOW);
    Serial.println("Lampu Menyala");
  }
}

// Tombol DoorLock BLYNK
BLYNK_WRITE(V3) {
  int pinValue = param.asInt();
  
  if (pinValue == 0) {
    digitalWrite(relayPin2, HIGH);
    Serial.println("Pintu Tertutup");
  } else {
    digitalWrite(relayPin2, LOW);
    Serial.println("Pintu Terbuka");
  }
}

// Tombol Servo BLYNK
BLYNK_WRITE(V4) {
  int pinValue = param.asInt();
  
  if (pinValue == 0) {
    Serial.println("Jendela Tertutup");
    myServo.write(0);
    delay(2000);
  } else {
    Serial.println("Jendela Terbuka");
    myServo.write(180);
    delay(2000);
  }
}
