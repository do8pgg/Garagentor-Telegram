/*
   Copyright (c) 2021 do8pgg. All rights reserved.
   See the bottom of this file for the license terms.
                garage@nsa2go.de
*/

// Version 2.86 - 05.01.2022 / 23:30
// All-in-One Version!

/*
    Drin/Erledigt ist:

    WWW-Server für Token ansehen/neu setzen
    Token in spiffs speichern
    Token aus spiffs lesen und verwenden

    Wifi-Connector
    Wifi-Connector-Deleter per Pin D7 an Ground

    NTP-Client

    OTA für Entwicklung!

    Delete aller Nutzer mit einen Klick. Erledigt,
    Zwei Seiten zum zum Löschen reichen.

    User-Limit auf auf userN1-5.txt aus spiffs

    Status von Relais2 anzeigen und warnen,
    falls es nochmals ein/ausgeschaltet werden soll

    Fallback: Da gerade von allen Seiten gegen Telegram geschossen
    wird, habe ich eine Manuelle Tasterung eingebaut. Laesst sich
    auch per curl o.Ae. triggern.

    Wenn Wifi wegbricht versucht er noch noch 30 sekunden
    bevor das Wifi-Portal fuer 120 Sekunden gestartet wird. 
    Meldet sich niemand in den 120 Sekunden an, startet der
    esp neu und alles noch mal von vorn bis entweder:
    Bekanntes Wifi wieder erreichbar oder
    Neues Wifi im Portal eingestellt. 

    Es fehlen immer noch Party-Alpakas...
    Und Bier... :-)
*/

// Wifi-RSSi
const int amazing = -30;
const int verygood = -67;
const int okay = -70;
const int notgood = -80;
const int unuseable = -90;
// Wifi-RSSi

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <UniversalTelegramBot.h>
#include <time.h>
#include <LittleFS.h>
#include <Effortless_SPIFFS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "uptime_formatter.h"
#include "uptime.h"

#define BOT_TOKEN ""
const unsigned long BOT_MTBS = 500;

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

FS& gfs = LittleFS;
eSPIFFS fileSystem;

const int lichtPin = D2;
const int torPin = D5;
int tastdauer = 150; // in Millisekunden
int rtaster = D7; // Taster fuer Wifi-Reset!
int tasterstatus = 0;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    String from_id = bot.messages[i].from_id;

    // ChatID Geraffel
    String user1String;
    String user2String;
    String user3String;
    String user4String;
    String user5String;

    fileSystem.openFromFile("/user1.txt", user1String);
    fileSystem.openFromFile("/user2.txt", user2String);
    fileSystem.openFromFile("/user3.txt", user3String);
    fileSystem.openFromFile("/user4.txt", user4String);
    fileSystem.openFromFile("/user5.txt", user5String);

    if (chat_id != user1String) {
      if (chat_id != user2String) {
        if (chat_id != user3String) {
          if (chat_id != user4String) {
            if (chat_id != user5String) {
              //bot.sendMessage(chat_id, "Du nicht!", "");
              continue;
            }
          }
        }
      }
    }

    // ChatID Geraffel
    if (text == "/start")
    {
      String welcome = "Hallo und Herzlich willkommen, " + from_name + ".\n\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
    if (text == "tor")
    {
      int tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      digitalWrite(torPin, LOW); // Relais 1
      bot.sendMessage(chat_id, " Tor wird bewegt ", "");
      delay(tord1String);
      digitalWrite(torPin, HIGH);
    }

    if (text == "/tor")
    {
      int tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      digitalWrite(torPin, LOW); // Relais 1
      bot.sendMessage(chat_id, " Tor wird bewegt ", "");
      delay(tord1String);
      digitalWrite(torPin, HIGH);
    }
 
    /*
        if (text == "/users")
        {
          String user1String;
          String user2String;
          String user3String;
          String user4String;
          String user5String;
          fileSystem.openFromFile("/user1.txt", user1String);
          fileSystem.openFromFile("/user2.txt", user2String);
          fileSystem.openFromFile("/user3.txt", user3String);
          fileSystem.openFromFile("/user4.txt", user4String);
          fileSystem.openFromFile("/user5.txt", user5String);
          String welcome = "Erlaubte Benutzer:\n";
          welcome += "1.) " + user1String + "\n";
          welcome += "2.) " + user2String + "\n";
          welcome += "3.) " + user3String + "\n";
          welcome += "4.) " + user4String + "\n";
          welcome += "5.) " + user5String + "\n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    if (text == "/status")
    {
      String tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      //      String firmwarev;
      //      String firmwared;
      String rssi1String;
      int32_t rssi = wifi_station_get_rssi();

      if (rssi < unuseable) { // unter -90
        rssi1String = "Wifi so nicht benutzbar, sorry";
      }
      else if (rssi >= notgood && rssi < okay) {        // zwischen -80 und -70
        rssi1String = "Na, nicht wirklich gut!";
      }
      else if (rssi >= okay && rssi < verygood) {         // Zwischen -70 und -67
        rssi1String = "Des passt scho!";
      }
      else if (rssi >= verygood && rssi < amazing) {      // zwischen -30 und -67
        rssi1String = "Absolut Stark :-)";
      }
      String user1String;
      String user2String;
      String user3String;
      String user4String;
      String user5String;
      fileSystem.openFromFile("/user1.txt", user1String);
      fileSystem.openFromFile("/user2.txt", user2String);
      fileSystem.openFromFile("/user3.txt", user3String);
      fileSystem.openFromFile("/user4.txt", user4String);
      fileSystem.openFromFile("/user5.txt", user5String);

      uptime::calculateUptime();
      String IPaddress = WiFi.localIP().toString();
      String welcome = " ESP Bot - Version 2.86 - 05.01.2022 \n\n";
      welcome += "Verbunden mit SSID:\n";
      welcome += WiFi.SSID();
      welcome += "\n\n";
      welcome += "Wifi-SignalstÃ¤rke: \n";
      welcome += rssi;
      welcome += " dBm - ";
      welcome += rssi1String;
      welcome += "\n\n";
      welcome += "IP-Adresse: ";
      welcome += IPaddress + "\n\n";
      welcome += "Laufzeit:\n";
      welcome += uptime_formatter::getUptime() + "\n\n";
      welcome += "Tor-Tastdauer: \n";
      welcome += "" + tord1String + "ms\n\n";
      welcome += "Erlaubte Benutzer:\n";
      welcome += user1String + "\n";
      welcome += user2String + "\n";
      welcome += user3String + "\n";
      welcome += user4String + "\n";
      welcome += user5String + "\n\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
      Serial.println("Debug: /Status");
    }
    /* /uptime
      if (text == "/uptime")
      {
      String welcome = "Laufzeit:\n";
      welcome += uptime_formatter::getUptime() + "\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
      wifirssi();
      }
    */
    /* // /an
        if (text == "/an") // Relais 2
        {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 wird eingeschaltet\n";
            digitalWrite(lichtPin, LOW);
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 ist schon eingeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // an
        if (text == "an") // Relais 2
       {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 wird eingeschaltet\n";
            digitalWrite(lichtPin, LOW);
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 ist schon eingeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // aus
        if (text == "aus") // Relais 2
        {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 ist schon ausgeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 wird ausgeschaltet\n";
            digitalWrite(lichtPin, HIGH);
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // //aus
        if (text == "/aus") // Relais 2
        {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 ist schon ausgeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 wird ausgeschaltet\n";
            digitalWrite(lichtPin, HIGH);
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // /hilfe
        if (text == "/hilfe")
        {
          int tord1String;
          fileSystem.openFromFile("/tord.txt", tord1String);

          String welcome = "Willkommen beim Garagentor, " + from_name + ".\n\n";
          welcome += "/tor    Tastert den Tor-Taster\n";
          welcome += "/an     Schaltet Relais2 an \n";
          welcome += "/aus    Schaltet Relais2 aus \n";
          welcome += "/users  Zeigt aktuell gespeicherte Benutzer an\n";
          welcome += "/ip     zeigt die IP-Adresse vom ESP an\n";
          welcome += "/status     zeigt den Status von Relais2 an\n";
          welcome += "/uptime   Zeigt die Laufzeit vom ESP an\n";
          welcome += " \n";
          welcome += "'tor' 'an' und 'aus' sind auch ohne '/' möglich. \n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // status
        if (text == "status")
        {
          String welcome = "Relais2 ist ";
          if (digitalRead(lichtPin)) {
            welcome += "aus\n";
          } else {
            welcome += "an\n";
          }
          bot.sendMessage(chat_id, welcome, "Markdown");
          Serial.println("status");
        }


        if (text == "/status")
        {
          String welcome = "Relais2 ist ";
          if (digitalRead(lichtPin)) {
            welcome += "aus\n";
          } else {
            welcome += "an\n";
          }
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // /about
        if (text == "/about")
        {
          String welcome = "Garage-Controller - ";
          welcome += "This little code controls your garage door using ";
          welcome += "one (or two) simple relays and a Wemos D1-Mini. ";
          welcome += "\n\n";
          welcome += "It is forbidden to use this code ";
          welcome += "commercially/profitably or for police/military purposes! ";
          welcome += "\n\n ";
          welcome += "2022 by Peter Geher\n";
          welcome += "peter.geher@nsa2go.de\n";
          welcome += "http://www.layers-gedanken.de\n";
          welcome += " \n";
          welcome += "Have a lot of Fun! \n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // /ip
        if (text == "/ip")
        {
          String IPaddress = WiFi.localIP().toString();
          String welcome = "Das Tor hat die IP:\n";
          welcome += IPaddress + "\n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // /tord
      if (text == "/tord")
      {
      String tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      String welcome = "Tastdauer: ";
      welcome += "" + tord1String + "ms\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
      }
    */
  }
}

unsigned long previousMillis = 0;
const long blinkinterval = 125;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String Argument_Name, Clients_Response1, Clients_Response2;
const char* host = "garagen-esp";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";


void WifiVerbinder() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);

  WiFiManager wm;
  bool res;
  wm.setConnectTimeout(30);
  wm.setConfigPortalTimeout(120);
  res = wm.autoConnect("Garage");

  if (!res) {
    Serial.println("Debug: Failed to connect");
    delay(1000);
    Serial.println("Debug: Reboot!");
    delay(250);
    ESP.reset();
  }
  else {
    Serial.println("Debug: Verbunden... yay");
  }
}


void WifiDeleter() {

  // Wird nicht wirklich gebraucht, daher ist es auch im Loop auskommentiert!

  unsigned long currentMillis = millis();
  tasterstatus = digitalRead(rtaster);
  if (currentMillis - previousMillis >= blinkinterval) {
    previousMillis = currentMillis;
    if (tasterstatus == HIGH) {
    } else {
      Serial.println("Flash-Reset getastet!!!");
      ESP.eraseConfig();
      delay(100);
      ESP.reset();
    }
  }
}

void OTA() {
  ArduinoOTA.setHostname("garagen8266");
  ArduinoOTA.onStart([]() {
    String type;
    type = "sketch";
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
}

void ntp() {
  // NTP-Zeiten beziehen für Telegram-Cert...
  configTime(0, 0, "194.25.134.196");   // T-Online-Zeitserver
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {

    delay(100);
    now = time(nullptr);
  }
}

void webupdater() {
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
}

void tokenGetter() {
  const char* blah;
  fileSystem.openFromFile("/token.txt", blah);
  void updateToken(const char* blah);
  bot.updateToken(blah);
}

void Hauptseite() {
  String user1String;
  String user2String;
  String user3String;
  String user4String;
  String user5String;
  String tord1String;
  fileSystem.openFromFile("/user1.txt", user1String);
  fileSystem.openFromFile("/user2.txt", user2String);
  fileSystem.openFromFile("/user3.txt", user3String);
  fileSystem.openFromFile("/user4.txt", user4String);
  fileSystem.openFromFile("/user5.txt", user5String);
  fileSystem.openFromFile("/tord.txt", tord1String);

  int32_t rssi = wifi_station_get_rssi();

  String IPaddress = WiFi.localIP().toString();
  String Hauptseite;
  Hauptseite =  "<html>";
  Hauptseite += "<head><title>ESP Bot </title>";
  Hauptseite += "<center>";
  Hauptseite += "<style>";
  Hauptseite += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  Hauptseite += "</style>";
  Hauptseite += "</head>";
  Hauptseite += "<body><br>";
  Hauptseite += "<h1><br>ESP Bot &Uuml;bersicht</h1>";
  Hauptseite += "<a href=http://" + IPaddress + "/st" + ">Token anzeigen</a><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/SetToken" + ">Neues Token setzen</a><br><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user1" + ">Benutzer 1 &auml;ndern</a> (" + user1String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user2" + ">Benutzer 2 &auml;ndern</a> (" + user2String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user3" + ">Benutzer 3 &auml;ndern</a> (" + user3String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user4" + ">Benutzer 4 &auml;ndern</a> (" + user4String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user5" + ">Benutzer 5 &auml;ndern</a> (" + user5String + ") <br><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/predelete" + ">Alle Benutzer l&ouml;schen</a><br><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/tord" + ">Tastdauer &auml;ndern</a> (" + tord1String + "ms)<br>";
  Hauptseite += "<br><a href=http://" + IPaddress + "/firmware" + ">Firmware Aktualisieren</a><br>(Passowrt wird ben&ouml;tigt!)<br><br>";
  Hauptseite += "<br><br>Wifi-Signalst&auml;rke: <br>";
  Hauptseite += "" + String(rssi) +  " dBm<br>";
  Hauptseite += "<br>Uptime: <br>";
  Hauptseite += "" + uptime_formatter::getUptime() +  "";
  Hauptseite += "<br><br><a href=http://" + IPaddress + "/manuell" + ">Tor Tastern</a><br><br><br>";
  Hauptseite += "<small>Version 2.86</small><br>";
  Hauptseite += "</body>";
  Hauptseite += "</center>";
  Hauptseite += "</html>";
  httpServer.send(200, "text/html", Hauptseite);
}

void SetToken() {
  String IPaddress = WiFi.localIP().toString();
  String SetTokenString;
  fileSystem.openFromFile("/token.txt", SetTokenString);
  String SetToken;
  SetToken =  "<html>";
  SetToken += "<head><title>Neues Token speichern... </title>";
  SetToken += "<center>";
  SetToken += "<style>";
  SetToken += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  SetToken += "</style>";
  SetToken += "</head>";
  SetToken += "<body>";
  SetToken += "<h1><br>Token ( " + SetTokenString + " )</h1>";
  SetToken += "Bitte Token 2x eingeben!";
  SetToken += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  SetToken += "<form action='http://" + IPaddress + "/SetToken' method='POST'>";
  SetToken += "ID: <input type='text' name='name_input' size=30><br>";
  SetToken += "ID: <input type='text' name='name_input1' size=30><br><br>";
  SetToken += "<input type='submit' value='Token Speichern'>";
  SetToken += "</form>";
  SetToken += "<a href=http://" + IPaddress + "/SetToken" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  SetToken += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  SetToken += "</body>";
  SetToken += "</center>";
  SetToken += "</html>";
  httpServer.send(200, "text/html", SetToken);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/token.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
          delay(1000);
          ESP.restart();
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user1() {
  String IPaddress = WiFi.localIP().toString();
  String user1String;
  fileSystem.openFromFile("/user1.txt", user1String);
  String user1;
  user1 =  "<html>";
  user1 += "<head><title>Benutzer #1 speichern... </title>";
  user1 += "<center>";
  user1 += "<style>";
  user1 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user1 += "</style>";
  user1 += "</head>";
  user1 += "<body>";
  user1 += "<h1><br>Benutzer ID #1 ( " + user1String + " )</h1>";
  user1 += "Bitte ID 2x eingeben!";
  user1 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user1 += "<form action='http://" + IPaddress + "/user1' method='POST'>";
  user1 += "ID: <input type='text' name='name_input' size=12><br>";
  user1 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user1 += "<input type='submit' value='Benutzer ID Speichern'>";
  user1 += "</form>";
  user1 += "<a href=http://" + IPaddress + "/user1" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user1 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user1 += "</body>";
  user1 += "</center>";
  user1 += "</html>";
  httpServer.send(200, "text/html", user1);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user1.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user2() {
  String IPaddress = WiFi.localIP().toString();
  String user2String;
  fileSystem.openFromFile("/user2.txt", user2String);
  String user2;
  user2 =  "<html>";
  user2 += "<head><title>Benutzer #2 speichern... </title>";
  user2 += "<center>";
  user2 += "<style>";
  user2 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user2 += "</style>";
  user2 += "</head>";
  user2 += "<body>";
  user2 += "<h1><br>Benutzer ID #2 ( " + user2String + " )</h1>";
  user2 += "Bitte ID 2x eingeben!";
  user2 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user2 += "<form action='http://" + IPaddress + "/user2' method='POST'>";
  user2 += "ID: <input type='text' name='name_input' size=12><br>";
  user2 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user2 += "<input type='submit' value='Benutzer ID Speichern'>";
  user2 += "</form>";
  user2 += "<a href=http://" + IPaddress + "/user2" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user2 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user2 += "</body>";
  user2 += "</center>";
  user2 += "</html>";
  httpServer.send(200, "text/html", user2);

  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {

        // Writer...
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user2.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user3() {
  String IPaddress = WiFi.localIP().toString();
  String user3String;
  fileSystem.openFromFile("/user3.txt", user3String);
  String user3;
  user3 =  "<html>";
  user3 += "<head><title>Benutzer #3 speichern... </title>";
  user3 += "<center>";
  user3 += "<style>";
  user3 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user3 += "</style>";
  user3 += "</head>";
  user3 += "<body>";
  user3 += "<h1><br>Benutzer ID #3 ( " + user3String + " )</h1>";
  user3 += "Bitte ID 2x eingeben!";
  user3 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user3 += "<form action='http://" + IPaddress + "/user3' method='POST'>";
  user3 += "ID: <input type='text' name='name_input' size=12><br>";
  user3 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user3 += "<input type='submit' value='Benutzer ID Speichern'>";
  user3 += "</form>";
  user3 += "<a href=http://" + IPaddress + "/user3" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user3 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user3 += "</body>";
  user3 += "</center>";
  user3 += "</html>";
  httpServer.send(200, "text/html", user3);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {

        // Writer...
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user3.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user4() {
  String IPaddress = WiFi.localIP().toString();
  String user4String;
  fileSystem.openFromFile("/user4.txt", user4String);

  String user4;
  user4 =  "<html>";
  user4 += "<head><title>Benutzer #4 speichern... </title>";
  user4 += "<center>";
  user4 += "<style>";
  user4 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user4 += "</style>";
  user4 += "</head>";
  user4 += "<body>";
  user4 += "<h1><br>Benutzer ID #4 ( " + user4String + " )</h1>";
  user4 += "Bitte ID 2x eingeben!";
  user4 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user4 += "<form action='http://" + IPaddress + "/user4' method='POST'>";
  user4 += "ID: <input type='text' name='name_input' size=12><br>";
  user4 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user4 += "<input type='submit' value='Benutzer ID Speichern'>";
  user4 += "</form>";
  user4 += "<a href=http://" + IPaddress + "/user4" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user4 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user4 += "</body>";
  user4 += "</center>";
  user4 += "</html>";
  httpServer.send(200, "text/html", user4);

  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user4.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user5() {
  String IPaddress = WiFi.localIP().toString();
  String user5String;
  fileSystem.openFromFile("/user5.txt", user5String);

  String user5;
  user5 =  "<html>";
  user5 += "<head><title>Benutzer #5 speichern... </title>";
  user5 += "<center>";
  user5 += "<style>";
  user5 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user5 += "</style>";
  user5 += "</head>";
  user5 += "<body>";
  user5 += "<h1><br>Benutzer ID #5 ( " + user5String + " )</h1>";
  user5 += "Bitte ID 2x eingeben!";
  user5 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user5 += "<form action='http://" + IPaddress + "/user5' method='POST'>";
  user5 += "ID: <input type='text' name='name_input' size=12><br>";
  user5 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user5 += "<input type='submit' value='Benutzer ID Speichern'>";
  user5 += "</form>";
  user5 += "<a href=http://" + IPaddress + "/user5" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user5 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user5 += "</body>";
  user5 += "</center>";
  user5 += "</html>";
  httpServer.send(200, "text/html", user5);

  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user5.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void ShowUsers() {
  String IPaddress = WiFi.localIP().toString();
  String user1String;
  String user2String;
  String user3String;
  String user4String;
  String user5String;
  fileSystem.openFromFile("/user1.txt", user1String);
  fileSystem.openFromFile("/user2.txt", user2String);
  fileSystem.openFromFile("/user3.txt", user3String);
  fileSystem.openFromFile("/user4.txt", user4String);
  fileSystem.openFromFile("/user5.txt", user5String);
  String userspage;
  userspage =  "<html>";
  userspage += "<head><title>Benutzer anzeigen... </title>";
  userspage += "<center>";
  userspage += "<style>";
  userspage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  userspage += "</style>";
  userspage += "</head>";
  userspage += "<body>";
  userspage += "<h1><br>Gespeicherte Benutzer </h1><br>";
  userspage += "Benutzer 1:<br>" + user1String + "<br><br>";
  userspage += "Benutzer 2:<br>" + user2String + "<br><br>";
  userspage += "Benutzer 3:<br>" + user3String + "<br><br>";
  userspage += "Benutzer 4:<br>" + user4String + "<br><br>";
  userspage += "Benutzer 5:<br>" + user5String + "<br><br><br>";
  userspage += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  userspage += "</body>";
  userspage += "</center>";
  userspage += "</html>";
  httpServer.send(200, "text/html", userspage);
}

void predelete() {
  String IPaddress = WiFi.localIP().toString();
  String predelete;
  predelete =  "<html>";
  predelete += "<head><title>GaragenESP </title>";
  predelete += "<center>";
  predelete += "<style>";
  predelete += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  predelete += "</style>";
  predelete += "</head>";
  predelete += "<body>";
  predelete += "<h1><br>Wirklich alle Benutzer L&ouml;schen?</h1>";
  predelete += "<br><a href=http://" + IPaddress + "/deleter" + ">JA, wirklich ALLE Benutzer l&ouml;schen</a><br><br>";
  predelete += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  predelete += "</body>";
  predelete += "</center>";
  predelete += "</html>";
  httpServer.send(200, "text/html", predelete); // Send a response to the client asking for input
}

void deleter() {
  auto a = gfs.open("/user1.txt", "w");
  a.close();
  auto b = gfs.open("/user2.txt", "w");
  b.close();
  auto c = gfs.open("/user3.txt", "w");
  c.close();
  auto d = gfs.open("/user4.txt", "w");
  d.close();
  auto e = gfs.open("/user5.txt", "w");
  e.close();
  String IPaddress = WiFi.localIP().toString();
  String deleter;
  deleter =  "<html>";
  deleter += "<head><title>GaragenESP </title>";
  deleter += "<center>";
  deleter += "<style>";
  deleter += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  deleter += "</style>";
  deleter += "</head>";
  deleter += "<body>";
  deleter += "<h1><br>Alle benutzer Gel&ouml;scht!</h1>";
  deleter += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  deleter += "</body>";
  deleter += "</center>";
  deleter += "</html>";
  httpServer.send(200, "text/html", deleter); // Send a response to the client asking for input
}

void st() {
  String IPaddress = WiFi.localIP().toString();
  String tokenString;
  fileSystem.openFromFile("/token.txt", tokenString);

  String stpage;
  stpage =  "<html>";
  stpage += "<head><title>Gespeicherten Token anzeigen... </title>";
  stpage += "<center>";
  stpage += "<style>";
  stpage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  stpage += "</style>";
  stpage += "</head>";
  stpage += "<body>";
  stpage += "<h1><br>Garagen Token</h1>";
  stpage += "Gespeicherter Token:<br><br><h1><br>" + tokenString + "</h1><br><br>";
  stpage += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  stpage += "</body>";
  stpage += "</center>";
  stpage += "</html>";
  httpServer.send(200, "text/html", stpage);
}

void tord() {
  String IPaddress = WiFi.localIP().toString();
  String tordString;
  fileSystem.openFromFile("/tord.txt", tordString);
  String tord;
  tord =  "<html>";
  tord += "<head><title>Tastzeit Tor</title>";
  tord += "<center>";
  tord += "<style>";
  tord += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  tord += "</style>";
  tord += "</head>";
  tord += "<body>";
  tord += "<h1><br>Tastdauer</h1>";
  tord += "Aktuelle Tastdauer: " + tordString + "ms<br><br>";
  tord += "<form action='http://" + IPaddress + "/tord' method='POST'>";
  tord += "Neue Tastdauer: <br><input type='text' name='name_input' size=5>ms<br>";
  tord += "<input type='submit' value='Tastdauer Speichern'>";
  tord += "</form>";
  tord += "<a href=http://" + IPaddress + "/tord" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  tord += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  tord += "</body>";
  tord += "</center>";
  tord += "</html>";
  httpServer.send(200, "text/html", tord);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      auto f = gfs.open("/tord.txt", "w");
      f.print(httpServer.arg(0));
      f.close();
    }
  }
}

void manuell() {
  int tord1String;
  String IPaddress = WiFi.localIP().toString();
  String manuell;
  manuell =  "<html>";
  manuell += "<head><title>GaragenESP </title>";
  manuell += "<center>";
  manuell += "<style>";
  manuell += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  manuell += "</style>";
  manuell += "</head>";
  manuell += "<body>";
  manuell += "<h1><br>Tor wurde getastert!</h1>";
  manuell += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  manuell += "</body>";
  manuell += "</center>";
  manuell += "</html>";
  httpServer.send(200, "text/html", manuell); // Send a response to the client asking for input
  fileSystem.openFromFile("/tord.txt", tord1String);
  digitalWrite(torPin, LOW); // Relais 1
  delay(tord1String);
  digitalWrite(torPin, HIGH);
}

void setup() {
  MDNS.begin(host);
  Serial.begin(115200);
  secured_client.setInsecure();
  Serial.println(" ");
  pinMode(rtaster, INPUT_PULLUP);
  pinMode(torPin, OUTPUT);
  pinMode(lichtPin, OUTPUT);
  digitalWrite(lichtPin, HIGH);
  digitalWrite(torPin, HIGH);
  delay(10);
  Serial.setDebugOutput(true);
  WifiVerbinder();
  //  Serial.println(WiFi.localIP());
  secured_client.setTrustAnchors(&cert);
  Serverstarten();
  ntp();
  tokenGetter();
  webupdater();
  OTA();
  Tastdauerchecker();
  //  wifirssi();
  // Serial.println(WiFi.SSID());
}

void Tastdauerchecker() {
  int tord;
  tord = gfs.exists("/tord.txt");
  if (tord == 1) {
  }
  if (tord == 0) {
    auto f = gfs.open("/tord.txt", "w");
    f.print("251");
    f.close();
  }
}

void Serverstarten() {
  httpServer.begin();
  httpServer.on("/", Hauptseite);
  httpServer.on("/st", st); // Show Token
  httpServer.on("/SetToken", SetToken);
  httpServer.on("/user1", user1);
  httpServer.on("/user2", user2);
  httpServer.on("/user3", user3);
  httpServer.on("/user4", user4);
  httpServer.on("/user5", user5);
  httpServer.on("/ShowUsers", ShowUsers);
  httpServer.on("/predelete", predelete);
  httpServer.on("/deleter", deleter);
  httpServer.on("/tord", tord);
  httpServer.on("/manuell", manuell);/*
   Copyright (c) 2021 Peter Geher. All rights reserved.
   See the bottom of this file for the license terms.
                peter.geher@nsa2go.de
*/

// Version 2.86 - 05.01.2022 / 23:30
// All-in-One Version!

/*
    Drin/Erledigt ist:

    WWW-Server für Token ansehen/neu setzen
    Token in spiffs speichern
    Token aus spiffs lesen und verwenden

    Wifi-Connector
    Wifi-Connector-Deleter per Pin D7 an Ground

    NTP-Client

    OTA für Entwicklung!

    Delete aller Nutzer mit einen Klick. Erledigt,
    Zwei Seiten zum zum Löschen reichen.

    User-Limit auf auf userN1-5.txt aus spiffs

    Status von Relais2 anzeigen und warnen,
    falls es nochmals ein/ausgeschaltet werden soll

    Fallback: Da gerade von allen Seiten gegen Telegram geschossen
    wird, habe ich eine Manuelle Tasterung eingebaut. Laesst sich
    auch per curl o.Ae. triggern.

    Wenn Wifi wegbricht versucht er noch noch 30 sekunden
    bevor das Wifi-Portal fuer 120 Sekunden gestartet wird. 
    Meldet sich niemand in den 120 Sekunden an, startet der
    esp neu und alles noch mal von vorn bis entweder:
    Bekanntes Wifi wieder erreichbar oder
    Neues Wifi im Portal eingestellt. 

    Es fehlen immer noch Party-Alpakas...
    Und Bier... :-)
*/

// Wifi-RSSi
const int amazing = -30;
const int verygood = -67;
const int okay = -70;
const int notgood = -80;
const int unuseable = -90;
// Wifi-RSSi

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <UniversalTelegramBot.h>
#include <time.h>
#include <LittleFS.h>
#include <Effortless_SPIFFS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "uptime_formatter.h"
#include "uptime.h"

#define BOT_TOKEN ""
const unsigned long BOT_MTBS = 500;

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

FS& gfs = LittleFS;
eSPIFFS fileSystem;

const int lichtPin = D2;
const int torPin = D5;
int tastdauer = 150; // in Millisekunden
int rtaster = D7; // Taster fuer Wifi-Reset!
int tasterstatus = 0;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    String from_id = bot.messages[i].from_id;

    // ChatID Geraffel
    String user1String;
    String user2String;
    String user3String;
    String user4String;
    String user5String;

    fileSystem.openFromFile("/user1.txt", user1String);
    fileSystem.openFromFile("/user2.txt", user2String);
    fileSystem.openFromFile("/user3.txt", user3String);
    fileSystem.openFromFile("/user4.txt", user4String);
    fileSystem.openFromFile("/user5.txt", user5String);

    if (chat_id != user1String) {
      if (chat_id != user2String) {
        if (chat_id != user3String) {
          if (chat_id != user4String) {
            if (chat_id != user5String) {
              //bot.sendMessage(chat_id, "Du nicht!", "");
              continue;
            }
          }
        }
      }
    }

    // ChatID Geraffel
    if (text == "/start")
    {
      String welcome = "Hallo und Herzlich willkommen, " + from_name + ".\n\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
    if (text == "tor")
    {
      int tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      digitalWrite(torPin, LOW); // Relais 1
      bot.sendMessage(chat_id, " Tor wird bewegt ", "");
      delay(tord1String);
      digitalWrite(torPin, HIGH);
    }

    if (text == "/tor")
    {
      int tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      digitalWrite(torPin, LOW); // Relais 1
      bot.sendMessage(chat_id, " Tor wird bewegt ", "");
      delay(tord1String);
      digitalWrite(torPin, HIGH);
    }
 
    /*
        if (text == "/users")
        {
          String user1String;
          String user2String;
          String user3String;
          String user4String;
          String user5String;
          fileSystem.openFromFile("/user1.txt", user1String);
          fileSystem.openFromFile("/user2.txt", user2String);
          fileSystem.openFromFile("/user3.txt", user3String);
          fileSystem.openFromFile("/user4.txt", user4String);
          fileSystem.openFromFile("/user5.txt", user5String);
          String welcome = "Erlaubte Benutzer:\n";
          welcome += "1.) " + user1String + "\n";
          welcome += "2.) " + user2String + "\n";
          welcome += "3.) " + user3String + "\n";
          welcome += "4.) " + user4String + "\n";
          welcome += "5.) " + user5String + "\n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    if (text == "/status")
    {
      String tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      //      String firmwarev;
      //      String firmwared;
      String rssi1String;
      int32_t rssi = wifi_station_get_rssi();

      if (rssi < unuseable) { // unter -90
        rssi1String = "Wifi so nicht benutzbar, sorry";
      }
      else if (rssi >= notgood && rssi < okay) {        // zwischen -80 und -70
        rssi1String = "Na, nicht wirklich gut!";
      }
      else if (rssi >= okay && rssi < verygood) {         // Zwischen -70 und -67
        rssi1String = "Des passt scho!";
      }
      else if (rssi >= verygood && rssi < amazing) {      // zwischen -30 und -67
        rssi1String = "Absolut Stark :-)";
      }
      String user1String;
      String user2String;
      String user3String;
      String user4String;
      String user5String;
      fileSystem.openFromFile("/user1.txt", user1String);
      fileSystem.openFromFile("/user2.txt", user2String);
      fileSystem.openFromFile("/user3.txt", user3String);
      fileSystem.openFromFile("/user4.txt", user4String);
      fileSystem.openFromFile("/user5.txt", user5String);

      uptime::calculateUptime();
      String IPaddress = WiFi.localIP().toString();
      String welcome = " ESP Bot - Version 2.86 - 05.01.2022 \n\n";
      welcome += "Verbunden mit SSID:\n";
      welcome += WiFi.SSID();
      welcome += "\n\n";
      welcome += "Wifi-SignalstÃ¤rke: \n";
      welcome += rssi;
      welcome += " dBm - ";
      welcome += rssi1String;
      welcome += "\n\n";
      welcome += "IP-Adresse: ";
      welcome += IPaddress + "\n\n";
      welcome += "Laufzeit:\n";
      welcome += uptime_formatter::getUptime() + "\n\n";
      welcome += "Tor-Tastdauer: \n";
      welcome += "" + tord1String + "ms\n\n";
      welcome += "Erlaubte Benutzer:\n";
      welcome += user1String + "\n";
      welcome += user2String + "\n";
      welcome += user3String + "\n";
      welcome += user4String + "\n";
      welcome += user5String + "\n\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
      Serial.println("Debug: /Status");
    }
    /* /uptime
      if (text == "/uptime")
      {
      String welcome = "Laufzeit:\n";
      welcome += uptime_formatter::getUptime() + "\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
      wifirssi();
      }
    */
    /* // /an
        if (text == "/an") // Relais 2
        {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 wird eingeschaltet\n";
            digitalWrite(lichtPin, LOW);
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 ist schon eingeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // an
        if (text == "an") // Relais 2
       {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 wird eingeschaltet\n";
            digitalWrite(lichtPin, LOW);
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 ist schon eingeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // aus
        if (text == "aus") // Relais 2
        {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 ist schon ausgeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 wird ausgeschaltet\n";
            digitalWrite(lichtPin, HIGH);
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // //aus
        if (text == "/aus") // Relais 2
        {
          if (digitalRead(lichtPin)) {
            String welcome = "Relais2 ist schon ausgeschaltet!\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
          } else {
            String welcome = "Relais2 wird ausgeschaltet\n";
            digitalWrite(lichtPin, HIGH);
            bot.sendMessage(chat_id, welcome, "Markdown");
          }
        }
    */
    /* // /hilfe
        if (text == "/hilfe")
        {
          int tord1String;
          fileSystem.openFromFile("/tord.txt", tord1String);

          String welcome = "Willkommen beim Garagentor, " + from_name + ".\n\n";
          welcome += "/tor    Tastert den Tor-Taster\n";
          welcome += "/an     Schaltet Relais2 an \n";
          welcome += "/aus    Schaltet Relais2 aus \n";
          welcome += "/users  Zeigt aktuell gespeicherte Benutzer an\n";
          welcome += "/ip     zeigt die IP-Adresse vom ESP an\n";
          welcome += "/status     zeigt den Status von Relais2 an\n";
          welcome += "/uptime   Zeigt die Laufzeit vom ESP an\n";
          welcome += " \n";
          welcome += "'tor' 'an' und 'aus' sind auch ohne '/' möglich. \n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // status
        if (text == "status")
        {
          String welcome = "Relais2 ist ";
          if (digitalRead(lichtPin)) {
            welcome += "aus\n";
          } else {
            welcome += "an\n";
          }
          bot.sendMessage(chat_id, welcome, "Markdown");
          Serial.println("status");
        }


        if (text == "/status")
        {
          String welcome = "Relais2 ist ";
          if (digitalRead(lichtPin)) {
            welcome += "aus\n";
          } else {
            welcome += "an\n";
          }
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // /about
        if (text == "/about")
        {
          String welcome = "Garage-Controller - ";
          welcome += "This little code controls your garage door using ";
          welcome += "one (or two) simple relays and a Wemos D1-Mini. ";
          welcome += "\n\n";
          welcome += "It is forbidden to use this code ";
          welcome += "commercially/profitably or for police/military purposes! ";
          welcome += "\n\n ";
          welcome += "2022 by Peter Geher\n";
          welcome += "peter.geher@nsa2go.de\n";
          welcome += "http://www.layers-gedanken.de\n";
          welcome += " \n";
          welcome += "Have a lot of Fun! \n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // /ip
        if (text == "/ip")
        {
          String IPaddress = WiFi.localIP().toString();
          String welcome = "Das Tor hat die IP:\n";
          welcome += IPaddress + "\n";
          bot.sendMessage(chat_id, welcome, "Markdown");
        }
    */
    /* // /tord
      if (text == "/tord")
      {
      String tord1String;
      fileSystem.openFromFile("/tord.txt", tord1String);
      String welcome = "Tastdauer: ";
      welcome += "" + tord1String + "ms\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
      }
    */
  }
}

unsigned long previousMillis = 0;
const long blinkinterval = 125;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String Argument_Name, Clients_Response1, Clients_Response2;
const char* host = "garagen-esp";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";


void WifiVerbinder() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);

  WiFiManager wm;
  bool res;
  wm.setConnectTimeout(30);
  wm.setConfigPortalTimeout(120);
  res = wm.autoConnect("Garage");

  if (!res) {
    Serial.println("Debug: Failed to connect");
    delay(1000);
    Serial.println("Debug: Reboot!");
    delay(250);
    ESP.reset();
  }
  else {
    Serial.println("Debug: Verbunden... yay");
  }
}


void WifiDeleter() {

  // Wird nicht wirklich gebraucht, daher ist es auch im Loop auskommentiert!

  unsigned long currentMillis = millis();
  tasterstatus = digitalRead(rtaster);
  if (currentMillis - previousMillis >= blinkinterval) {
    previousMillis = currentMillis;
    if (tasterstatus == HIGH) {
    } else {
      Serial.println("Flash-Reset getastet!!!");
      ESP.eraseConfig();
      delay(100);
      ESP.reset();
    }
  }
}

void OTA() {
  ArduinoOTA.setHostname("garagen8266");
  ArduinoOTA.onStart([]() {
    String type;
    type = "sketch";
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
}

void ntp() {
  // NTP-Zeiten beziehen für Telegram-Cert...
  configTime(0, 0, "194.25.134.196");   // T-Online-Zeitserver
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {

    delay(100);
    now = time(nullptr);
  }
}

void webupdater() {
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
}

void tokenGetter() {
  const char* blah;
  fileSystem.openFromFile("/token.txt", blah);
  void updateToken(const char* blah);
  bot.updateToken(blah);
}

void Hauptseite() {
  String user1String;
  String user2String;
  String user3String;
  String user4String;
  String user5String;
  String tord1String;
  fileSystem.openFromFile("/user1.txt", user1String);
  fileSystem.openFromFile("/user2.txt", user2String);
  fileSystem.openFromFile("/user3.txt", user3String);
  fileSystem.openFromFile("/user4.txt", user4String);
  fileSystem.openFromFile("/user5.txt", user5String);
  fileSystem.openFromFile("/tord.txt", tord1String);

  int32_t rssi = wifi_station_get_rssi();

  String IPaddress = WiFi.localIP().toString();
  String Hauptseite;
  Hauptseite =  "<html>";
  Hauptseite += "<head><title>ESP Bot </title>";
  Hauptseite += "<center>";
  Hauptseite += "<style>";
  Hauptseite += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  Hauptseite += "</style>";
  Hauptseite += "</head>";
  Hauptseite += "<body><br>";
  Hauptseite += "<h1><br>ESP Bot &Uuml;bersicht</h1>";
  Hauptseite += "<a href=http://" + IPaddress + "/st" + ">Token anzeigen</a><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/SetToken" + ">Neues Token setzen</a><br><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user1" + ">Benutzer 1 &auml;ndern</a> (" + user1String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user2" + ">Benutzer 2 &auml;ndern</a> (" + user2String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user3" + ">Benutzer 3 &auml;ndern</a> (" + user3String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user4" + ">Benutzer 4 &auml;ndern</a> (" + user4String + ") <br>";
  Hauptseite += "<a href=http://" + IPaddress + "/user5" + ">Benutzer 5 &auml;ndern</a> (" + user5String + ") <br><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/predelete" + ">Alle Benutzer l&ouml;schen</a><br><br>";
  Hauptseite += "<a href=http://" + IPaddress + "/tord" + ">Tastdauer &auml;ndern</a> (" + tord1String + "ms)<br>";
  Hauptseite += "<br><a href=http://" + IPaddress + "/firmware" + ">Firmware Aktualisieren</a><br>(Passowrt wird ben&ouml;tigt!)<br><br>";
  Hauptseite += "<br><br>Wifi-Signalst&auml;rke: <br>";
  Hauptseite += "" + String(rssi) +  " dBm<br>";
  Hauptseite += "<br>Uptime: <br>";
  Hauptseite += "" + uptime_formatter::getUptime() +  "";
  Hauptseite += "<br><br><a href=http://" + IPaddress + "/manuell" + ">Tor Tastern</a><br><br><br>";
  Hauptseite += "<small>Version 2.86</small><br>";
  Hauptseite += "</body>";
  Hauptseite += "</center>";
  Hauptseite += "</html>";
  httpServer.send(200, "text/html", Hauptseite);
}

void SetToken() {
  String IPaddress = WiFi.localIP().toString();
  String SetTokenString;
  fileSystem.openFromFile("/token.txt", SetTokenString);
  String SetToken;
  SetToken =  "<html>";
  SetToken += "<head><title>Neues Token speichern... </title>";
  SetToken += "<center>";
  SetToken += "<style>";
  SetToken += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  SetToken += "</style>";
  SetToken += "</head>";
  SetToken += "<body>";
  SetToken += "<h1><br>Token ( " + SetTokenString + " )</h1>";
  SetToken += "Bitte Token 2x eingeben!";
  SetToken += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  SetToken += "<form action='http://" + IPaddress + "/SetToken' method='POST'>";
  SetToken += "ID: <input type='text' name='name_input' size=30><br>";
  SetToken += "ID: <input type='text' name='name_input1' size=30><br><br>";
  SetToken += "<input type='submit' value='Token Speichern'>";
  SetToken += "</form>";
  SetToken += "<a href=http://" + IPaddress + "/SetToken" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  SetToken += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  SetToken += "</body>";
  SetToken += "</center>";
  SetToken += "</html>";
  httpServer.send(200, "text/html", SetToken);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/token.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
          delay(1000);
          ESP.restart();
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user1() {
  String IPaddress = WiFi.localIP().toString();
  String user1String;
  fileSystem.openFromFile("/user1.txt", user1String);
  String user1;
  user1 =  "<html>";
  user1 += "<head><title>Benutzer #1 speichern... </title>";
  user1 += "<center>";
  user1 += "<style>";
  user1 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user1 += "</style>";
  user1 += "</head>";
  user1 += "<body>";
  user1 += "<h1><br>Benutzer ID #1 ( " + user1String + " )</h1>";
  user1 += "Bitte ID 2x eingeben!";
  user1 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user1 += "<form action='http://" + IPaddress + "/user1' method='POST'>";
  user1 += "ID: <input type='text' name='name_input' size=12><br>";
  user1 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user1 += "<input type='submit' value='Benutzer ID Speichern'>";
  user1 += "</form>";
  user1 += "<a href=http://" + IPaddress + "/user1" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user1 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user1 += "</body>";
  user1 += "</center>";
  user1 += "</html>";
  httpServer.send(200, "text/html", user1);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user1.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user2() {
  String IPaddress = WiFi.localIP().toString();
  String user2String;
  fileSystem.openFromFile("/user2.txt", user2String);
  String user2;
  user2 =  "<html>";
  user2 += "<head><title>Benutzer #2 speichern... </title>";
  user2 += "<center>";
  user2 += "<style>";
  user2 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user2 += "</style>";
  user2 += "</head>";
  user2 += "<body>";
  user2 += "<h1><br>Benutzer ID #2 ( " + user2String + " )</h1>";
  user2 += "Bitte ID 2x eingeben!";
  user2 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user2 += "<form action='http://" + IPaddress + "/user2' method='POST'>";
  user2 += "ID: <input type='text' name='name_input' size=12><br>";
  user2 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user2 += "<input type='submit' value='Benutzer ID Speichern'>";
  user2 += "</form>";
  user2 += "<a href=http://" + IPaddress + "/user2" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user2 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user2 += "</body>";
  user2 += "</center>";
  user2 += "</html>";
  httpServer.send(200, "text/html", user2);

  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {

        // Writer...
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user2.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user3() {
  String IPaddress = WiFi.localIP().toString();
  String user3String;
  fileSystem.openFromFile("/user3.txt", user3String);
  String user3;
  user3 =  "<html>";
  user3 += "<head><title>Benutzer #3 speichern... </title>";
  user3 += "<center>";
  user3 += "<style>";
  user3 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user3 += "</style>";
  user3 += "</head>";
  user3 += "<body>";
  user3 += "<h1><br>Benutzer ID #3 ( " + user3String + " )</h1>";
  user3 += "Bitte ID 2x eingeben!";
  user3 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user3 += "<form action='http://" + IPaddress + "/user3' method='POST'>";
  user3 += "ID: <input type='text' name='name_input' size=12><br>";
  user3 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user3 += "<input type='submit' value='Benutzer ID Speichern'>";
  user3 += "</form>";
  user3 += "<a href=http://" + IPaddress + "/user3" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user3 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user3 += "</body>";
  user3 += "</center>";
  user3 += "</html>";
  httpServer.send(200, "text/html", user3);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {

        // Writer...
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user3.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user4() {
  String IPaddress = WiFi.localIP().toString();
  String user4String;
  fileSystem.openFromFile("/user4.txt", user4String);

  String user4;
  user4 =  "<html>";
  user4 += "<head><title>Benutzer #4 speichern... </title>";
  user4 += "<center>";
  user4 += "<style>";
  user4 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user4 += "</style>";
  user4 += "</head>";
  user4 += "<body>";
  user4 += "<h1><br>Benutzer ID #4 ( " + user4String + " )</h1>";
  user4 += "Bitte ID 2x eingeben!";
  user4 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user4 += "<form action='http://" + IPaddress + "/user4' method='POST'>";
  user4 += "ID: <input type='text' name='name_input' size=12><br>";
  user4 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user4 += "<input type='submit' value='Benutzer ID Speichern'>";
  user4 += "</form>";
  user4 += "<a href=http://" + IPaddress + "/user4" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user4 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user4 += "</body>";
  user4 += "</center>";
  user4 += "</html>";
  httpServer.send(200, "text/html", user4);

  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user4.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void user5() {
  String IPaddress = WiFi.localIP().toString();
  String user5String;
  fileSystem.openFromFile("/user5.txt", user5String);

  String user5;
  user5 =  "<html>";
  user5 += "<head><title>Benutzer #5 speichern... </title>";
  user5 += "<center>";
  user5 += "<style>";
  user5 += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  user5 += "</style>";
  user5 += "</head>";
  user5 += "<body>";
  user5 += "<h1><br>Benutzer ID #5 ( " + user5String + " )</h1>";
  user5 += "Bitte ID 2x eingeben!";
  user5 += "<br><br><br>Beide Felder leer lassen zum l&ouml;schen";
  user5 += "<form action='http://" + IPaddress + "/user5' method='POST'>";
  user5 += "ID: <input type='text' name='name_input' size=12><br>";
  user5 += "ID: <input type='text' name='name_input1' size=12><br><br>";
  user5 += "<input type='submit' value='Benutzer ID Speichern'>";
  user5 += "</form>";
  user5 += "<a href=http://" + IPaddress + "/user5" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  user5 += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  user5 += "</body>";
  user5 += "</center>";
  user5 += "</html>";
  httpServer.send(200, "text/html", user5);

  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      if (httpServer.argName(i) == "name_input") {
        if (httpServer.arg(0) == httpServer.arg(1)) {
          auto f = gfs.open("/user5.txt", "w");
          f.print(httpServer.arg(i));
          f.close();
          Serial.println("gleich");
        }
        else
        {
          Serial.println("Ungleich");
        }
      }
    }
  }
}

void ShowUsers() {
  String IPaddress = WiFi.localIP().toString();
  String user1String;
  String user2String;
  String user3String;
  String user4String;
  String user5String;
  fileSystem.openFromFile("/user1.txt", user1String);
  fileSystem.openFromFile("/user2.txt", user2String);
  fileSystem.openFromFile("/user3.txt", user3String);
  fileSystem.openFromFile("/user4.txt", user4String);
  fileSystem.openFromFile("/user5.txt", user5String);
  String userspage;
  userspage =  "<html>";
  userspage += "<head><title>Benutzer anzeigen... </title>";
  userspage += "<center>";
  userspage += "<style>";
  userspage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  userspage += "</style>";
  userspage += "</head>";
  userspage += "<body>";
  userspage += "<h1><br>Gespeicherte Benutzer </h1><br>";
  userspage += "Benutzer 1:<br>" + user1String + "<br><br>";
  userspage += "Benutzer 2:<br>" + user2String + "<br><br>";
  userspage += "Benutzer 3:<br>" + user3String + "<br><br>";
  userspage += "Benutzer 4:<br>" + user4String + "<br><br>";
  userspage += "Benutzer 5:<br>" + user5String + "<br><br><br>";
  userspage += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  userspage += "</body>";
  userspage += "</center>";
  userspage += "</html>";
  httpServer.send(200, "text/html", userspage);
}

void predelete() {
  String IPaddress = WiFi.localIP().toString();
  String predelete;
  predelete =  "<html>";
  predelete += "<head><title>GaragenESP </title>";
  predelete += "<center>";
  predelete += "<style>";
  predelete += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  predelete += "</style>";
  predelete += "</head>";
  predelete += "<body>";
  predelete += "<h1><br>Wirklich alle Benutzer L&ouml;schen?</h1>";
  predelete += "<br><a href=http://" + IPaddress + "/deleter" + ">JA, wirklich ALLE Benutzer l&ouml;schen</a><br><br>";
  predelete += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  predelete += "</body>";
  predelete += "</center>";
  predelete += "</html>";
  httpServer.send(200, "text/html", predelete); // Send a response to the client asking for input
}

void deleter() {
  auto a = gfs.open("/user1.txt", "w");
  a.close();
  auto b = gfs.open("/user2.txt", "w");
  b.close();
  auto c = gfs.open("/user3.txt", "w");
  c.close();
  auto d = gfs.open("/user4.txt", "w");
  d.close();
  auto e = gfs.open("/user5.txt", "w");
  e.close();
  String IPaddress = WiFi.localIP().toString();
  String deleter;
  deleter =  "<html>";
  deleter += "<head><title>GaragenESP </title>";
  deleter += "<center>";
  deleter += "<style>";
  deleter += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  deleter += "</style>";
  deleter += "</head>";
  deleter += "<body>";
  deleter += "<h1><br>Alle benutzer Gel&ouml;scht!</h1>";
  deleter += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  deleter += "</body>";
  deleter += "</center>";
  deleter += "</html>";
  httpServer.send(200, "text/html", deleter); // Send a response to the client asking for input
}

void st() {
  String IPaddress = WiFi.localIP().toString();
  String tokenString;
  fileSystem.openFromFile("/token.txt", tokenString);

  String stpage;
  stpage =  "<html>";
  stpage += "<head><title>Gespeicherten Token anzeigen... </title>";
  stpage += "<center>";
  stpage += "<style>";
  stpage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  stpage += "</style>";
  stpage += "</head>";
  stpage += "<body>";
  stpage += "<h1><br>Garagen Token</h1>";
  stpage += "Gespeicherter Token:<br><br><h1><br>" + tokenString + "</h1><br><br>";
  stpage += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  stpage += "</body>";
  stpage += "</center>";
  stpage += "</html>";
  httpServer.send(200, "text/html", stpage);
}

void tord() {
  String IPaddress = WiFi.localIP().toString();
  String tordString;
  fileSystem.openFromFile("/tord.txt", tordString);
  String tord;
  tord =  "<html>";
  tord += "<head><title>Tastzeit Tor</title>";
  tord += "<center>";
  tord += "<style>";
  tord += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  tord += "</style>";
  tord += "</head>";
  tord += "<body>";
  tord += "<h1><br>Tastdauer</h1>";
  tord += "Aktuelle Tastdauer: " + tordString + "ms<br><br>";
  tord += "<form action='http://" + IPaddress + "/tord' method='POST'>";
  tord += "Neue Tastdauer: <br><input type='text' name='name_input' size=5>ms<br>";
  tord += "<input type='submit' value='Tastdauer Speichern'>";
  tord += "</form>";
  tord += "<a href=http://" + IPaddress + "/tord" + ">Seite neu laden. Bitte kein Reload benutzen!</a><br><br>";
  tord += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  tord += "</body>";
  tord += "</center>";
  tord += "</html>";
  httpServer.send(200, "text/html", tord);
  if (httpServer.args() > 0 ) {
    for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
      Argument_Name = httpServer.argName(i);
      auto f = gfs.open("/tord.txt", "w");
      f.print(httpServer.arg(0));
      f.close();
    }
  }
}

void manuell() {
  int tord1String;
  String IPaddress = WiFi.localIP().toString();
  String manuell;
  manuell =  "<html>";
  manuell += "<head><title>GaragenESP </title>";
  manuell += "<center>";
  manuell += "<style>";
  manuell += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  manuell += "</style>";
  manuell += "</head>";
  manuell += "<body>";
  manuell += "<h1><br>Tor wurde getastert!</h1>";
  manuell += "<a href=http://" + IPaddress + "/" + ">Zur&uuml;ck zur Hauptseite</a>";
  manuell += "</body>";
  manuell += "</center>";
  manuell += "</html>";
  httpServer.send(200, "text/html", manuell); // Send a response to the client asking for input
  fileSystem.openFromFile("/tord.txt", tord1String);
  digitalWrite(torPin, LOW); // Relais 1
  delay(tord1String);
  digitalWrite(torPin, HIGH);
}

void setup() {
  MDNS.begin(host);
  Serial.begin(115200);
  secured_client.setInsecure();
  Serial.println(" ");
  pinMode(rtaster, INPUT_PULLUP);
  pinMode(torPin, OUTPUT);
  pinMode(lichtPin, OUTPUT);
  digitalWrite(lichtPin, HIGH);
  digitalWrite(torPin, HIGH);
  delay(10);
  Serial.setDebugOutput(true);
  WifiVerbinder();
  //  Serial.println(WiFi.localIP());
  secured_client.setTrustAnchors(&cert);
  Serverstarten();
  ntp();
  tokenGetter();
  webupdater();
  OTA();
  Tastdauerchecker();
  //  wifirssi();
  // Serial.println(WiFi.SSID());
}

void Tastdauerchecker() {
  int tord;
  tord = gfs.exists("/tord.txt");
  if (tord == 1) {
  }
  if (tord == 0) {
    auto f = gfs.open("/tord.txt", "w");
    f.print("251");
    f.close();
  }
}

void Serverstarten() {
  httpServer.begin();
  httpServer.on("/", Hauptseite);
  httpServer.on("/st", st); // Show Token
  httpServer.on("/SetToken", SetToken);
  httpServer.on("/user1", user1);
  httpServer.on("/user2", user2);
  httpServer.on("/user3", user3);
  httpServer.on("/user4", user4);
  httpServer.on("/user5", user5);
  httpServer.on("/ShowUsers", ShowUsers);
  httpServer.on("/predelete", predelete);
  httpServer.on("/deleter", deleter);
  httpServer.on("/tord", tord);
  httpServer.on("/manuell", manuell);
}

void wifirssi() {
  int32_t rssi = wifi_station_get_rssi();
  /*
    amazing = -30;
    verygood = -67;
    okay = -70;
    notgood = -80;
    unuseable = -90;
  */
  if (rssi < unuseable) { // unter -90
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.print("Wifi ist so nicht nutzbar!");
  }

  else if (rssi >= notgood && rssi < okay) {        // zwischen -80 und -70
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.println("Nicht gut!");
  }

  else if (rssi >= okay && rssi < verygood) {         // Zwischen -70 und -67
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.println("Alles gut!");
  }

  else if (rssi >= verygood && rssi < amazing) {      // zwischen -30 und -67
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.println("WOW, PERFEKT!");
  }
}

void loop() {
  {
    if (millis() - bot_lasttime > BOT_MTBS)
    {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      while (numNewMessages)
      {
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      bot_lasttime = millis();
    }
  }
  httpServer.handleClient();
  ArduinoOTA.handle(); // Auskommentieren!
  //WifiDeleter();
}

// Version 2.86 - 05.01.2022 / 23:30
// All-in-One Version!


/*
   Copyright (c) 2022 Peter Geher

   This Code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This Code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this Code; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

}

void wifirssi() {
  int32_t rssi = wifi_station_get_rssi();
  /*
    amazing = -30;
    verygood = -67;
    okay = -70;
    notgood = -80;
    unuseable = -90;
  */
  if (rssi < unuseable) { // unter -90
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.print("Wifi ist so nicht nutzbar!");
  }

  else if (rssi >= notgood && rssi < okay) {        // zwischen -80 und -70
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.println("Nicht gut!");
  }

  else if (rssi >= okay && rssi < verygood) {         // Zwischen -70 und -67
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.println("Alles gut!");
  }

  else if (rssi >= verygood && rssi < amazing) {      // zwischen -30 und -67
    Serial.print(rssi);
    Serial.println("dBm");
    Serial.println("WOW, PERFEKT!");
  }
}

void loop() {
  {
    if (millis() - bot_lasttime > BOT_MTBS)
    {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      while (numNewMessages)
      {
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      bot_lasttime = millis();
    }
  }
  httpServer.handleClient();
  ArduinoOTA.handle(); // Auskommentieren!
  //WifiDeleter();
}

// Version 2.86 - 05.01.2022 / 23:30
// All-in-One Version!


/*
   Copyright (c) 2022 do8pgg

   This Code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This Code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this Code; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
