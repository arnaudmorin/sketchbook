/* Arnaud Morin
 * Poulailler
 */

/*
 * Librairies
 */
#include <avr/sleep.h>

/*
 * Definition des PIN et variables globales
 */
int capteur_haut = 2;
int capteur_bas = 3;
int in1 = 11;
int in2 = 12;
int photo = A0;
int jour = 30; // il fait jour si superieur a ca - avec 10 d'hysteresis
bool debug = true;

/*
 * Fonction pour fermer la porte
 */
void fermer_porte()
{
  // Timeout de 45sec pour descendre
  long timeout = millis() + 45000;

  // Tant qu'on a pas atteind le capteur bas
  while ((millis() < timeout) && (digitalRead(capteur_bas) == true)) {
    // On descend
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }

  // On s'arrete
  digitalWrite(in1, LOW);
}

/*
 * Fonction pour ouvrir la porte
 */
void ouvrir_porte()
{
  // Timeout de 45sec pour descendre
  long timeout = millis() + 45000;

  // Tant qu'on a pas atteind le capteur bas
  while ((millis() < timeout) && (digitalRead(capteur_haut) == true)) {
    // On monte
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }

  // On s'arrete
  digitalWrite(in2, LOW);
}

/*
 * Fonction pour entrer en sleep mode
 */
void goToSleep()   
{
// The ATmega328 has five different sleep states.
// See the ATmega 328 datasheet for more information.
// SLEEP_MODE_IDLE -the least power savings 
// SLEEP_MODE_ADC
// SLEEP_MODE_PWR_SAVE
// SLEEP_MODE_STANDBY
// SLEEP_MODE_PWR_DOWN -the most power savings
// I am using the deepest sleep mode from which a
// watchdog timer interrupt can wake the ATMega328

set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode.
sleep_enable(); // Enable sleep mode.
sleep_mode(); // Enter sleep mode.
// After waking from watchdog interrupt the code continues
// to execute from this point.
sleep_disable(); // Disable sleep mode after waking.
}

/*
 * Fonction pour activer le watchdog
 */
void watchdogOn() {
  
// Clear the reset flag, the WDRF bit (bit 3) of MCUSR.
MCUSR = MCUSR & B11110111;
  
// Set the WDCE bit (bit 4) and the WDE bit (bit 3) 
// of WDTCSR. The WDCE bit must be set in order to 
// change WDE or the watchdog prescalers. Setting the 
// WDCE bit will allow updtaes to the prescalers and 
// WDE for 4 clock cycles then it will be reset by 
// hardware.
WDTCSR = WDTCSR | B00011000; 

// Set the watchdog timeout prescaler value to 1024 K
// which will yeild a time-out interval of about 8.0 s.
WDTCSR = B00100001;

// Enable the watchdog timer interupt.
WDTCSR = WDTCSR | B01000000;
MCUSR = MCUSR & B11110111;

}

/*
 * Setup
 */
void setup()
{
  // Turn on watchdog
  watchdogOn();

  // Initialise
  pinMode(capteur_haut, INPUT_PULLUP);
  pinMode(capteur_bas, INPUT_PULLUP);
  pinMode(photo, INPUT);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  if (debug) Serial.begin(9600);
}

/*
 * Loop
 */
void loop()
{
  // Lecture valeur luminosite (entre 0 et 1023)
  int photo_read = analogRead(photo);
  
  // Debug log
  if (debug){
    Serial.print("Capteur (photo / haut / bas): ");
    Serial.print(photo_read);
    Serial.print(" / ");
    Serial.print(digitalRead(capteur_haut));
    Serial.print(" / ");
    Serial.println(digitalRead(capteur_bas));
  }
  
  // Si il fait jour, on ouvre la porte
  if (photo_read > jour + 10) {
    if (debug) Serial.println("Ouverture porte");
    ouvrir_porte();
  }
  // Sinon on ferme
  else if (photo_read < jour - 10) {
    if (debug) Serial.println("Fermeture porte");
    fermer_porte();
  }

  // ATmega328 goes to sleep for about 8 seconds
  if (debug) Serial.flush();
  goToSleep();
}
