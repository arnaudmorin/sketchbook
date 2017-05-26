// Pour le LCD
#include <SoftwareSerial.h>

// Les variables
int cuve_pleine = 160;    // hauteur en cm quand la cuve est pleine
int capteur = 170;        // hauteur en cm du capteur
int trig = 9;             // PIN9 branché sur le TRIP du module (vert)
int echo = 8;             // PIN8 branché sur l'ECHO du module (bleu)
int lcd = 2;              // PIN2 branché sur RX LCD (jaune)
long echo_lu;             // le temps aller/retour de l'impulsion en us
int cm;                   // la hauteur
int pourcent;
char string[3];           // string array pour le LCD

// Attach the serial display's RX line to digital pin
// See https://www.sparkfun.com/tutorials/246
SoftwareSerial mySerial(3,lcd); // pin 2 = TX, pin 3 = RX (unused)

void setup()
{
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT);
  Serial.begin(9600);     // Pour debug, log console
  mySerial.begin(9600);   // LCD
  delay(500);             // wait for display to boot up
}

void loop()
{
  digitalWrite(trig, HIGH);       // on envoi une impulsion...
  delayMicroseconds(10);          // ... pendant 10 micro secondes ...
  digitalWrite(trig, LOW);        // ... et on arrête l'envoi de l'impulsion
  echo_lu = pulseIn(echo, HIGH);  // et on écoute l'impulsion revenir
  
  // Calculs
  cm = capteur - (echo_lu / 50);
  
  // Debug on console
  Serial.print(cm);
  Serial.println(" cm");
  
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);

  mySerial.write("                "); // clear display
  mySerial.write("                ");

  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
 
  mySerial.write("Hauteur cuve :");
  
  mySerial.write(254); // move cursor to beginning of second line
  mySerial.write(192);
  
  sprintf(string,"%3u",cm);
  mySerial.write(string);
  mySerial.write(" cm, ");
  
  pourcent = cm * 100 / cuve_pleine;
  sprintf(string,"%3u",pourcent);
  mySerial.write(string);
  mySerial.write(" %");
  
  delay(1000);
}
