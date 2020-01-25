/* Arnaud Morin
 * Poulailler
 */

/*
 * Librairies
 */
#include <avr/sleep.h>
#include <avr/wdt.h>

/*
 * Definition des PIN et variables globales
 */
int capteur_haut = 2;
int capteur_bas = 3;
int in1 = 11;
int in2 = 12;
int photo = A0;
int jour = 10; // il fait jour si superieur a ca
int hyst = 20; // avec cet d'hysteresis
int c_nuit = 0 ; // Nombre de fois quil fait nuit, reset a 0 a chaque fois quil fait jour
int c_nuit_max = 450; // Nombre de fois quil doit faire nuit pour fermer la porte, comme l'arduino se met en veille environ 8s avant de refaire une boucle, 450*8s = 3600s = 1H
                      // Le systeme attend done une heure apres la nuit avant de fermer, ca permet a coco de rentrer tranquilement.
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
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Set sleep mode.
  sleep_enable();                       // Enable sleep mode.
  sleep_mode();                         // Enter sleep mode.

  // After waking from watchdog interrupt the code continues
  // to execute from this point.
  sleep_disable();                      // Disable sleep mode after waking.

  // Reset the watchdog
  wdt_reset();
}

/*
 * Fonction pour activer le watchdog
 */
void watchdogOn() {
  /* 
   WDTCSR (Watch Dog Timer register) contains a byte (8 bits)
      7 WDIF -- Interrupt Flag / automatically flagged high and low by the system.
      6 WDIE -- Interrupt Enable
      5 WDP3 -- See timeout table
      4 WDCE -- Configuration Enable, set this to 1 to enter setup mode (and change values of the register). It works only for 4 cycles
      3 WDE  -- Enable the watchdog
      2 WDP2 -- See timeout table
      1 WDP1 -- See timeout table
      0 WDP0 -- See timeout table

      WDP WDP WDP WDP TimeOut
       3   2   1   0   (ms)  
       0   0   0   0   16  
       0   0   0   1   32  
       0   0   1   0   64  
       0   0   1   1   125  
       0   1   0   0   250  
       0   1   0   1   500  
       0   1   1   0   1000  
       0   1   1   1   2000  
       1   0   0   0   4000  
       1   0   0   1   8000
  */

  // Disable all interrupts so that configuration is never disrupted and left unfinished
  cli();

  // Reset the watchdog before configuring it
  // As this is usually not necessary, and it may contain a bug (see https://forum.arduino.cc/index.php?topic=483731.0)
  wdt_reset();

  // Clear the reset flag, the WDRF bit (bit 3) of MCUSR.
  // To clear WDE, WDRF must be cleared first. 
  MCUSR &= ~(1<<WDRF);
  //MCUSR = MCUSR & B11110111;

  // Enter "Setup" mode, by setting WDCE and WDE to 1
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  //WDTCSR = WDTCSR | B00011000; 

  // Set the watchdog timeout prescaler value to 1024 K
  // which will yeild a time-out interval of about 8.0 s.
  WDTCSR = B01100001;

  // Enable back the interrupts
  sei();
}

/*
 * Function called when waking up from watchdog
 */
ISR(WDT_vect) {
  // Do nothing
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
  if (photo_read > jour + hyst) {
    if (debug) Serial.println("Il fait jour");
    c_nuit = 0;
    ouvrir_porte();
  }
  // Sinon on ferme
  else if (photo_read < jour) {
    if (debug){
      Serial.println("Il fait nuit");
      Serial.print("c_nuit = ");
      Serial.println(c_nuit);
    }
    if (c_nuit > c_nuit_max) {
      if (debug) Serial.println("c_nuit > c_nuit_max, fermeture porte");
      fermer_porte();
    }
    else {
      if (debug) Serial.println("c_nuit < c_nuit_max, attente");
      c_nuit++;
    }
  }

  // ATmega328 goes to sleep for about 8 seconds
  if (debug) Serial.println("Dodo");
  if (debug) Serial.flush();
  wdt_reset();
  goToSleep();
}
