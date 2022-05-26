/*
Basic Panasonic CQ-FX55LEN (and similar) CD Changer emulator
Steve Hennerley 2017

 
Requires 3 IO Pins - tested on ATMEGA 328P 16MHz
 
//TODO - implememnt CDC - headunit remote control
 
*/
const int SCK_pin = 13; // DIN Pin 2 - Clock
const int MOSI_pin = 11;  //DIN Pin 1 - Data
const int STR_pin = 8;  // DIM Pin 4 - Strobe
 
 
//dispData - 0xCB, <disc>, <track>, <min>, <sec>, <state>, <unknown>, 0xc3, (additional zero bytes for padding between packets)
//Can all be zeros apart from the 0xCB at the start and the 0xC3 at the end
// Note that you can substitute 0xC3 for 0xD3 and the head unit thinks an MD Changer is installed
byte dispData[] = { 0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0xC3,0x00,0x00,0x00,0x00 };
 
void setup()
{
       pinMode(SCK_pin, OUTPUT);
       pinMode(MOSI_pin, OUTPUT);
       pinMode(STR_pin, OUTPUT);
 
       digitalWrite(SCK_pin, HIGH);
       digitalWrite(MOSI_pin, HIGH);
       digitalWrite(STR_pin, HIGH);
}
 
void loop()
{     
       for (int z = 0; z <= sizeof(dispData); z++) {  //Loop through all the bytes in the byte array
              bitBang(dispData[z]); // Transmit (bitbang) the data
                    if ((z == 0)  || z == sizeof(dispData) ) {  //if we are after the first byte or after the last
                     digitalWrite(STR_pin, LOW);       //set strobe - it doesn't seem to matter if we have the extra padding bytes
              }
       }
}
 
 
byte bitBang(byte _send)  //Bitbang the data
//TODO: change for direct port manipulation (uC specific) - may not be needed though
// becuase it seems like it works even with the very slow speed of the digitalWrite
{
       digitalWrite(STR_pin, HIGH); //reset strobe if low
              for (int i = 0; i<8; i++)  // There are 8 bits in a byte
       {
              digitalWrite(MOSI_pin, !(bitRead(_send, 7-i)));    // Set Data - 7-i to send MSB first
              digitalWrite(SCK_pin, LOW);                  // SCK high
              digitalWrite(SCK_pin, HIGH);                   // SCK low
       }
      
}
