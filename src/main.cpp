#include <M5Stack.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <FS.h>

#include <stdio.h>
#include <string.h>

#define DELAYTIMESTEP 500
#define TIMEOUT 5000

String wifi_conf = "/wifi.comf";

// SmartConfigのセットアップと接続待ち
void smartConfig()
{
  M5.Lcd.println("Start Smart Config");

  // SmartConfig開始
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  // 設定待ち
  M5.Lcd.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone())
  {
    delay(DELAYTIMESTEP);
    M5.Lcd.print(".");
  }

  M5.Lcd.println("SmartConfig Finish.");
}

// WiFiの接続待ちとTimeout判定
bool tryConnectWiFi(){
  int sumtime = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (sumtime > TIMEOUT)
      return false;
    
    sumtime += DELAYTIMESTEP;
    delay(DELAYTIMESTEP);
    M5.Lcd.print('.');
  }
  M5.Lcd.println("WiFi Connected");
  return true;
}

// WiFiセットアップのメインルーチン
void setupWiFi(){

  SPIFFS.begin();
  if (SPIFFS.exists(wifi_conf))
  {
    // Fileが存在していたら読み込み
    M5.Lcd.println("Found Setting");
    File confFile = SPIFFS.open(wifi_conf.c_str(), "r");
    String ssid = confFile.readStringUntil('\n');
    String pass = confFile.readStringUntil('\n');
    confFile.close();
    // 文字列を加工
    std::string ssid_std_str = std::string(ssid.c_str());
    ssid_std_str.pop_back();
    std::string pass_std_str = std::string(pass.c_str());
    pass_std_str.pop_back();

    M5.Lcd.printf("SSID:%s\n", ssid_std_str.c_str());
    M5.Lcd.printf("PASS:%s\n", pass_std_str.c_str());
    
    // WiFi接続開始
    M5.Lcd.println("Try Connect");
    WiFi.begin(ssid_std_str.c_str(), pass_std_str.c_str());
  }
  else
  {
    // SmartConfigでWiFi設定
    smartConfig();
    String ssid = WiFi.SSID();
    String pass = WiFi.psk();
    M5.Lcd.printf("SSID:%s\n", ssid.c_str());
    M5.Lcd.printf("PASS:%s\n", pass.c_str());

    // Save settings.
    File confFile = SPIFFS.open(wifi_conf.c_str(), "w");
    confFile.println(ssid.c_str());
    confFile.println(pass.c_str());
    confFile.close();
  }

  while (!tryConnectWiFi())
  {
    // 既存の設定ファイルを削除
    SPIFFS.remove(wifi_conf);

    // SmartConfigでWiFi設定
    smartConfig();
  }
}

// NTP
const char *ntpServer = "ntp.nict.jp";
const long gmtOffset_sec = 9 * 3600;
const int daylightOffset_sec = 0;
// 時間関連
struct tm timeinfo;
uint8_t secLastReport = 0;
const char *week[7] = {"Sun", "Mon", "Tue", "wed", "Thu", "Fri", "Sat"};

void setup()
{
  M5.begin();
  M5.Lcd.setTextSize(2);

  // WiFiセットアップ
  setupWiFi();

  // ネット接続確認用サンプルとして時刻取得
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // NTPによる時刻取得
  if (!getLocalTime(&timeinfo))
  {
    M5.Lcd.printf("/nFailed to obtain time"); //時刻取得失敗表示
  }
  M5.Lcd.printf("%02d/%02d/%02d(%s) %02d:%02d:%02d",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, week[timeinfo.tm_wday],
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void loop()
{
  if (M5.BtnC.pressedFor(2000))
  {
    SPIFFS.remove(wifi_conf);
    ESP.restart();
  }
  M5.update();
}
