#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include "RTClib.h"

// WiFi credentials
#define WIFI_SSID "Dialog 4G 869"
#define WIFI_PASSWORD "6b7C4AD3"

// Telegram Bot credentials
#define BOT_TOKEN "7710816167:AAEVhXOftaXzaSISbDnFUT95PL7f3gXmEx4"
const unsigned long BOT_MTBS = 1000;  // Scan interval for messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

// Define pins for the RFID module
#define RST_PIN D3
#define SS_PIN D4

// RFID and RTC
MFRC522 rfid(SS_PIN, RST_PIN);
RTC_DS1307 RTC;

// Authorized RFID UID
byte authorizedUID[] = {0x03, 0xF8, 0xC0, 0x05};

// Store chat ID (for notification)
String chat_id;

// Last scan data
String lastUID = "N/A";
String lastStatus = "No scan yet";
String lastTime = "N/A";

// Timing
unsigned long previousMillis = 0;
const long INTERVAL = 6000;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setInsecure(); // Don't use certificate
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize RFID, SPI, RTC
  SPI.begin();
  rfid.PCD_Init();
  if (!RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Sync RTC with compile time
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println("Place your card on the reader...");
}

void loop() {
  // Handle incoming Telegram messages
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  // Periodically check for RFID card
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    checkRFID();
  }
}

void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Convert UID to string
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uidString += ":";
  }
  uidString.toUpperCase();

  Serial.println("Card UID: " + uidString);

  // Get time
  DateTime now = RTC.now();
  String timeString = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) +
                      " on " + String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());

  // Save scan info
  lastUID = uidString;
  lastTime = timeString;

  if (checkUID(rfid.uid.uidByte, rfid.uid.size, authorizedUID, sizeof(authorizedUID))) {
    lastStatus = "✅ Access Granted";
    Serial.println("Access Granted!");

    String message = "✅ *Access Granted*\n";
    message += "🆔 UID: `" + uidString + "`\n";
    message += "🕒 Time: " + timeString;

    if (chat_id != "") {
      bot.sendMessage(chat_id, message, "Markdown");
    }
  } 
  else {
    lastStatus = "❌ Access Denied";
    Serial.println("Access Denied!");

    String message = "❌ *Access Denied*\n";
    message += "🆔 UID: `" + uidString + "`\n";
    message += "🕒 Time: " + timeString;

    if (chat_id != "") {
      bot.sendMessage(chat_id, message, "Markdown");
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

bool checkUID(byte *readUID, byte readSize, byte *authUID, byte authSize) {
  if (readSize != authSize) return false;
  for (byte i = 0; i < readSize; i++) {
    if (readUID[i] != authUID[i]) return false;
  }
  return true;
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/start") {
      String welcome = "👋 Welcome, " + from_name + ".\n\n";
      welcome += "📡 *Available Commands:*\n";
      welcome += "🔍 /status - Show last RFID scan\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    } else if (text == "/status") {
      String statusMsg = "*Last RFID Scan Info:*\n";
      statusMsg += "🆔 UID: `" + lastUID + "`\n";
      statusMsg += "📅 Time: " + lastTime + "\n";
      statusMsg += "🔐 Status: " + lastStatus;
      bot.sendMessage(chat_id, statusMsg, "Markdown");
    } else {
      bot.sendMessage(chat_id, "❓ Unknown command. Try /start or /status", "");
    }
  }
}
