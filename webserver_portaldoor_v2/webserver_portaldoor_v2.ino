/* intentado probar con autentificación, peron n ome dejaba instalar librerías:
 *  AsyncHTTPRequest_Generic
Quería instalar las siguientes:
ESPAsync_WIFIManager
ESP_DoubleResetDetector
LittleFS_esp32
STP32duino LwIP
STM32duino STM32Ethernet

Era este: https://randomnerdtutorials.com/esp32-esp8266-web-server-http-authentication/

Siguiente prueba:
https://github.com/luisllamasbinaburo/ESP32-Examples/blob/main/05_Server_Simple/

Instrucciones para instalar AsyncWebServer https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/
aunque basicamente:
  cd /Users/ivan/Documents/Arduino/libraries
  git clone https://github.com/me-no-dev/ESPAsyncWebServer
  git clone https://github.com/me-no-dev/AsyncTCP

Version 1.1: añadidao watchdog, comprobacion de status, resetear placa si en 60 segundos no se conecta

Version 2: añadido OTA para actualizaciones sencillas (segun https://lastminuteengineers.com/esp32-ota-updates-arduino-ide/)

Si al abrir arduino no se puede compilar y dice que no encuentra python en mac monterrey o posterior, abrir arduino con
  open /Applications/Arduino.app

*/
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>


// opciones configurables

// nombre, ssid
#define ssid "UBAWIFI2"
const char* password = "E1ldlmdcnnqa2";
#define hostname "ESP32_PortalDoor"

// cuanto tiempo abre la puerta
#define RELAYTIME 4000
#define RELAYTIMETXT "4000"

// en que pin esta puesto el rele
#define RELAY 27  // pin 27, relay for the light
#define RELAYTXT "27"

// si la siguiente variable tiene algo en lugar de pedir /open y /status,
// habrá solicitar /open_password y /status_password
#define PASSWORD ""
//#define PASSWORD "_password"

#define WATCHDOGHOURS 34


// usar direccion statica o dhcp
bool useStaticIP = false;

// direccion de dhcp
IPAddress ip(192, 168, 4, 204);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);


// fin configuración


// algunas variables
// timer para saber a que hora se ha abierto puerta (y estar ¿4? segundos abierta)
unsigned long timercount=0;

// variable con ultima peticion
unsigned long timelastaccess=0;


#define LED 2


#define SwitchOnRelay() digitalWrite(RELAY,HIGH)
#define SwitchOffRelay() digitalWrite(RELAY,LOW)

AsyncWebServer server(80);

// Funcion que se ejecutara en la URI '/'
void handleRoot(AsyncWebServerRequest *request) 
{
   request->send(200, "text/html", "<html>"
     "<head><title>Portal Door</title></head>"
     "<body><h2>Portal Door</h2>"
     "<p>hostname: " hostname "</p>"
     "<p>GPIO: " RELAYTXT "</p>"
     "<p>RelayTime: " RELAYTIMETXT "</p>"
     "<p>ssid: " ssid "</p>"
     "<p>ws: /openXXX y /statusXXX</p></body>"
     "</html>");
}

// Funcion que se ejecutara en la URI '/status'
void handleStatus(AsyncWebServerRequest *request) 
{
      AsyncWebServerResponse *response;

      timelastaccess=millis();  // actualizar hora ultimo acceso
      if(timercount==0)
        response = request->beginResponse(200, "text/json", "{ \"result\":\"ok\",\"resultcode\":\"0\",\"alive\":\"1\",\"opening\":\"0\" }");
      else
        response = request->beginResponse(200, "text/json", "{ \"result\":\"ok\",\"resultcode\":\"0\",\"alive\":\"1\",\"opening\":\"1\" }");
        
      response->addHeader("Access-Control-Allow-Origin","*");
      request->send(response);
}


// Funcion que se ejecutara en la URI '/open'
void handleOpen(AsyncWebServerRequest *request) 
{
      timelastaccess=millis();  // actualizar hora ultimo acceso
      AsyncWebServerResponse *response = request->beginResponse(200, "text/json", "{ \"result\":\"ok\",\"resultcode\":\"0\",\"relaytime\":\"" RELAYTIMETXT "\" }");
      response->addHeader("Access-Control-Allow-Origin","*");
      request->send(response);
      SwitchOnRelay();
      timercount=millis();
}

// Funcion que se ejecutara en URI desconocida
void handleNotFound(AsyncWebServerRequest *request) 
{
   request->send(404, "text/plain", "404 Not found");
}

void InitServer()
{
   // Ruteo para '/'
   server.on("/", [](AsyncWebServerRequest *request) { handleRoot(request); } );
 
   // Ruteo para '/status'
   server.on("/status" PASSWORD, [](AsyncWebServerRequest *request) {
     handleStatus(request);
   });

   // Ruteo para '/open'
   server.on("/open" PASSWORD, [](AsyncWebServerRequest *request) {
     handleOpen(request);
   });
 
   // Ruteo para URI desconocida
   server.onNotFound([](AsyncWebServerRequest *request){
     request->send(404);
   });
    
   // Iniciar servidor
   server.begin();
   Serial.println("HTTP server started");
}



/* rutinas gestión WIFI */
// modo cliente
void ConnectWiFi_STA(bool useStaticIP = false)
{
   Serial.println("Connecting to wifi network");
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   if(useStaticIP)
     WiFi.config(ip, gateway, subnet);

   timelastaccess=millis();

   while (WiFi.status() != WL_CONNECTED) 
   { 
     if (millis() - timelastaccess >= 60000) {
       Serial.println("");
       Serial.println("Could not connect to wifi network, reseting board");
       esp_restart();
     }
     delay(500);  
     Serial.print('.'); 
   }
 
   Serial.println("");
   Serial.print("Iniciado STA:\t");
   Serial.println(ssid);
   Serial.print("IP address:\t");
   Serial.println(WiFi.localIP());
   timelastaccess=millis();
}

// modo AP
void ConnectWiFi_AP(bool useStaticIP = false)
{ 
   Serial.println("Connecting to wifi network");
   WiFi.mode(WIFI_AP);
   while(!WiFi.softAP(ssid, password))
   {
     Serial.println(".");
     delay(500);
   }
   if(useStaticIP)
     WiFi.softAPConfig(ip, gateway, subnet);

   Serial.println("");
   Serial.print("Iniciado AP:\t");
   Serial.println(ssid);
   Serial.print("IP address:\t");
   Serial.println(WiFi.softAPIP());
}


void InitOTA(void)
{
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("portaldoor");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}


void setup(void) 
{
  // set relay to switchoff
  pinMode(RELAY, OUTPUT);
  SwitchOffRelay(); 

  // set led output
  pinMode(LED, OUTPUT);

  // puerto serie para log
  Serial.begin(115200);

  // configurar wifi
  ConnectWiFi_STA(useStaticIP);

  // configurar servidor web
  InitServer();

  // handle OTA update
  InitOTA();
}

unsigned long inittime=0;  // timer para led

void BlinkLed(int status)
{
  unsigned int t;

  char ledtables[4][8]={
//    {1,0,0,0,0,0,0,0},  // 0=ok (brilla lento)
    {0,0,0,0,0,0,0,0},  // 0=ok (todo apagado)
    {1,0,1,0,1,0,1,0},  // 1=error
    {1,1,1,1,1,1,1,1},  // 2=todo el rato encendido
    {1,1,1,0,1,0,0,0},  // 3=1 largo y corto
  };
  
  t=(millis()-inittime)>>8;
  if(t>8) {
    inittime=millis();
    t=0;
  }
  if(ledtables[status&3][t]!=0)
    digitalWrite(LED,HIGH);  // encender led
  else
    digitalWrite(LED,LOW);   // apagar led
}


void HandleLed()
{
     // vamos a poner led
   if(timercount)
     BlinkLed(2);    // pulsador pulsado, luz encendida
   else if(WiFi.status() != WL_CONNECTED)
     BlinkLed(1);    // sin conexion, led rapido
   else
     BlinkLed(0);    // con conexion, o led lento o apagada luz

}
void loop()
{
   delay(1);  // dar tiempo a multitarea, y creo
   // que hace que servidor web vaya mucho mas rapido
   // https://github.com/espressif/arduino-esp32/issues/4348#issuecomment-695115885

   HandleLed();

   // manejar el OTA
   ArduinoOTA.handle();
   
   // atender servidor web
//   server.handleClient();  // AsyncWebServer no necesita esto

   // dejar de abrir la puerta 4s despues ultima pulsacion
   if(timercount!=0) {
     if(millis()-timercount>RELAYTIME) {
       timercount=0;
       SwitchOffRelay();
     }
   }

   // si se desconecta de la wifi resetear placa
   if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Wifi Connection lost, reseting board");
     esp_restart();
   }

   // si no hemos recibido peticiones en muchas horas reiniciar por si acaso
   if( millis()-timelastaccess > WATCHDOGHOURS*3600000) {  // si en 34 horas no recibido nada, resetear
     Serial.println("not received anything in some hours, reseting board");
     esp_restart();
   }
  
}
