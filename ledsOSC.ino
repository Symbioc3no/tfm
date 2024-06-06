// Importamos Librerías necesarias para contro ArtNet
#include <Art-Net.h>
#include <Art-NetOemCodes.h>
#include <ArtDmx.h>
#include <ArtNode.h>
#include <ArtPollReply.h>
#include <RDM.h>
#include <WiFiS3.h> //Incluímos librería para conexión Wifi

#include <WiFi.h>
#include <SPI.h>
#include <ArtNode.h>
#include <FastLED.h>

#define VERSION_HI 0
#define VERSION_LO 1

const char* ssid = "Innovacion";        //"Innovation";// Router
const char* password = "Innovacion24";  //"btmcbK4N";//Contraseña

////////////////////////////////////////////////////////////
ArtConfig config = {
  .mac = { 0x00, 0xAD, 0xBE, 0xEF, 0xFE, 0xED },  // MAC > ESTO TIENE QUE CAMBIAR ENTRE COMPUS!!!!
  .ip = { 2, 3, 4, 5 },                           // IP
  .mask = { 255, 0, 0, 0 },                       // Subnet mask
  .udpPort = 0x1936,
  .dhcp = false,
  .net = 0,              // Net (0-127)
  .subnet = 0,           // Subnet (0-15)
  "Arduino",             // Short name
  "Arduino controller",  // Long name
  .numPorts = 4,
  .portTypes = {
    PortTypeDmx | PortTypeOutput,
    PortTypeDmx | PortTypeOutput,
    PortTypeDmx | PortTypeOutput,
    PortTypeDmx | PortTypeOutput },
  .portAddrIn = { 0, 1, 2, 3 },   // Port input universes (0-15)
  .portAddrOut = { 0, 1, 2, 3 },  // Port output universes (0-15)
  .verHi = VERSION_HI,
  .verLo = VERSION_LO
};
////////////////////////////////////////////////////////////

byte buffer[600];
ArtNode node;
WiFiUDP udp;

//AQUI AUMENTAMOS LAS DOS TIRAS LED EXTRA
#define NUM_LEDS_1 37

//AQUI DECLARAMOS LAS SALIDAS EN LA ARDUINO
#define DATA_PIN_1 9

//AUMENTAMOS DOS ARRAY PARA LOS OTROS LEDS
CRGB leds_1[NUM_LEDS_1];

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Información importante para conectar con otros dispositivos

  //Parte del código que decidimos deshabilitar para mejorar el funcionamiento del ArtNet

  // //actualizar MAC y IP según conexión
  // IPAddress address = WiFi.localIP();
  // config.ip[0] = address[0];
  // config.ip[1] = address[1];
  // config.ip[2] = address[2];
  // config.ip[3] = address[3];
  // uint64_t uid = ESP.getEfuseMac();
  // uint64_t number = 0;
  // number = (uid & 0x00FF0000000000)>>40;
  // config.mac[0] = (uint8_t) number;
  // number = (uid & 0x0000FF00000000)>>32;
  // config.mac[1] = (uint8_t) number;
  // number = (uid & 0x000000FF000000)>>24;
  // config.mac[2] = (uint8_t) number;
  // number = (uid & 0x00000000FF0000)>>16;
  // config.mac[3] = (uint8_t) number;
  // number = (uid & 0x0000000000FF00)>>8;
  // config.mac[4] = (uint8_t) number;
  // number = (uid & 0x000000000000FF);
  // config.mac[5] = (uint8_t) number;

  config.mask[0] = 255;
  config.mask[1] = 255;
  config.mask[2] = 255;
  config.mask[3] = 0;

  //inicialitzem node Artnet amb la configuració que toca
  node = ArtNode(config, sizeof(buffer), buffer);
  //inicialitzem socket UDP
  udp.begin(config.udpPort);

  //IGUALMENTE DUPLICAMOS ESTA CONEXIION PARA USAR LAS OTRAS TIRAS LEDS
  FastLED.addLeds<NEOPIXEL, DATA_PIN_1>(leds_1, NUM_LEDS_1);

  //No ens volem deixar els ulls fent proves
  FastLED.setBrightness(255);

  for (int i = 0; i < NUM_LEDS_1; i++) {
    // Iniciamos todos los leds en negro
    leds_1[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

int value = 0;

void loop() {
  while (udp.parsePacket()) {
    int n = udp.read(buffer, min((unsigned int)udp.available(), sizeof(buffer)));
    if (n >= sizeof(ArtHeader) && node.isPacketValid()) {

      // Package Op-Code determines type of packet
      switch (node.getOpCode()) {

        // Poll packet. Send poll reply.
        case OpPoll:
          {
            ArtPoll* poll = (ArtPoll*)buffer;
            node.createPollReply();
            udp.beginPacket(node.broadcastIP(), config.udpPort);
            udp.write(buffer, sizeof(ArtPollReply));
            udp.endPacket();
          }
          break;

        // DMX packet
        case OpDmx:
          {
            ArtDmx* dmx = (ArtDmx*)buffer;
            int port = node.getPort(dmx->Net, dmx->SubUni);
            int len = dmx->getLength();
            byte* data = dmx->Data;
            if (port == 0) {
              int k = 0;

              for (int i = 0; i < 111; i = i + 3) { //Modificamos el tope del loop para los leds que tenemos
                if (k < NUM_LEDS_1)
                  leds_1[k] = CRGB(data[i], data[i + 1], data[i + 2]);
                k++;
              }
              FastLED.show();
            }
          }
          break;

        default:
          break;
      }
    }
  }
}
