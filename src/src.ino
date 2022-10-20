#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "env.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
StaticJsonDocument<512> doc;
String line1="", line2="";

WiFiClient espClient;

int lastTime = 0, l1_lasttime = 0, l2_lasttime = 0, display_lasttime = 0;
int WebTimerDelay = 15000, line1timer = 1000, line2timer = 1000, displayTimer = 1000, songTimer = 0;

int state_l1 = 0, state_l2 = 0;
int l1_pos1, l1_pos2;
int l2_pos1, l2_pos2;

int httpResponseCode = 0;

struct api_data {
  int api_response, timer;
  String artists, device, song_title, status;
  api_data() {
    api_response = 204;
    timer = 0;
    artists = "";
    device = "";
    song_title = "";
    status = "";
  }
}var;


//arduino main code
void setup() {
  lcd.begin();
  lcd.backlight();
  
  Serial.begin(115200);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  sendHTTP();
  lastTime = millis();
  song_lasttime = lastTime;
  l1_lasttime = lastTime;
  l2_lasttime = lastTime;
  display_lasttime = lastTime;
}

void loop() {
  if (((millis() - lastTime) > WebTimerDelay) || (((millis() - song_lasttime) > songTimer) && (songTime != 0))) {
    sendHTTP();
    lastTime = millis();
  }
  if ((millis() - l1_lasttime) > line1timer) {
    set_text_l1();
    l1_lasttime = millis();
  }
  if ((millis() - l2_lasttime) > line2timer) {
    set_text_l2();
    l2_lasttime = millis();
  }
  if ((millis() - display_lasttime) > displayTimer) {
    update_display();
    display_lasttime = millis();
  }
}

// HTTP get function
void sendHTTP(){
      //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.useHTTP10(true);
      http.begin(espClient, WEB_SERVER);
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "uid=" + String(USER);
      // Send HTTP POST request
      httpResponseCode = http.POST(httpRequestData);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      bool res;
      if (httpResponseCode == 200) {
        deserializeJson(doc, http.getStream());
        res = fetch_variable();
        Serial.println(var.status);
        Serial.println(var.song_title);
        Serial.println(var.timer);
      }

      if (res == 1 || httpResponseCode != 200) WebTimerDelay = 15000;

      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
}

// Json data related function
bool fetch_variable() {
  var.api_response = doc["api_response"];
  if (var.api_response != 200) return 1;
  var.timer = doc["timer"];
  var.artists = doc["artist_string"].as<String>();
  var.device = doc["device_name"].as<String>();
  var.song_title = doc["song_title"].as<String>();
  var.status = doc["status"].as<String>();
  // var.device = doc["device_name"];
  // var.song_title = doc["song_title"];
  // var.status = doc["status"];
  songTimer = var.timer + 3000;
  Serial.print("web timer delay: ");
  Serial.println(WebTimerDelay);
  return 0;
}

// LCD related function
void update_display(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2);
}
String artist;
void set_text_l1(){
  if (httpResponseCode != 200){
    line1timer = 1000;
    songTimer = 0;
    line1 = "Not Connected";
    return;
  }
  if (var.api_response != 200){
    line1timer = 1000;
    songTimer = 0;
    line1 = "Not Connected";
    return;
  }
  if (var.status == "False"){
    line1timer = 1000;
    songTimer = 0;
    line1 = "PAUSED/STOP ON:";
    return;
  }
  if (var.artists.length() <= 9){
    line1timer = 1000;
    line1 = "PLAY - " + var.artists;
    return;
  }
  switch(state_l1){
    case 0:
      line1timer = 3000;
      l1_pos1 = -1;
      l1_pos2 = 8;

      artist = var.artists;
      artist.remove(l1_pos2);

      line1 = "PLAY - " + artist;
      state_l1 = 1;
      break;
    case 1:
      line1timer = 1000;
      l1_pos1+=1;
      if (var.artists.length() > l1_pos2) l1_pos2+=1;

      artist = var.artists;
      artist.remove(l1_pos2);
      artist.remove(0,l1_pos1);

      line1 = "PLAY - " + var.artists.substring(l1_pos1,l1_pos2);
      if (l1_pos1 == l1_pos2) state_l1 = 2;
      break;
    case 2:
      line1timer = 3000;
      line1 = "PLAY - ";
      state_l1 = 0;
      break;
  }
}
String song;
void set_text_l2(){
  if (httpResponseCode != 200){
    line2timer = 1000;
    line2 = "To Local Server";
    return;     
  }
  if (var.api_response != 200){
    line2timer = 1000;
    line2 = "To Spotify";
    return; 
  }
  if (var.status == "False"){
    line2timer = 1000;
    line2 = var.device;
    return;
  }
  if (var.song_title.length() <= 16){
    line2timer = 1000;
    line2 = var.song_title;
    return;
  }
  switch(state_l2){
    case 0:
      line2timer = 3000;
      l2_pos1 = -1;
      l2_pos2 = 15;
      //line2 = var.song_title.substring(l1_pos1,l1_pos2);
      song = var.song_title;
      song.remove(l2_pos2);
      line2 = song;
      state_l2 = 1;
      break;
    case 1:
      line2timer = 1000;

      l2_pos1+=1;
      if (var.song_title.length() > l2_pos2) l2_pos2++;

      song = var.song_title;
      song.remove(l2_pos2);
      song.remove(0,l2_pos1);

      line2 = song;
      if (l2_pos1 == l2_pos2) state_l2 = 2;
      break;
    case 2:
      line2timer = 3000;
      line2 = "";
      state_l2 = 0;
      break;
  }
}
