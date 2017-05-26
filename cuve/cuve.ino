// Pour le LCD
#include <SoftwareSerial.h>

// Les variables
int cuve_pleine = 160;    // hauteur en cm quand la cuve est pleine
int capteur = 170;        // hauteur en cm du capteur
int trig = 9;             // PIN9 branché sur le TRIP du module (vert)
int echo = 8;             // PIN8 branché sur l'ECHO du module (bleu)
int lcd_rx = 2;           // PIN2 branché sur RX LCD (jaune)
int lcd_tx = 3;           // PIN3 branché sur TX LCD (non utilisée)
long echo_lu;             // le temps aller/retour de l'impulsion en us
long cm;                  // la hauteur
char string[3];           // string array pour le LCD

// Attach the serial display's RX line to digital pin
// See https://www.sparkfun.com/tutorials/246
SoftwareSerial lcd(lcd_tx,lcd_rx);

void setup()
{
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT);
  Serial.begin(9600);     // Pour debug, log console
  lcd.begin(9600);        // LCD
  delay(1000);            // wait for display to boot up
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
  
  // Déplace le curseur au debut de la premiere ligne
  lcd.write(254);
  lcd.write(128);

  // Efface l'écran
  lcd.write("                ");
  lcd.write("                ");

  // Déplace le curseur au debut de la premiere ligne
  lcd.write(254);
  lcd.write(128);

  // Affiche les valeurs
  lcd.print("Hauteur cuve :  ");
  lcd.print(cm);
  lcd.print(" cm / ");
  lcd.print(cm * 100 / cuve_pleine);
  lcd.print(" %");

  // Attend une seconde avant prochain check
  delay(1000);
}
