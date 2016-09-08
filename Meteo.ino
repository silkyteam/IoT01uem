//Wifi librerias
#include <SPI.h>
#include <WiFi.h>

//LCD librerias
#include <Wire.h>
#include "rgb_lcd.h"

#define thing_name "" //definimos nombre de nuestro internet de las cosas para servicio dweet.io
#define MAX 25 // Variable valor maximo de intensidad de luz noche
#define led 13      // numero de pin para el LED

String LAT = ""; // Posicion GPS del IoT
String LON = "";
String ALUMBRADO; // variable para alumbrado del campus
String CINE = ""; // url de descarga del apk de InteractiveCinema
String VIDEOS = ""; // url de descarga de los videos 360
String CARTEL = ""; // url del cartel de RV

//Wifi parametros de inicio
WiFiClient client;
char ssid[] = "";     // nombre del SSID
char pass[] = "";     // password de la conexion

char server[] = "dweet.io";     // direcciones de servers usando DNS
char server2[] = "data.sparkfun.com";
char GET1[] = "GET /dweet/for/*url del servicio dweet.io*"; // Comienzo cadenas GET con o sin key
char GET2[] = "GET /input/*url con claves del servicio sparkfun*";

int status = WL_IDLE_STATUS; // inicializamos varaible para el estado de la wifi a reposo

//LCD parametros de inicio
rgb_lcd lcd;
int colorR = 255;
int colorG = 0;
int colorB = 0;


// Definie los pines de los sensores
const int pinTemp = A0; // sensor temperatura
const int pinLight = A1; // sensor de luz
const int pinSound = A2; // sensor de sonido

// Este valor es una propiedad del sensor de temp. utilizado en el Grove,
// y se utiliza para convertir del valor analógico que mide a un valor de temperatura.
const int B = 3975;


void setup()
{
	Serial.begin(115200); // iniciar puerto serie de salida
	pinMode(led, OUTPUT); // modo output para los leds
  pinMode(pinSound, INPUT); // modo input para los sensores
	pinMode(pinTemp, INPUT);
  pinMode(pinLight, INPUT);
	
  /* CONEXION A LA WIFI */
  while (status != WL_CONNECTED)//mientras no este conectado por wifi
	  { 
    Serial.print("Tratando de conectar con SSID: ");
    digitalWrite(led, HIGH); // encendemos led para monitorizar
    Serial.println(ssid);      
    status = WiFi.begin(ssid, pass); // lanzamos la petición de conexion con los parametros
    delay(5000); // esperamos 5 segundos para dar tiempo a establecer conexión
	}
  
  Serial.println("Conectado por wifi con IP: "); // Informamos de los datos de la conexión
  Serial.println(WiFi.localIP());
  digitalWrite(led, LOW); // apagamos el led de monitorización
  
  /* CONEXION AL SERVER dweet.io */
  Serial.println("Tratando de conectar con server...");
  Serial.println(server);
  digitalWrite(led, HIGH); // encender led de monitorización
  
   if (client.connect(server, 80)) // si conectamos con el server
	   {
     Serial.println("CONECTADO!!!");
     digitalWrite(led, LOW);	// apagamos el led de monitorización
		} else {Serial.println("ERROR DE CONEXION!!!");delay(5000);} // mensaje si no hemos podido conectar
   
  // establecer el número de columnas y filas en la pantalla LCD  
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB); 
}

void loop()
{
	int light = analogRead(pinLight); // Leer el valor del sensor de luz, es un sensor analógico.
  delay(100);
	int val = analogRead(pinTemp); // Obtener el valor (en bruto ) del sensor de temperatura, es analógico.
  delay(100);
  int sound = analogRead(pinSound); // leer valor del sensor de sonido, sensor analogico.
  delay(500);
 
 
	float resistance = (float)(1023 - val) * 10000 / val; //Determinar la resistencia actual del termistor basado en el valor del sensor
	float temperature = 1 / (log(resistance / 10000) / B + 1 / 298.15) - 273.15; //Calcular la temperatura basado en el valor de resistencia
 
  /* LCD */
  lcd.clear();
  lcd.print("Temp. Luz  Ruido");
  lcd.setCursor(0, 1); // el cursor a columna 0, linea 1, (nota: linea 1 es la segunda).
  lcd.print(temperature);
  lcd.print("  ");
  lcd.print(light);
  lcd.print("  ");
  lcd.print(sound);

  /* SALIDA SERIAL */
  Serial.print("TEMPERATURA");  
  Serial.print("\t");      
  Serial.println(temperature);
  Serial.print("LUZ"); 
  Serial.print("\t");
  Serial.print("\t");    
  Serial.println(light);
  Serial.print("RUIDO"); 
  Serial.print("\t");
  Serial.print("\t");    
  Serial.println(sound);

  /* CONDICION PARA ENCENDER LUCES DE NOCHE */
  if (light < MAX)
	{
    digitalWrite(led, HIGH);
    ALUMBRADO = "ENCENDIDO";
    }
  else{
   digitalWrite(led, LOW);
   ALUMBRADO = "";
	}
  delay(500);
  
  /* ENVIAR PETICION HTTP GET A LOS SERVIDORES CON LOS DATOS */

int t = temperature; // convertimos a enteros para el HTTP GET
int l = light;
int s = sound;

Serial.print(F("Enviando datos a... ")); Serial.println(server);

client.stop(); // Importante!!! cerrar antes cualquier posible conexión abierta
client.connect(server, 80); // conectamos con server por puerto 80
delay(500); // damos tiempo

if (client.connected()) { // protegemos la petición por si no hubiese conexión
	// la petición HTTP GET propiamente.
client.print(String(GET1) + "?Temperatura=" + String (t) + "&Luz=" + String(l) + "&Ruido=" + String(s) + "&Latitud=" + String(LAT) + "&Longitud=" + String(LON)
+ "&Alumbrado=" + String(ALUMBRADO) + "&CINEMA=" + String(CINE) + "&VIDEOS360=" + String(VIDEOS) + "&Cartel=" + String(CARTEL) + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: close\r\n\r\n" + "AT+CIPCLOSE\r\n");
delay(500);
Serial.println(F("OK.")); // todo bien

Serial.print(F("Enviando datos a... ")); Serial.println(server2);

client.stop(); // Importante!!! cerrar antes cualquier posible conexión abierta
client.connect(server2, 80); // conectamos con server2 por puerto 80
delay(500); // damos tiempo

if (client.connected()) { // protegemos la petición por si no hubiese conexión
  // la petición HTTP GET propiamente.
client.print(String(GET2) + "&Luz=" + String (l) + "&Ruido=" + String(s) + "&Temperatura=" + String(t) 
+ " HTTP/1.1\r\n" + "Host: " + server2 + "\r\n" + "Connection: close\r\n\r\n" + "AT+CIPCLOSE\r\n");
delay(500);
Serial.println(F("OK.")); // todo bien

} else { 
	Serial.println(F("Conexión fallida."));  // no se pudo realizar la conexión, pero todo sigue 
		} 
}
}