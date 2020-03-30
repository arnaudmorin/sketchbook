/*

Demonstrates simple RX and TX operation.
Any of the Basic_RX examples can be used as a receiver.
Please read through 'NRFLite.h' for a description of all the methods available in the library.

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
  int capteur_photo;
  int capteur_haut;
  int capteur_bas;
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
    
    _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS, 108);
}

void loop()
{
    paquetSortant.mode = 1;
    paquetSortant.compteur_nuit_max = 450;
    paquetSortant.jour = 100;

    Serial.print("Envoi config...");

    // By default, 'send' transmits data and waits for an acknowledgement.  If no acknowledgement is received,
    // it will try again up to 16 times.  You can also perform a NO_ACK send that does not request an acknowledgement.
    // The data packet will only be transmitted a single time so there is no guarantee it will be successful.  Any random
    // electromagnetic interference can sporatically cause packets to be lost, so NO_ACK sends are only suited for certain
    // types of situations, such as streaming real-time data where performance is more important than reliability.
    //   _radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::NO_ACK)
    //   _radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::REQUIRE_ACK) // THE DEFAULT
    
    if (_radio.send(DESTINATION_RADIO_ID, &paquetSortant, sizeof(paquetSortant))) // Note how '&' must be placed in front of the variable name.
    {
        Serial.println("...Success");
    }
    else
    {
        Serial.println("...Failed");
    }

    while (_radio.hasData())
    {
        _radio.readData(&paquetEntrant); // Note how '&' must be placed in front of the variable name.

        String msg = "capteur_photo=";
        msg += paquetEntrant.capteur_photo;
        msg += ", capteur_haut=";
        msg += paquetEntrant.capteur_haut;
        msg += ", capteur_bas=";
        msg += paquetEntrant.capteur_bas;
        msg += ", compteur_nuit=";
        msg += paquetEntrant.compteur_nuit;

        Serial.println(msg);
    }

    delay(1000);
}
