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
const long timeout_portes = 30000; // Temps max pour monter ou descendre
long timeout = 0; // Variable pour compter le temps qui passe a descendre
int state = 0;  // state machine
                // 0: inconnu
                // 1: haut
                // 2: en cours de descente
                // 3: bas
                // 4: en cours de montee
const int jour = 2; // il fait jour si superieur a ca
const int hyst = 1; // avec cet d'hysteresis

void start_timeout(){
  // Init timeout
  timeout = millis() + timeout_portes;
}

/*
 * Fonction pour fermer la porte
 */
void fermer_porte(){
  Serial.println("Action: fermeture porte");
  digitalWrite(PIN_moteur1, HIGH);
  digitalWrite(PIN_moteur2, LOW);
}

/*
 * Fonction pour ouvrir la porte
 */
void ouvrir_porte(){
  Serial.println("Action: ouverture porte");
  digitalWrite(PIN_moteur1, LOW);
  digitalWrite(PIN_moteur2, HIGH);
}

/*
 * Fonction pour stopper la porte
 */
void stop_porte(){
  Serial.println("Action: stop porte");
  digitalWrite(PIN_moteur1, LOW);
  digitalWrite(PIN_moteur2, LOW);  
}

/*
 * Fonction pour entrer en sleep mode
 */
void goToSleep(){
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
void watchdogOn(){
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
ISR(WDT_vect){
  // Do nothing
}

/*
 * Setup
 */
void setup(){
  Serial.begin(115200);
  
  // Turn on watchdog
  watchdogOn();

  // Initialise
  pinMode(PIN_capteur_haut, INPUT_PULLUP); // 1 == capteur ouvert / 0 = capteur ferme
  pinMode(PIN_capteur_bas, INPUT_PULLUP);
  pinMode(PIN_photo, INPUT);
  digitalWrite(PIN_moteur1, LOW);
  digitalWrite(PIN_moteur2, LOW);
}

/*
 * Loop
 */
void loop(){

  Serial.println("");

  String msg = "Capteurs: ";
  msg += " photo=";
  msg += analogRead(PIN_photo);
  msg += " haut=";
  msg += digitalRead(PIN_capteur_haut);
  msg += " bas=";
  msg += digitalRead(PIN_capteur_bas);
  Serial.println(msg);

  // State machine
  switch( state ) {
    case 0:
      Serial.println("State=inconnu");
      // Etat inconnu, on monte
      ouvrir_porte();
      // Si on atteind le capteur haut
      if (digitalRead(PIN_capteur_haut) == 0){
        stop_porte();
        state = 1;
      }
      break;
    case 1:
      Serial.println("State=en haut");
      // on est en haut
      // Si il fait nuit, on descend
      if (analogRead(PIN_photo) <= jour) {
        fermer_porte();
        start_timeout();
        state = 2;
      }
      break;
    case 2:
      Serial.println("State=en cours de descente");
      // on est en cours de descente
      // on attend la fin du timeout pour stopper la porte
      if (millis() > timeout) {
        stop_porte();
        state = 3;
      }
      break;
    case 3:
      Serial.println("State=en bas");
      // on est en bas
      // Si il fait jour, on monte
      if (analogRead(PIN_photo) > jour + hyst) {
        ouvrir_porte();
        state = 4;
      }
      break;
    case 4:
      Serial.println("State=en cours de montee");
      // on est en cours de montee
      // Si on atteind le capteur haut
      if (digitalRead(PIN_capteur_haut) == 0){
        stop_porte();
        state = 1;
      }
      break;
  }

  // Si on est dans un etat stable, on peut dormir
  if ((state == 1) or (state == 3)) {
    // ATmega328 goes to sleep for about 8 seconds
    Serial.println("Fin: je vais dormir pendant 8 secondes");
    Serial.flush();
    wdt_reset();
    goToSleep();
  }

  // Evitons de tourner trop vite
  delay(1000);
}
