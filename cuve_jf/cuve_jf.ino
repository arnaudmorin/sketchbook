#include <Wire.h>
#include<LiquidCrystal_I2C.h>

// Les variables
float cuve_pleine = 125;  // hauteur en cm quand la cuve est pleine
float volume_pleine = cuve_pleine * 110 * 258 ; // volume cuve pleine
int capteur = 151;        // hauteur en cm du capteur
int trig = 9;             // PIN9 branché sur le TRIG du module (bleu)
int echo = 8;             // PIN8 branché sur l'ECHO du module (gris)
long echo_lu;             // le temps aller/retour de l'impulsion en us
long cm;                  // la hauteur d'eau mesuree
float pourcent;           // la hauteur en pourcentage
float volume;             // le volume d'eau
char string[3];           // string array pour le LCD


// Le LCD, I2C sur arduino:
//  A4 --> SDA (Serial Data) - Gris
//  A5 --> SCL (Serial Clock) - Bleu
//LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2C lcd(0x3F, 16, 2);


void setup(){
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT);
  Serial.begin(9600);     // Pour debug, log console
  lcd.init();             // LCD
  lcd.backlight();
}

void loop(){
  digitalWrite(trig, HIGH);       // on envoi une impulsion...
  delayMicroseconds(10);          // ... pendant 10 micro secondes ...
  digitalWrite(trig, LOW);        // ... et on arrête l'envoi de l'impulsion
  echo_lu = pulseIn(echo, HIGH);  // et on écoute l'impulsion revenir

  // Calculs
  cm = capteur - (echo_lu / 50);
  pourcent = cm * 100 / cuve_pleine;
  volume = volume_pleine * cm / cuve_pleine / 1000;   // en litre
  
  // Debug on console
  Serial.print(cm);
  Serial.println(" cm");

  
  lcd.clear();

  // Affiche les valeurs
  lcd.setCursor(0, 0);
  lcd.print(cm);
  lcd.print(" cm / ");
  lcd.print(pourcent);
  lcd.print(" %");

  lcd.setCursor(0, 1);
  
  // Affiche le volume
  lcd.print(volume);
  lcd.print(" L");

  // Attend une seconde avant prochain check
  delay(1000);
}
