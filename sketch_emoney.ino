#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SS_PIN 2  // D4
#define RST_PIN 0 // D3

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 20, 4); 
const char* ssid = "YellowBigBang!";
const char* password = "123123123";
String content;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(1000);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed to connect");
  } else {
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Use this URL to connect: http://");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected to WiFi");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    Serial.println();
    Serial.print(" UID tag :");
    content = "";
    byte letter;

    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    Serial.println();
    
    // Menampilkan menu pilihan ke Serial Monitor
    Serial.println("Silakan pilih opsi:");
    Serial.println("1. Isi Saldo");
    Serial.println("2. Pembayaran");
    Serial.println("3. Cek Saldo");
    
    // Mendapatkan pilihan dari pengguna
    while (Serial.available() <= 0); // Menunggu input dari Serial Monitor
    char option = Serial.read();

    // Menangani pilihan opsi
    switch (option) {
      case '1':
        isiSaldo();
        break;
      case '2':
        pembayaran();
        break;
      case '3':
        cekSaldo();
        break;
      default:
        Serial.println("Opsi tidak valid");
        break;
    }
  } else {
    Serial.println("Error in WiFi connection");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi error");
  }
}

void isiSaldo() {
  Serial.println("Masukkan jumlah saldo yang ingin diisi:");
  
  // Variabel penampung untuk nilai saldo
  String saldoInput = "";
  int saldo = 0;
  
  // Menunggu input dari Serial Monitor
  while (saldoInput == "") {
    if (Serial.available() > 0) {
      saldoInput = Serial.readStringUntil('\n');
      saldo = saldoInput.toInt();
    }
    delay(100); // Memberi sedikit jeda untuk input dari Serial Monitor
  }
  
  // Kirim data ke server untuk proses
  sendRequest("isi_saldo", saldo);
}

void pembayaran() {
  Serial.println("Masukkan jumlah yang ingin dibayar:");
  
  // Variabel penampung untuk nilai jumlah pembayaran
  String jumlahInput = "";
  int jumlah = 0;
  
  // Menunggu input dari Serial Monitor
  while (jumlahInput == "") {
    if (Serial.available() > 0) {
      jumlahInput = Serial.readStringUntil('\n');
      jumlah = jumlahInput.toInt();
    }
    delay(100); // Memberi sedikit jeda untuk input dari Serial Monitor
  }
  
  // Kirim data ke server untuk proses
  sendRequest("pembayaran", jumlah);
}

void cekSaldo() {
  // Kirim permintaan ke server untuk cek saldo
  sendRequest("cek_saldo", 0); // Mengirim 0 karena tidak ada jumlah yang harus dikirim
}

void sendRequest(String option, int amount) {
  HTTPClient http;
  WiFiClient client;
  http.begin(client, "http://192.168.170.48/rfid_emoney/proses.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Membuat payload untuk dikirim
  String payload = "uid=" + content + "&option=" + option;
  
  // Jika option adalah pembayaran atau isi_saldo, tambahkan amount ke payload
  if (option == "pembayaran" || option == "isi_saldo") {
    payload += "&amount=" + String(amount);
  }
  
  // Jika option adalah daftar_rfid, tambahkan nama ke payload
  if (option == "daftar_rfid") {
    Serial.println("Masukkan nama untuk pendaftaran RFID:");
    while (Serial.available() <= 0); // Menunggu input dari Serial Monitor
    String nama = Serial.readStringUntil('\n');
    payload += "&nama=" + nama;
  }
  
  int httpResponseCode = http.POST(payload);
  delay(100);
  if (httpResponseCode > 0) {
    String response = http.getString();
    char json[500];
    response.toCharArray(json, 500);
    StaticJsonDocument<200> doc;
    deserializeJson(doc, json);

    const char* status = doc["Detail"]["Status"];
    const char* nama = doc["Detail"]["Data User"]["nama"];
    int saldo_sebelum = doc["Detail"]["Data User"]["saldo"];
    int nilai_transaksi = doc["Detail"]["Nilai Transaksi"];
    int saldo_sekarang = doc["Detail"]["Saldo Akhir"];

    Serial.println("=============== HASIL PARSING RESPONSE ==========");
    Serial.print("Status Transaksi = ");
    Serial.println(status);
    Serial.print("Nama Pengguna = ");
    Serial.println(nama);
    Serial.print("Saldo sebelum = Rp.");
    Serial.println(saldo_sebelum);
    Serial.print("Nilai transaksi = Rp.");
    Serial.println(nilai_transaksi);
    Serial.print("Saldo Sekarang = Rp.");
    Serial.println(saldo_sekarang);
    Serial.println("==================================================");

    lcd.clear();
    lcd.setCursor(0, 0);
   
    lcd.print(status);
    lcd.setCursor(0, 1);
    lcd.print("Nama: ");
    lcd.print(nama);
    lcd.setCursor(0, 2);
    lcd.print("Sebelum: Rp.");
    lcd.print(saldo_sebelum);
    lcd.setCursor(0, 3);
    lcd.print("Sekarang: Rp.");
    lcd.print(saldo_sekarang);

  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    Serial.print("HTTPClient error: ");
    Serial.println(http.errorToString(httpResponseCode)); // Tambahkan ini untuk debugging
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("POST error: ");
    lcd.setCursor(0, 1);
    lcd.print(httpResponseCode);
  }
  delay(2000);
  http.end();
}
