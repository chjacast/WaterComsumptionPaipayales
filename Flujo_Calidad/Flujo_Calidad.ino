#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 53
#define RFM95_RST 9
#define RFM95_INT 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


#include <SoftwareSerial.h>
#include <ModbusMaster.h>
 
#define SLAVE_ID 1
#define REGISTRO_FLUJO 1
#define LONGITUD_FLUJO 2
#define REGISTRO_CONSUMO 113
#define LONGITUD_CONSUMO 2
#define MAX485_DE 22
#define MAX485_RE 21


// Instanciar el objeto con ambas librerías. 

ModbusMaster sensor;

void preTransmission()
{
  delay(10);  
  digitalWrite(MAX485_RE, HIGH); // Modo transmisión.
  digitalWrite(MAX485_DE, HIGH);
}

void postTransmission()
{
  delay(10);
  digitalWrite(MAX485_RE,LOW); //Modo recepción
  digitalWrite(MAX485_DE,LOW);
}


void setup()
{
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE,LOW);
  digitalWrite(MAX485_DE,LOW);
  //Serial.begin(9600);
  //Serial.println("PRUEBA CON EL SIMULADOR SLAVE MODBUS");
  Serial1.begin(9600,SERIAL_8N1); //Cambio 8o1 por 8N1
  sensor.begin(1, Serial1); //SLAVE_ID cambio por 1  este serial es para el flujometro
  sensor.preTransmission(preTransmission);
  sensor.postTransmission(postTransmission);

   pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  delay(100);

  Serial.println("Arduino LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission


union
{
uint32_t i;
float f;
}flujo;

union
{
uint32_t j;
float t;
}consu;

void loop() 
{
  //Serial.println("LECTURA DE REGISTROS");
  leer_flujo();
  leer_consumo();
  //imprimir_resultados();
  Serial.println(consu.t);
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  
  char radiopacket[20] = "hell       ";
  itoa(packetnum++, radiopacket+5, 10);
  //char flujo ;
  //String valor = "5,6";
  String cinco = "528,30 ";
  int cuatro = 4;
  
  float flujoso =flujo.f;
  float consumos = float(consu.t);
  
  String color = String(flujoso)+","+String(consumos); 
  const char* nuevo="";
   
  int color_len= color.length() +1 ;
  char char_array_color [color_len];
  color.toCharArray(char_array_color,color_len);
  nuevo = char_array_color;

  int caudal = 4;
  
  Serial.print("Sending "); Serial.println(/*String(radiopacket)+","+String(cinco)+","+String(caudal)*/ nuevo);
  /*radiopacket[19] = 0;*/
  
  Serial.println("Sending..."); delay(10);
  rf95.send((uint8_t *)nuevo, 10);
  
  
  Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
  delay(1000);
}

void leer_flujo(){
 uint8_t result;
 result = sensor.readHoldingRegisters(REGISTRO_FLUJO,LONGITUD_FLUJO);
 delay(1500);
 if (result == sensor.ku8MBSuccess)
  {
   flujo.i=(((unsigned long)sensor.getResponseBuffer(0x00)<<16) | (sensor.getResponseBuffer(0x01)));
   }
 }
  
void leer_consumo(){
 uint8_t result;
 result = sensor.readHoldingRegisters(REGISTRO_CONSUMO,LONGITUD_CONSUMO);
 delay(1500);
 if (result == sensor.ku8MBSuccess)
  {
  consu.j=(((unsigned long)sensor.getResponseBuffer(0x00)<<16) | (sensor.getResponseBuffer(0x01)));
 
  //cons.k=(((unsigned long)sensor.getResponseBuffer(0x03)<<16) | (sensor.getResponseBuffer(0x02)));
  }
 } 

//Funcion para leer error presente en el flujometro




void imprimir_resultados(){
//  Serial.println("Temperatura 1 es:  "+String(temp.t,5)+" grados Celcius");  
//  Serial.println("Temperatura 2 es:  "+String(tem.te,5)+" grados Celcius"); 
  Serial.println("Flujo es:  "+String(flujo.f,5)+ " l/s");
  // Imprimimos por pantalla el registro de error
  }
 
