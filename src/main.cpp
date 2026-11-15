#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include "secrets.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int MAX_EVENTS = 10;
String eventYears[MAX_EVENTS];
String eventTexts[MAX_EVENTS];
int totalEvents = 0;
int currentEventIdx = 0;

// Налаштування таймінгів читання
unsigned long lastPageTurn = 0;
const unsigned long pageReadTime = 8000;   // Час на читання однієї сторінки (8 секунд)
const unsigned long shortEventTime = 25000; // Час показу короткої події на 1 сторінку (25 секунд)

int currentPage = 0;
int totalPages = 1;
int lastFetchedDay = -1;

// Розбиваємо довгий текст на красиві рядки по 20 символів без розриву слів
int wrapTextToLines(String text, String lines[], int maxLines) {
  int lineCount = 0;
  text.trim();

  while (text.length() > 0 && lineCount < maxLines) {
    if ((int)text.length() <= 20) {
      lines[lineCount++] = text;
      break;
    }

    // Шукаємо останній пробіл у межах 20 символів
    int splitPos = text.lastIndexOf(' ', 20);
    if (splitPos <= 0) {
      splitPos = 20; // Якщо слово довше 20 символів — ріжемо примусово
    }

    lines[lineCount++] = text.substring(0, splitPos);
    text = text.substring(splitPos);
    text.trim();
  }
  return lineCount;
}

void fetchWikipediaEvents(int month, int day) {
  if (WiFi.status() != WL_CONNECTED) return;

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();

  HTTPClient https;
  char url[100];
  sprintf(url, "https://en.wikipedia.org/api/rest_v1/feed/onthisday/events/%02d/%02d", month, day);

  if (https.begin(*client, url)) {
    https.addHeader("User-Agent", "ESP8266-LCD2004-Widget/1.0");
    int httpCode = https.GET();

    if (httpCode == 200) {
      JsonDocument filter;
      filter["events"][0]["year"] = true;
      filter["events"][0]["text"] = true;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, https.getStream(), DeserializationOption::Filter(filter));

      if (!error) {
        JsonArray events = doc["events"].as<JsonArray>();
        totalEvents = 0;

        for (JsonObject ev : events) {
          if (totalEvents >= MAX_EVENTS) break;
          int yr = ev["year"] | 0;
          const char* txt = ev["text"] | "";

          if (yr > 0 && strlen(txt) > 0) {
            eventYears[totalEvents] = String(yr);
            String cleanText = String(txt);
            // Зберігаємо до 240 символів (вистачить на 4 сторінки по 3 рядки)
            if (cleanText.length() > 240) {
              cleanText = cleanText.substring(0, 237) + "...";
            }
            eventTexts[totalEvents] = cleanText;
            totalEvents++;
          }
        }
        currentEventIdx = 0;
        currentPage = 0;
      }
    }
    https.end();
  }
}

void showCurrentEventPage(int day, int month) {
  lcd.clear();

  if (totalEvents == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Wikipedia OnThisDay");
    lcd.setCursor(0, 2);
    lcd.print("No events loaded...");
    return;
  }

  // Нарізаємо поточну подію на рядки по 20 символів (максимум 12 рядків = 4 сторінки)
  String wrappedLines[12];
  int totalLines = wrapTextToLines(eventTexts[currentEventIdx], wrappedLines, 12);
  totalPages = (totalLines + 2) / 3; // По 3 рядки на один екран
  if (totalPages < 1) totalPages = 1;
  if (currentPage >= totalPages) currentPage = 0;

  // Рядок 0: Дата | Рік | Номер події та сторінки (напр. 15.11|1971 1/10 p1/2)
  String header = "";
  if (day < 10) header += "0";
  header += String(day) + ".";
  if (month < 10) header += "0";
  header += String(month) + "|" + eventYears[currentEventIdx] + " " + String(currentEventIdx + 1) + "/" + String(totalEvents);

  if (totalPages > 1) {
    header += " p" + String(currentPage + 1) + "/" + String(totalPages);
  }

  lcd.setCursor(0, 0);
  lcd.print(header.substring(0, 20));

  // Рядки 1, 2, 3: Виводимо 3 рядки поточної сторінки
  int startLine = currentPage * 3;
  for (int row = 0; row < 3; row++) {
    int lineIdx = startLine + row;
    if (lineIdx < totalLines) {
      lcd.setCursor(0, row + 1);
      lcd.print(wrappedLines[lineIdx]);
    }
  }
}

void setup() {
  Wire.begin(D2, D1);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Wikipedia History");
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.setCursor(0, 2);
  lcd.print("Syncing date...");
  configTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  while (now < 100000) {
    delay(500);
    now = time(nullptr);
  }

  struct tm* t = localtime(&now);
  lastFetchedDay = t->tm_mday;

  lcd.setCursor(0, 3);
  lcd.print("Loading events...");
  fetchWikipediaEvents(t->tm_mon + 1, t->tm_mday);
  showCurrentEventPage(t->tm_mday, t->tm_mon + 1);
  lastPageTurn = millis();
}

void loop() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);

  // Опівночі підтягуємо події на новий день
  if (t->tm_mday != lastFetchedDay) {
    lastFetchedDay = t->tm_mday;
    fetchWikipediaEvents(t->tm_mon + 1, t->tm_mday);
    showCurrentEventPage(t->tm_mday, t->tm_mon + 1);
    lastPageTurn = millis();
  }

  // Визначаємо, скільки чекати: 8 сек якщо є продовження сторінок, або 25 сек якщо подія на 1 екран
  unsigned long waitTime = (totalPages > 1) ? pageReadTime : shortEventTime;

  if (totalEvents > 0 && millis() - lastPageTurn >= waitTime) {
    currentPage++;
    // Якщо всі сторінки цієї події прочитані — переходимо до наступної події
    if (currentPage >= totalPages) {
      currentPage = 0;
      currentEventIdx = (currentEventIdx + 1) % totalEvents;
    }
    showCurrentEventPage(t->tm_mday, t->tm_mon + 1);
    lastPageTurn = millis();
  }
}