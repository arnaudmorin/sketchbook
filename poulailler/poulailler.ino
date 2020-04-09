/* Arnaud Morin
 * Poulailler
 */

/*
 * Librairies
 */
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <NRFLite.h>

/*
 * Definition des PIN et variables globales
 */
const int PIN_capteur_haut = 2;
const int PIN_capteur_bas = 3;
const int PIN_in1 = 7;
const int PIN_in2 = 8;
const int PIN_photo = A0;
volatile int jour = 50; // il fait jour si superieur a ca
volatile int hyst = 20; // avec cet d'hysteresis
int compteur_nuit = 0 ; // Nombre de fois quil fait nuit, reset a 0 a chaque fois quil fait jour
volatile int compteur_nuit_max = 450; // Nombre de fois quil doit faire nuit pour fermer la porte, comme l'arduino se met en veille environ 8s avant de refaire une boucle, 450*8s = 3600s = 1H
                           // Le systeme attend done une heure apres la nuit avant de fermer, ca permet a coco de rentrer tranquilement.
volatile int mode = 1; // 1 = auto, 2 = fermer, 3 = ouvrir

/* Params radio avec NRF24L01
Radio    Arduino
CE    -> 9
CSN   -> 10 (Hardware SPI SS)
MOSI  -> 11 (Hardware SPI MOSI)
MISO  -> 12 (Hardware SPI MISO)
SCK   -> 13 (Hardware SPI SCK)
IRQ   -> No connection
VCC   -> No more than 3.6 volts
GND   -> GND
*/
const static uint8_t RADIO_ID = 1;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 0; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;

/*
 * Paquet sortant, emis par le poulailler
 * Pourrait aller jusqu'a 32 bytes
 * int = 2 bytes
 */
struct PaquetSortant
{
  int compteur_nuit;
  int compteur_nuit_max;
  int capteur_photo;
  int capteur_haut;
  int capteur_bas;
  int jour;
  int hyst;
  int mode;
};

/*
 * Paquet entrant, arrivant au poulailler
 * Utilise pour changer dynamiquement la config
 * 
 */
struct PaquetEntrant
{
  int mode;
  int jour;
  int compteur_nuit_max;
  int hyst;
};

NRFLite _radio;

/*
 * Fonction pour fermer la porte
 */
void fermer_porte()
{
  Serial.println("Action: fermeture porte");
  // Timeout de 45sec pour descendre
  long timeout = millis() + 45000;

  // Tant qu'on a pas atteind le capteur bas
  while ((millis() < timeout) && (digitalRead(PIN_capteur_bas) == true)) {
    // On descend
    digitalWrite(PIN_in1, HIGH);
    digitalWrite(PIN_in2, LOW);
  }

  // On s'arrete
  digitalWrite(PIN_in1, LOW);
}

/*
 * Fonction pour ouvrir la porte
 */
void ouvrir_porte()
{
  Serial.println("Action: ouverture porte");
  // Timeout de 45sec pour monter
  long timeout = millis() + 45000;

  // Tant qu'on a pas atteind le capteur haut
  while ((millis() < timeout) && (digitalRead(PIN_capteur_haut) == true)) {
    // On monte
    digitalWrite(PIN_in1, LOW);
    digitalWrite(PIN_in2, HIGH);
  }

  // On s'arrete
  digitalWrite(PIN_in2, LOW);
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
  digitalWrite(PIN_in1, LOW);
  digitalWrite(PIN_in2, LOW);
  _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN);
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

  // Ecriture du paquetSortant
  PaquetSortant paquetSortant;
  paquetSortant.capteur_photo = capteur_photo;
  paquetSortant.capteur_haut = capteur_haut;
  paquetSortant.capteur_bas = capteur_bas;
  paquetSortant.compteur_nuit = compteur_nuit;
  paquetSortant.compteur_nuit_max = compteur_nuit_max;
  paquetSortant.jour = jour;
  paquetSortant.hyst = hyst;
  paquetSortant.mode = mode;

  // Envoi du paquetSortant
  Serial.println("Action: envoi radio");
  _radio.send(DESTINATION_RADIO_ID, &paquetSortant, sizeof(paquetSortant));//, NRFLite::NO_ACK);

  // Reception paquetEntrant
  Serial.println("Action: lecture radio");
  while (_radio.hasAckData()) {
    PaquetEntrant paquetEntrant;
    _radio.readData(&paquetEntrant);

    String msg = "Config received:";
    msg += " mode=";
    msg += paquetEntrant.mode;
    msg += " compteur_nuit_max=";
    msg += paquetEntrant.compteur_nuit_max;
    msg += " jour=";
    msg += paquetEntrant.jour;
    msg += " hyst=";
    msg += paquetEntrant.hyst;
    Serial.println(msg);

    compteur_nuit_max = paquetEntrant.compteur_nuit_max;
    jour = paquetEntrant.jour;
    mode = paquetEntrant.mode;
    hyst = paquetEntrant.hyst;
  }

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
    else if (capteur_photo < jour) {
      Serial.println("Info: il fait nuit");
      if (compteur_nuit > compteur_nuit_max) {
        fermer_porte();
      }
      else {
        Serial.println("Action: attente");
        compteur_nuit++;
      }
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
}
