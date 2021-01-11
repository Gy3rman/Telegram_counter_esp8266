#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
//^^^Библиотеки^^^

ESP8266WiFiMulti WiFiMulti;

long subscribersCount = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);
//^^^Объявление переменных/устройств^^^

void setup()
{
  pinMode(2, OUTPUT);
  lcd.begin(4, 5);
  lcd.backlight();
  lcd.print("Connecting...");
  Serial.begin(115200);
//^^^Настройка дисплея^^^
  delay(5000);
  WiFi.mode(WIFI_STA);
  WiFi.begin("NAME", "PASSWORD"); //Впишите название точки доступа(NAME) и пароль(PASSWORD) 
//^^^Подключение к Wi-Fi^^^
}

void check()
{
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  //^^^Настраиваем ssl протокол^^^
  HTTPClient http;
  http.begin(*client, "https://api.telegram.org/botBOT_TOKEN/getChatMembersCount?chat_id=@channelname"); //Впишите токен своего бота, который есть на канале(BOT_TOKEN) и название канала(channelname)
  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
      long tempCount = root["result"];
      //^^^Получаем данные о количестве подписчиков^^^
      Serial.print("New count = ");
      Serial.println(tempCount);
      noize(tempCount); //Включаем звуковое уведомление
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());//Ошибка
  }
  http.end();
}

void loop()
{
  check();//проверка количества подписчиков
  delay(1000);//ждем 1 секунду(1000мс)
}

void showSubscribersCount(int count)
{
  lcd.clear(); //Очистка дисплея
  lcd.print("  My channel  "); //Название канала(Верхняя строчка дислпея
  lcd.setCursor(5, 1); //Ставим "курсор" на сегмент 5 на нижней строчке
  String data = "*" + String(count) + "*"; //Оформляем нижнию строчку
  lcd.print(data); //Печатаем второю строчку дисплея
}

void noize(int tempCount)
{
  if (subscribersCount != tempCount) {
    if (tempCount > subscribersCount) {
      tone (2, 5000);
      delay(250);
      tone (2, 0);
      showSubscribersCount(tempCount);
    } else if (tempCount < subscribersCount) {
      tone (2, 300);
      delay(250);
      tone (2, 0);
      showSubscribersCount(tempCount);
    }
    subscribersCount = tempCount;
  }
  //^^^Проверка: количество подписчиков увеличилось или уменьшилось и включаем соответствующий звук^^^
}
