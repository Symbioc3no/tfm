#include "WiFiS3.h"      //Incluímos librería para conexión Wifi
#include <OSCMessage.h>  // Incluímos librería para manejo de mensajes OSC
WiFiUDP Udp;

int update_rate = 16;  // Valor por seguridad del proceso OSC

// Network settings
char ssid[] = "Innovacion";      //Red Wifi
char pass[] = "Innovacion24";    // Clave de red Wifi
unsigned int localPort = 11111;  // Puerto por el que escucharemos

IPAddress outIp(192, 168, 0, 119);  //  Ip al que enviamos mensajes OSC
const unsigned int outPort = 8888;  // Puerto al que enviamos los mensajes OSC

int modalidad;  //Variable para elegir entre Biofeedback o válvulas

//Empezamos definiendo variables para la modalidad de Biofeedback

const byte interruptPin = 3;           //galvanometer o Biofeedback input
volatile unsigned long startTime = 0;  //variable donde se registra cuando recibimos la señal del biofeedback
volatile unsigned long endTime = 0;    // variable que registra cuando termina esta señal
volatile bool pulseStarted = false;    // variable que avisa cuando recibimos señal del biofeedback

int lowpass = 15000;  // Valor máximo del que registraremos la señal del biofeedback
int highpass = 0;     // Valor máximo del que registraremos la señal del biofeedback
int noteMin = 0;      // Valor mínimo para conversión a nota MIDI
int noteMax = 127;    // Valor máximo para conversión a nota MIDI

int frecuencia;  //aquí se registra la duración del pulso obtenido del biofeedback
float notaMidi;  // aquí se registra el equivalente de frecuencia en nota MIDI

int cantidadCeldas = 50;  // tamaño inicial del array donde registraremos los pulsos del biofeedback

unsigned long* pulsoDuraciones;  // array de pulsos que podrá cambiar su tamaño

int indiceDuraciones = 0; // Esta variable nos ayuda a saber cuando se ha completado el array

unsigned long sumaDuraciones = 0; // Variable que nos ayudará a hacer un promedio

// Pasamos a definir variables para el control de válvulas

int relay1 = 10;  //salida digital relay 1
int relay2 = 11;  //salida digital relay 2
int relay3 = 12;  //salida digital relay 3
int goteo = 0;

int val1;  // Valor de trigger para relay 1
int val2;  // Valor de trigger para relay 1
int val3;  // Valor de trigger para relay 1

int ritmo = 10000000;  // Duración de nuestro loop para control de válvulas

void setup() {
  //establecemos las salidas digitales
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);

  //establecemos entrada del biofeedback
  pinMode(interruptPin, INPUT_PULLUP);

  Serial.begin(115200);

  WiFi.begin(ssid, pass);  // establecemos conexión wifi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Retry");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  // Información importante para conectar con otros dispositivos

  // Mostramos los valores con que iniciamos en el Biofeedback
  Serial.println("");
  Serial.println("Valores de control iniciales");
  Serial.print("Cantidad de celdas: ");
  Serial.println(cantidadCeldas);
  Serial.print("Lowpass threshold: ");
  Serial.println(lowpass);
  Serial.print("Highpass threshold: ");
  Serial.println(highpass);

  Udp.begin(localPort);

  // Vamos a inicializar el array para registro de biofeedback
  for (int i = 0; i < cantidadCeldas; i++) {
    pulsoDuraciones[i] = 0;
  }
  attachInterrupt(digitalPinToInterrupt(interruptPin), sample, CHANGE);  // Función que nos permite escuchar por este puerto sin necesidad de depender del void loop
}

void loop() {

  receiveMessage();  //Empezamos escuchando cualquier comando OSC que afecte al comportamiento de la arduino

  if (modalidad == 0) {  // Si modalidad es 0 Arduino se ocupara solo de controlar las válvulas
    
    //asignamos valores random a nuestras variables de acción para válvulas
    val1 = random(ritmo);
    val2 = random(ritmo);
    val3 = random(ritmo);
    
    // Creamos nuestro loop para activación de válvulas
    for (int i = 0; i <= ritmo; i++) {
      if (i == val1) {
        digitalWrite(relay1, HIGH); //Válvula 1 se abre
        Serial.println("Relay1 accionado");
        delay(100);

        digitalWrite(relay1, LOW); //Válvula 1 se cierra
        Serial.println("Relay1 off");
      }
      if (i == val2) {
        digitalWrite(relay2, HIGH); //Válvula 2 se abre
        Serial.println("Relay2 accionado");
        delay(100);

        digitalWrite(relay2, LOW); //Válvula 2 se cierra
        Serial.println("Relay2 off");
      }
      if (i == val3) {
        digitalWrite(relay3, HIGH); //Válvula 3 se abre
        Serial.println("Relay3 accionado");
        delay(100);

        digitalWrite(relay3, LOW); //Válvula 3 se cierra
        Serial.println("Relay3 off");
      }
    }
  }

  if (modalidad == 1) { // Si modalidad es 1 Arduino se ocupara solo leer y traducir el biofeedback además de enviar los resultados vía osc
    //Revisamos si llegó alguna señal del biofeedback
    if (pulseStarted) {
      unsigned long pulsoDuracion = endTime - startTime;  // Vamos a calcular la duración del primer pulso detectado

      //Filtramos los pulsos que no nos interesan con las variables lowpass y highpass como threshold
      if (pulsoDuracion < lowpass && pulsoDuracion > highpass) {

        pulsoDuraciones[indiceDuraciones] = pulsoDuracion; //guardamos la duraci´øn del pulso en la celda del array correspondiente
        sumaDuraciones += pulsoDuraciones[indiceDuraciones]; // vamos sumando los resultados
        indiceDuraciones = (indiceDuraciones + 1) % cantidadCeldas;  // Esto asegura que el índice se mantenga dentro del rango 0-9
        
        if (indiceDuraciones == 0) {
          // Array completado, calcular promedio y enviar mensaje
          unsigned long promedio = sumaDuraciones / cantidadCeldas; //Promedio de las duraciones del pulso en el array
          
          Serial.println("Array completo");
          Serial.print("Promedio del array: "); //visualisamos resultados
          Serial.println(promedio);
          frecuencia = promedio; //almacenamos el valor del promedio en frecuencia
          notaMidi = map(promedio, highpass, lowpass, noteMin, noteMax); // transformamos el valor de promedio con los valores de lowpass y hipass como minimo y máximo a su equivalente en nota midi
          sendMessage(1, static_cast<int>(frecuencia), static_cast<int>(notaMidi)); // enviamos mensajes
          
          // Reiniciar suma para el próximo conjunto de datos
          sumaDuraciones = 0;
        }
      }
      pulseStarted = false;//cambiamos estado
    }
  }
  delay(update_rate);
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

      //Según el comando ejecutamos función específica
      inmsg.dispatch("/modo", modalidadFunctionMessage);
      inmsg.dispatch("/spacing", callbackFunctionMessage);
      inmsg.dispatch("/lowpass", callbackFunctionMessageTwo);
      inmsg.dispatch("/highpass", callbackFunctionMessageThree);
    }
  }
}

//Función para cambiar de modalidad según OSC
void modalidadFunctionMessage(OSCMessage& msg) {
  modalidad = round(msg.getFloat(0));
  Serial.println("MESSAGE RECEIVED");
  Serial.println(modalidad);
}

//Función para cambiar tamaño de array según OSC
void callbackFunctionMessage(OSCMessage& msg) {
  cantidadCeldas = round(msg.getFloat(0));
  Serial.println("MESSAGE RECEIVED");
  Serial.println(cantidadCeldas);
}

//Función para cambiar límite máximo de lectura del biofeedback según OSC
void callbackFunctionMessageTwo(OSCMessage& msg) {
  lowpass = round(msg.getFloat(0));
  Serial.println("Adjusting Threshold Lowpass");
  Serial.println(lowpass);
}

//Función para cambiar límite mínimo de lectura del biofeedback según OSC
void callbackFunctionMessageThree(OSCMessage& msg) {
  highpass = round(msg.getFloat(0));
  Serial.println("Adjusting Threshold highpass");
  Serial.println(highpass);
}

// void callbackFunctionMessageFour(OSCMessage& msg) {
//   goteo = round(msg.getFloat(0));
//   Serial.println("Valvulas On");
//   Serial.println(goteo);
// }

//Función para estructurar mensaje OSC y enviar
void sendMessage(int trigger, int promedio, int freq) {
  //El código sigue en proceso. Esta parte dentro de la función send message no se está utilizando para este proyecto pero lo incluimos igualmente para mostrar las posibilidades
  if (trigger == 0) {
    OSCMessage msg("/note"); //comando que espera pure Data
    msg.add(1); // trigger
    msg.add(static_cast<int>(promedio)); //frecuencia del pulso
    msg.add(static_cast<int>(freq)); // nota Midi

    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);

    Udp.endPacket();
    msg.empty();

  } else if (trigger == 1) {
    OSCMessage msg("/note"); //comando que espera pure Data
    msg.add(trigger); // trigger
    msg.add(static_cast<int>(promedio)); //frecuencia del pulso
    msg.add(static_cast<int>(freq)); // nota Midi

    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);

    Udp.endPacket();
    msg.empty();
  }
}

// Función de lectura de Timer 555
void sample() {
  if (digitalRead(interruptPin) == HIGH) {
    startTime = micros(); //Registramos el tiempo en el que empieza el pulso
  } else {
    endTime = micros(); // Registramos el tiempo en el que termina el pulso
    pulseStarted = true; // Cambiamos variable para habilitar el cálculo de promedio y registro en array
  }
}