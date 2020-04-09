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
const static uint8_t RADIO_CHANNEL = 1;
String action;
int test;

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
  Serial.begin(9600);

  // By default, 'init' configures the radio to use a 2MBPS bitrate on channel 100 (channels 0-125 are valid).
  // Both the RX and TX radios must have the same bitrate and channel to communicate with each other.
  // You can run the 'ChannelScanner' example to help select the best channel for your environment.
  // You can assign a different bitrate and channel as shown below.
  //   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS, 0)
  //   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE1MBPS, 75)
  //   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100) // THE DEFAULT

  _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS, RADIO_CHANNEL);
}

void loop()
{
  paquetSortant.mode = 1;
  paquetSortant.compteur_nuit_max = 449;
  paquetSortant.jour = 51;
  paquetSortant.hyst = 21;

  // By default, 'send' transmits data and waits for an acknowledgement.  If no acknowledgement is received,
  // it will try again up to 16 times.  You can also perform a NO_ACK send that does not request an acknowledgement.
  // The data packet will only be transmitted a single time so there is no guarantee it will be successful.  Any random
  // electromagnetic interference can sporatically cause packets to be lost, so NO_ACK sends are only suited for certain
  // types of situations, such as streaming real-time data where performance is more important than reliability.
  //   _radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::NO_ACK)
  //   _radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::REQUIRE_ACK) // THE DEFAULT

  // Read data from serial
  if (Serial.available() > 0) {
    action = Serial.readString();
    Serial.print("action: ");
    Serial.println(action);
    test = paquetSortant.jour;
    Serial.println(test);
  }

  //switch(tolower(action[0])) {
    //case 'r':
      //Serial.println("Checking if data are received");
      
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
      }
      
    //break;
    //case 'c':
      //Serial.println("Trying to send config");

      /*
      if (_radio.send(DESTINATION_RADIO_ID, &paquetSortant, sizeof(paquetSortant))){
        Serial.println("...Success");
      }
      else {
        Serial.println("...Failed");
      }
      */
    //break;
    //default:
    //  Serial.println("Nothing to do");
    //break;
  //}

  // Reset action
  //action = "";

  // Wait a bit before next loop
  delay(200);
}
