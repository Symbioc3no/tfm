#include <Servo.h> //Incluímos librería para controla servomotores Wifi
#include "WiFiS3.h" //Incluímos librería para conexión Wifi
#include <OSCMessage.h> // Incluímos librería para manejo de mensajes OSC
WiFiUDP Udp;

int update_rate = 16; // Valor por seguridad del proceso OSC

int funcion = 0; //Variable con la que elegimos el movimiento de los motores

// Network settings
char ssid[] = "Innovacion"; //Red Wifi
char pass[] = "Innovacion24"; // Clave de red Wifi
unsigned int localPort = 11111; // Puerto por el que escucharemos

IPAddress outIp(192, 168, 0, 119);  // Ip al que enviamos mensajes OSC
const unsigned int outPort = 8888;  // Puerto al que enviamos los mensajes OSC

int motorPin1H = 2; //Pin digital 1 motor 1
int motorPin1L = 3; //Pin digital 2 motor 1
int motorPin2H = 4; //Pin digital 1 motor 2
int motorPin2L = 5; //Pin digital 2 motor 2
int motorPin3H = 6; //Pin digital 1 motor 3
int motorPin3L = 7; //Pin digital 2 motor 3
int motorPin4H = 8; //Pin digital 1 motor 4
int motorPin4L = 9; //Pin digital 2 motor 4

//Variables para controlar el tiempo acción de los motores

int diley = 1500;

boolean inicio = true; // Variable que usamos para activar movimiento inicial de plataforma solo cuando empieza un nuevo giro
boolean vuelta = false; // Variable que podriamos necesitar en caso de querer hacer cambios

void setup() {

//establecemos las salidas digitales
  pinMode(motorPin1H, OUTPUT);
  pinMode(motorPin1L, OUTPUT);
  pinMode(motorPin2H, OUTPUT);
  pinMode(motorPin2L, OUTPUT);
  pinMode(motorPin3H, OUTPUT);
  pinMode(motorPin3L, OUTPUT);
  pinMode(motorPin4H, OUTPUT);
  pinMode(motorPin4L, OUTPUT);

// Normalizamos los motores a su punto de inicio
  digitalWrite(motorPin1H, LOW);
  digitalWrite(motorPin1L, HIGH);
  digitalWrite(motorPin2H, LOW);
  digitalWrite(motorPin2L, HIGH);
  digitalWrite(motorPin3H, LOW);
  digitalWrite(motorPin3L, HIGH);
  digitalWrite(motorPin4H, LOW);
  digitalWrite(motorPin4L, HIGH);

  Serial.begin(115200);

  WiFi.begin(ssid, pass); // establecemos conexión wifi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Retry");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Información importante para conectar con otros dispositivos

  Udp.begin(localPort);
}

void loop() {

  receiveMessage(); //Empezamos escuchando cualquier comando OSC que afecte al comportamiento de la arduino


  if (funcion == 1) {
    //Funcion para subir plataforma
    digitalWrite(motorPin1H, HIGH);
    digitalWrite(motorPin1L, LOW);
    digitalWrite(motorPin2H, HIGH);
    digitalWrite(motorPin2L, LOW);
    digitalWrite(motorPin3H, HIGH);
    digitalWrite(motorPin3L, LOW);
    digitalWrite(motorPin4H, HIGH);
    digitalWrite(motorPin4L, LOW);

    delay(diley);
    funcion = 0;
  }
  if (funcion == 2) {

    //Funcion para girar plataforma
    if (inicio) { //Si no ha estado en movimiento inicio debería ser true y necesitamos hacer este movimiento previo

      digitalWrite(motorPin1H, HIGH);  //Movimiento Inicial
      digitalWrite(motorPin1L, LOW);
      delay(diley);

      digitalWrite(motorPin1H, HIGH);
      digitalWrite(motorPin1L, LOW);
      digitalWrite(motorPin2H, HIGH);
      digitalWrite(motorPin2L, LOW);
      digitalWrite(motorPin4H, HIGH);
      digitalWrite(motorPin4L, LOW);
      delay(diley);

      inicio = false;
    }

    //Ahora si giro normal
    digitalWrite(motorPin1H, LOW);  // Movimiento conjunto 1
    digitalWrite(motorPin1L, HIGH);
    digitalWrite(motorPin2H, HIGH);
    digitalWrite(motorPin2L, LOW);
    digitalWrite(motorPin3H, HIGH);
    digitalWrite(motorPin3L, LOW);
    digitalWrite(motorPin4H, LOW);
    digitalWrite(motorPin4L, HIGH);

    delay(diley);

    digitalWrite(motorPin1H, LOW);  // Movimiento conjunto 2
    digitalWrite(motorPin1L, HIGH);
    digitalWrite(motorPin2H, LOW);
    digitalWrite(motorPin2L, HIGH);
    digitalWrite(motorPin3H, HIGH);
    digitalWrite(motorPin3L, LOW);
    digitalWrite(motorPin4H, HIGH);
    digitalWrite(motorPin4L, LOW);

    delay(diley);

    digitalWrite(motorPin1H, HIGH);  // Movimiento conjunto 3
    digitalWrite(motorPin1L, LOW);
    digitalWrite(motorPin2H, LOW);
    digitalWrite(motorPin2L, HIGH);
    digitalWrite(motorPin3H, LOW);
    digitalWrite(motorPin3L, HIGH);
    digitalWrite(motorPin4H, HIGH);
    digitalWrite(motorPin4L, LOW);

    delay(diley);

    digitalWrite(motorPin1H, HIGH);  // Movimiento conjunto 4
    digitalWrite(motorPin1L, LOW);
    digitalWrite(motorPin2H, HIGH);
    digitalWrite(motorPin2L, LOW);
    digitalWrite(motorPin3H, LOW);
    digitalWrite(motorPin3L, HIGH);
    digitalWrite(motorPin4H, LOW);
    digitalWrite(motorPin4L, HIGH);

    delay(diley);
  }
  if (funcion == 3) {
    //Función para parar el giro
    digitalWrite(motorPin1H, LOW);  //5 SlowDown
    digitalWrite(motorPin1L, HIGH);
    digitalWrite(motorPin2H, LOW);
    digitalWrite(motorPin2L, HIGH);
    digitalWrite(motorPin3H, LOW);
    digitalWrite(motorPin3L, HIGH);
    digitalWrite(motorPin4H, LOW);
    digitalWrite(motorPin4L, HIGH);

    delay(diley);

    inicio = true; //regresamos esta variable a true en caso de volver a ejecutar el giro
    funcion = 0;
  }
  if (funcion == 4) {

    //Funcion para regresar los actuadores a 0
    digitalWrite(motorPin1H, LOW);
    digitalWrite(motorPin1L, HIGH);
    digitalWrite(motorPin2H, LOW);
    digitalWrite(motorPin2L, HIGH);
    digitalWrite(motorPin3H, LOW);
    digitalWrite(motorPin3L, HIGH);
    digitalWrite(motorPin4H, LOW);
    digitalWrite(motorPin4L, HIGH);

    delay(diley);

    inicio = true;
    funcion = 0;
  }
  if (funcion == 5) {

  //Función para mover primer motor
    digitalWrite(motorPin1H, HIGH);
    digitalWrite(motorPin1L, LOW);

    delay((diley*3)/2);

    digitalWrite(motorPin1H, LOW);
    digitalWrite(motorPin1L, HIGH);

    delay((diley*3)/2);
    funcion = 0;
  }
  if (funcion == 6) {

    //Función para mover segundo motor
    digitalWrite(motorPin2H, HIGH);
    digitalWrite(motorPin2L, LOW);

    delay((diley*3)/2);

    digitalWrite(motorPin2H, LOW);
    digitalWrite(motorPin2L, HIGH);

    delay((diley*3)/2);
    funcion = 0;
  }
  if (funcion == 7) {

    //Función para mover tercer motor
    digitalWrite(motorPin3H, HIGH);
    digitalWrite(motorPin3L, LOW);

    delay((diley*3)/2);

    digitalWrite(motorPin3H, LOW);
    digitalWrite(motorPin3L, HIGH);

    delay((diley*3)/2);
    funcion = 0;
  }
  if (funcion == 8) {

    //Función para mover cuarto motor
    digitalWrite(motorPin4H, HIGH);
    digitalWrite(motorPin4L, LOW);

    delay((diley*3)/2);

    digitalWrite(motorPin4H, LOW);
    digitalWrite(motorPin4L, HIGH);

    delay((diley*3)/2);
    funcion = 0;
  }
}

void receiveMessage() { //Función para leer mensajes OSC
  OSCMessage inmsg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      inmsg.fill(Udp.read());
    }
    //repeat for each address
    if (!inmsg.hasError()) {
      //Comprobamos que el mensaje osc tenga el comando que esperamos para realizar la función establecida
      inmsg.dispatch("/On", callbackFunctionMessage);
    }
  }
}

//Función ejecutar movimiento de motores según OSC
void callbackFunctionMessage(OSCMessage& msg) {
  funcion = round(msg.getFloat(0));
  Serial.println("MESSAGE RECEIVED");
  Serial.println(funcion);
}