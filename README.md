# Wemos D1 Mini + LCD 2004A — Wikipedia "On This Day"

Автономний настільний інформер на базі **ESP8266 (Wemos D1 Mini)** та символьного дисплея **LCD 2004A (I2C)**, який підтягує історичні події за поточну дату з API англійської Вікіпедії (`en.wikipedia.org/api/rest_v1/feed/onthisday`).

## Можливості
- Автоматична синхронізація дати й часу по NTP (часовий пояс України).
- Завантаження 10 історичних подій за поточний день та автоматичне оновлення о 00:00.
- Перенос слів без розривів та посторінкове гортання довгих описів (`p1/2`, `p2/2`).
- Ізоляція паролів та ключів Wi-Fi через `include/secrets.h`.

## Підключення (I2C)
| LCD 2004A (PCF8574) | Wemos D1 Mini |
| :--- | :--- |
| **GND** | `G` (GND) |
| **VCC** | `3V3` (3.3V) |
| **SDA** | `D2` (GPIO4) |
| **SCL** | `D1` (GPIO5) |

*(Адреса дисплея за замовчуванням: `0x27`)*

## Налаштування та запуск (PlatformIO)
1. Склонуй репозиторій та відкрий папку в **VS Code (PlatformIO)**.
2. Скопіюй шаблон `include/secrets.example.h` у новий файл `include/secrets.h`.
3. Впиши у `include/secrets.h` назву (`WIFI_SSID`) та пароль (`WIFI_PASSWORD`) своєї мережі Wi-Fi 2.4 ГГц.
4. Підключи плату по USB та натисни **Upload** (`→`).

## Використані бібліотеки
- `marcoschwartz/LiquidCrystal_I2C`
- `bblanchon/ArduinoJson`

---

# English Documentation

A standalone desktop info widget powered by **ESP8266 (Wemos D1 Mini)** and an **LCD 2004A (I2C)** character display that fetches historical "On This Day" events for the current date from the English Wikipedia REST API (`en.wikipedia.org/api/rest_v1/feed/onthisday`).

## Features
- Automatic date and time synchronization via NTP (with automatic DST adjustment).
- Fetches 10 historical events for the current day and refreshes automatically at midnight (00:00).
- Smart word-wrapping without splitting words and multi-page scrolling for long descriptions (`p1/2`, `p2/2`).
- Credentials isolation for Wi-Fi SSID/password using `include/secrets.h`.

## Wiring (I2C)
| LCD 2004A (PCF8574) | Wemos D1 Mini |
| :--- | :--- |
| **GND** | `G` (GND) |
| **VCC** | `3V3` (3.3V) |
| **SDA** | `D2` (GPIO4) |
| **SCL** | `D1` (GPIO5) |

*(Default I2C display address: `0x27`)*

## Setup & Build (PlatformIO)
1. Clone this repository and open the folder in **VS Code (PlatformIO)**.
2. Copy `include/secrets.example.h` to a new file named `include/secrets.h`.
3. Enter your 2.4 GHz Wi-Fi credentials (`WIFI_SSID` and `WIFI_PASSWORD`) inside `include/secrets.h`.
4. Connect the board via USB and click **Upload** (`→`).

## Dependencies
- `marcoschwartz/LiquidCrystal_I2C`
- `bblanchon/ArduinoJson`
