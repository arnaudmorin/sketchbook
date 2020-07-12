/*

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

#include <SPI.h>
#include <NRFLite.h>

const static uint8_t RADIO_ID = 0;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 1; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;

/*
 * Paquet sortant, emis par le poulailler
 * Pourrait aller jusqu'a 32 bytes
 * int = 2 bytes
 */
struct PaquetEntrant
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
struct PaquetSortant
{
  int mode;
  int jour;
  int compteur_nuit_max;
  int hyst;
};

NRFLite _radio;
PaquetEntrant paquetEntrant;
PaquetSortant paquetSortant;

void setup()
{
  Serial.begin(115200);
  
  paquetSortant.mode = 1;   // 1 = auto / 2 = fermeture / 3 = ouverture
  paquetSortant.compteur_nuit_max = 451;
  paquetSortant.jour = 51;
  paquetSortant.hyst = 21;

  _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN);
}

void loop()
{
  while (_radio.hasData()){
    Serial.println("Received:");
    _radio.readData(&paquetEntrant); // Note how '&' must be placed in front of the variable name.
    Serial.print(" capteur_photo     = ");
    Serial.println(paquetEntrant.capteur_photo);
    Serial.print(" capteur_haut      = ");
    Serial.println(paquetEntrant.capteur_haut);
    Serial.print(" capteur_bas       = ");
    Serial.println(paquetEntrant.capteur_bas);
    Serial.print(" compteur_nuit     = ");
    Serial.println(paquetEntrant.compteur_nuit);
    Serial.print(" compteur_nuit_max = ");
    Serial.println(paquetEntrant.compteur_nuit_max);
    Serial.print(" jour              = ");
    Serial.println(paquetEntrant.jour);
    Serial.print(" hyst              = ");
    Serial.println(paquetEntrant.hyst);
    Serial.print(" mode              = ");
    Serial.println(paquetEntrant.mode);

    _radio.addAckData(&paquetSortant, sizeof(paquetSortant));
  }
}
