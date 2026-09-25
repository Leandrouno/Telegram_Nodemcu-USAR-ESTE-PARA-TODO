#include <ESP8266WiFi.h>
#include "CTBot.h"

#define LED D1
#define PUERTA2 D2
#define PUERTA1 D5
#define PORTON D7

#define PUERTA1_CALLBACK "Puerta1"
#define PUERTA2_CALLBACK "Puerta2"
#define PORTON_CALLBACK "Porton"
#define ESTADO_CALLBACK "Estado"

CTBot myBot;
CTBotReplyKeyboard myKbd;

String ssid = "TU_WIFI";
String pass = "TU_PASSWORD";
String token = "TOKEN_TELEGRAM";
long id = 123456789; // ID Telegram sin comillas


void accionarRelay(uint8_t pin) {
  digitalWrite(pin, LOW);
  delay(500);
  digitalWrite(pin, HIGH);
}

String tiempoEncendido() {
  unsigned long totalSegundos = millis() / 1000;

  unsigned long dias = totalSegundos / 86400;
  unsigned long horas = (totalSegundos % 86400) / 3600;
  unsigned long minutos = (totalSegundos % 3600) / 60;

  String tiempo = "";

  if (dias > 0) {
    tiempo += String(dias) + " dias ";
  }

  if (horas > 0 || dias > 0) {
    tiempo += String(horas) + " hs ";
  }

  tiempo += String(minutos) + " min";

  return tiempo;
}

String calidadWifi() {
  int rssi = WiFi.RSSI();

  if (rssi >= -50) return "Excelente";
  if (rssi >= -60) return "Muy buena";
  if (rssi >= -70) return "Buena";
  if (rssi >= -80) return "Regular";

  return "Debil";
}

void enviarEstado(long chatId) {
  String estado = "ESTADO DEL SISTEMA\n";
  estado += "-------------------------\n";

  if (WiFi.status() == WL_CONNECTED) {
    estado += "WiFi: Conectado\n";
    estado += "Red: " + WiFi.SSID() + "\n";
    estado += "Senal: " + String(WiFi.RSSI()) + " dBm\n";
    estado += "Calidad: " + calidadWifi() + "\n";
    estado += "IP: " + WiFi.localIP().toString() + "\n";
  } else {
    estado += "WiFi: Desconectado\n";
  }

  estado += "Encendido: " + tiempoEncendido() + "\n";
  estado += "Telegram: Online\n";
  estado += "-------------------------\n";
  estado += "Very Good MONOO !";

  myBot.sendMessage(chatId, estado);
}

void mostrarTeclado(long chatId) {
  myBot.sendMessage(chatId, "CONTROL DE CASA", myKbd);
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Iniciando TelegramBot...");

  pinMode(LED, OUTPUT);
  pinMode(PUERTA1, OUTPUT);
  pinMode(PUERTA2, OUTPUT);
  pinMode(PORTON, OUTPUT);

  // Relays apagados
  digitalWrite(PUERTA1, HIGH);
  digitalWrite(PUERTA2, HIGH);
  digitalWrite(PORTON, HIGH);
  digitalWrite(LED, LOW);

  Serial.println("Conectando a WiFi...");

  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);

  if (myBot.testConnection()) {
    Serial.println("Conectado a la Red WiFi");
    Serial.println("Telegram conectado");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Senal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    digitalWrite(LED, HIGH);
  } else {
    Serial.println("Error al conectar");
    digitalWrite(LED, LOW);
  }


  // ============================================
  // TECLADO TELEGRAM
  // ============================================

  myKbd.addButton("Puerta Negra");
  myKbd.addButton("Puerta Blanca");

  myKbd.addRow();

  myKbd.addButton("Porton");

  myKbd.addRow();

  myKbd.addButton("Estado");

  myKbd.addRow();

  myKbd.addButton("EstadoPorton");
  myKbd.addButton("EstadoTimbre");

  myKbd.enableResize();

  Serial.println("Sistema listo.");
}

void loop() {

  TBMessage msg;

  if (myBot.getNewMessage(msg)) {

    // ============================================
    // USUARIO AUTORIZADO
    // ============================================

    if (msg.sender.id != id) {
      Serial.print("Acceso no autorizado: ");
      Serial.println(msg.sender.id);

      myBot.sendMessage(msg.sender.id, "JUIRAAAA !!");

      delay(100);
      return;
    }


    // ============================================
    // MENSAJES DE TEXTO
    // ============================================

    if (msg.messageType == CTBotMessageText) {

      Serial.print("Mensaje recibido: ");
      Serial.println(msg.text);


      // ------------------------------------------
      // MOSTRAR TECLADO
      // ------------------------------------------

      if (
        msg.text.equalsIgnoreCase("Teclado") ||
        msg.text.equalsIgnoreCase("Menu") ||
        msg.text.equalsIgnoreCase("/start")
      ) {
        mostrarTeclado(msg.sender.id);
      }


      // ------------------------------------------
      // PUERTA NEGRA
      // ------------------------------------------

      else if (
        msg.text.equalsIgnoreCase("Puerta Negra") ||
        msg.text.equalsIgnoreCase("Puerta1")
      ) {
        Serial.println("Abriendo Puerta Negra");

        accionarRelay(PUERTA1);

        myBot.sendMessage(
          msg.sender.id,
          "Puerta Negra Abierta"
        );
      }


      // ------------------------------------------
      // PUERTA BLANCA
      // ------------------------------------------

      else if (
        msg.text.equalsIgnoreCase("Puerta Blanca") ||
        msg.text.equalsIgnoreCase("Puerta2")
      ) {
        Serial.println("Abriendo Puerta Blanca");

        accionarRelay(PUERTA2);

        myBot.sendMessage(
          msg.sender.id,
          "Puerta Blanca Abierta"
        );
      }


      // ------------------------------------------
      // PORTON
      // ------------------------------------------

      else if (msg.text.equalsIgnoreCase("Porton")) {
        Serial.println("Accionando Porton");

        accionarRelay(PORTON);

        myBot.sendMessage(
          msg.sender.id,
          "Porton Accionado"
        );
      }


      // ------------------------------------------
      // ESTADO DE ESTE ESP
      // ------------------------------------------

      else if (msg.text.equalsIgnoreCase("Estado")) {
        Serial.println("Consultando estado");

        enviarEstado(msg.sender.id);
      }


      // ============================================
      // IMPORTANTE
      // ============================================
      //
      // "Estado Porton" y "Estado Timbre"
      // NO se procesan en este ESP.
      //
      // Solamente aparecen en el teclado.
      //
      // Los otros ESP8266 reciben esos mensajes
      // de Telegram y responden ellos.
      //
      // ============================================
    }


    // ============================================
    // CALLBACKS
    // Se mantienen por compatibilidad
    // ============================================

    else if (msg.messageType == CTBotMessageQuery) {

      if (msg.callbackQueryData.equals(PUERTA1_CALLBACK)) {

        accionarRelay(PUERTA1);

        myBot.endQuery(
          msg.callbackQueryID,
          "Puerta Negra Abierta",
          true
        );
      }

      else if (msg.callbackQueryData.equals(PUERTA2_CALLBACK)) {

        accionarRelay(PUERTA2);

        myBot.endQuery(
          msg.callbackQueryID,
          "Puerta Blanca Abierta",
          true
        );
      }

      else if (msg.callbackQueryData.equals(PORTON_CALLBACK)) {

        accionarRelay(PORTON);

        myBot.endQuery(
          msg.callbackQueryID,
          "Porton Accionado",
          true
        );
      }

      else if (msg.callbackQueryData.equals(ESTADO_CALLBACK)) {

        myBot.endQuery(
          msg.callbackQueryID,
          "Consultando...",
          false
        );

        enviarEstado(msg.sender.id);
      }
    }
  }

  delay(100);
}
