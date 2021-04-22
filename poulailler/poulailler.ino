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
const int PIN_capteur_haut = 2;
const int PIN_capteur_bas = 3;
const int PIN_moteur1 = 7;
const int PIN_moteur2 = 8;
const int PIN_photo = A0;
const long timeout_portes = 45000;
const int jour = 50; // il fait jour si superieur a ca
const int hyst = 20; // avec cet d'hysteresis
int compteur_nuit = 0 ; // Nombre de fois quil fait nuit, reset a 0 a chaque fois quil fait jour
const int compteur_nuit_max = 0; // Nombre de fois quil doit faire nuit pour fermer la porte, comme l'arduino se met en veille environ 8s avant de refaire une boucle, 450*8s = 3600s = 1H
                           // Le systeme attend done une heure apres la nuit avant de fermer, ca permet a coco de rentrer tranquilement.
const int mode = 1; // 1 = auto, 2 = fermer, 3 = ouvrir

/*
 * Fonction pour fermer la porte
 */
void fermer_porte()
{
  Serial.println("Action: fermeture porte");
  // Timeout de 45sec pour descendre
  long timeout = millis() + timeout_portes;

  // Tant qu'on a pas atteind le capteur bas
  while ((millis() < timeout) && (digitalRead(PIN_capteur_bas) == true)) {
    // On descend
    digitalWrite(PIN_moteur1, HIGH);
    digitalWrite(PIN_moteur2, LOW);
  }

  // On s'arrete
  digitalWrite(PIN_moteur1, LOW);
}

/*
 * Fonction pour ouvrir la porte
 */
void ouvrir_porte()
{
  Serial.println("Action: ouverture porte");
  // Timeout de 45sec pour monter
  long timeout = millis() + timeout_portes;

  // Tant qu'on a pas atteind le capteur haut
  while ((millis() < timeout) && (digitalRead(PIN_capteur_haut) == true)) {
    // On monte
    digitalWrite(PIN_moteur1, LOW);
    digitalWrite(PIN_moteur2, HIGH);
  }

  // On s'arrete
  digitalWrite(PIN_moteur2, LOW);
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
  Serial.begin(115200);
  
  // Turn on watchdog
  watchdogOn();

  // Initialise
  pinMode(PIN_capteur_haut, INPUT_PULLUP);
  pinMode(PIN_capteur_bas, INPUT_PULLUP);
  pinMode(PIN_photo, INPUT);
  digitalWrite(PIN_moteur1, LOW);
  digitalWrite(PIN_moteur2, LOW);
}

/*
 * Loop
 */
void loop()
{
  // Lecture valeur luminosite (entre 0 et 1023)
  int capteur_photo = analogRead(PIN_photo);
  // Lecture capteur haut et bas
  int capteur_haut = digitalRead(PIN_capteur_haut);
  int capteur_bas = digitalRead(PIN_capteur_bas);

  Serial.println("");

  String msg = "Capteurs: ";
  msg += " photo=";
  msg += capteur_photo;
  msg += " haut=";
  msg += capteur_haut;
  msg += " bas=";
  msg += capteur_bas;
  Serial.println(msg);

  msg = "Config: ";
  msg += " jour=";
  msg += jour;
  msg += " hyst=";
  msg += hyst;
  msg += " compteur_nuit_max=";
  msg += compteur_nuit_max;
  Serial.println(msg);

  msg = "Compteurs: ";
  msg += " compteur_nuit=";
  msg += compteur_nuit;
  Serial.println(msg);

  if (mode == 1) {
    // Mode auto
    Serial.println("Mode: auto");
    // Si il fait jour, on ouvre la porte
    if (capteur_photo > jour + hyst) {
      Serial.println("Info: il fait jour");
      compteur_nuit = 0;
      ouvrir_porte();
    }
    // Sinon on ferme
    else if (capteur_photo <= jour) {
      Serial.println("Info: il fait nuit");
      if (compteur_nuit >= compteur_nuit_max) {
        fermer_porte();
      }
      else {
        Serial.println("Action: attente");
        compteur_nuit++;
      }
    }
    else {
      Serial.println("Info: il fait entre jour et nuit (hyst)");
      Serial.println("Action: attente");
    }
  }
  else if (mode == 2){
    // Fermer
    Serial.println("Mode: fermeture");
    fermer_porte();
  }
  else if (mode == 3){
    // Ouvrir
    Serial.println("Mode: ouverture");
    ouvrir_porte();
  }

  // ATmega328 goes to sleep for about 8 seconds
  Serial.println("Fin: je vais dormir pendant 8 secondes");
  Serial.flush();
  wdt_reset();
  goToSleep();
  //delay(1000);
}
