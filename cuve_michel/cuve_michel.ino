// Pour le LCD
#include <SoftwareSerial.h>

// Les variables
float cuve_pleine = 125;  // hauteur en cm quand la cuve est pleine
float volume_pleine = cuve_pleine * 110 * 258 ; // volume cuve pleine
int capteur = 151;        // hauteur en cm du capteur
int trig = 8;             // TRIG
int echo = 9;             // ECHO
int lcd_rx = 2;           // PIN2 branché sur RX LCD (jaune)
int lcd_tx = 3;           // PIN3 branché sur TX LCD (non utilisée)
long echo_lu;             // le temps aller/retour de l'impulsion en us
long cm;                  // la hauteur d'eau mesuree
float pourcent;           // la hauteur en pourcentage
float volume;             // le volume d'eau
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
  clearDisplay();         // Efface ecran
}

void clearDisplay()
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x01);  // send the clear screen command
}

void setLCDCursor(byte cursor_position)
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x80);  // send the set cursor command
  lcd.write(cursor_position);  // send the cursor position
}

void loop()
{
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
  
  // Déplace le curseur au debut de la premiere ligne
  setLCDCursor(0);

  // Efface l'écran
  lcd.print("                ");
  lcd.print("                ");

  // Déplace le curseur au debut de la premiere ligne
  setLCDCursor(0);

  // Affiche les valeurs
  lcd.print(cm);
  lcd.print(" cm / ");
  lcd.print(pourcent);
  lcd.print(" %");
  
  // Déplace le curseur au debut de la seconde ligne
  setLCDCursor(16);
  
  // Affiche le volume
  lcd.print(volume);
  lcd.print(" L");

  // Attend une seconde avant prochain check
  delay(1000);
}

