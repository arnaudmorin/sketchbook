// Pour le LCD
#include <SoftwareSerial.h>

// Les variables
float cuve_pleine = 125;  // hauteur en cm quand la cuve est pleine
float capteur_pos = 151;        // hauteur en cm du capteur
long echo_lu;             // le temps aller/retour de l'impulsion en us
long cm;                  // la hauteur d'eau mesuree
float pourcent;           // la hauteur en pourcentage
float volume;             // le volume d'eau
char string[3];           // string array pour le LCD


SoftwareSerial ultraSerial(8, 9); // RX, TX
SoftwareSerial lcdSerial(3, 2);  // RX(unused), TX - See https://www.sparkfun.com/tutorials/246

void setup()
{
  Serial.begin(57600);
  ultraSerial.begin(9600);
  lcdSerial.begin(9600);
}

/*
 * Move cursor on LCD
 */
void lcdPosition(int row, int col) {
  lcdSerial.write(0xFE); //command flag
  lcdSerial.write((col + row*64 + 128));
}

/*
 * Clear LCD
 */
void lcdClear() {
  lcdSerial.write(0xFE); //command flag
  lcdSerial.write(0x01); //clear command.
  delay(10);
}

void loop()
{
  unsigned char data[4] = {};
  int i = 0 ;
  float distance;
  float pourcent;

  ultraSerial.listen();

  // Discard/flush first bytes
  while (ultraSerial.available() > 0) {
    ultraSerial.read();
  }

  // Wait for data to be ready from sensor
  while (ultraSerial.available() == 0) {
    Serial.println("Waiting for data");
    delay(100);
  }

  while (ultraSerial.available() > 0 && i < 4) {
    data[i] = ultraSerial.read();
    i++;
    if (data[0] != 0xff) {
      // We start over if the data first byte is not a header
      i = 0;
    }
  }

  // Being here mean we should have 4 bytes in data
  if (i != 4) {
    Serial.println("Error reading sensor data");
  }
  else {
    // Verify checksum
    int sum;
    sum = (data[0] + data[1] + data[2]) & 0x00FF;
    if (sum == data[3]) {
      // Checksum is good - compute distance
      distance = (data[1] << 8) + data[2];
      Serial.print("distance=");
      Serial.print(distance / 10);
      Serial.println("cm");
    } else {
      Serial.println("Error wrong checksum");
    }
  }

  // Calculs
  pourcent = (capteur_pos - distance/10) * 100 / cuve_pleine;
  Serial.print("pourcentage=");
  Serial.println(pourcent);

  lcdClear();
  lcdPosition(0, 0);

  // Affiche les valeurs
  lcdSerial.print("Hauteur cuve :  ");
  lcdPosition(1, 0);
  lcdSerial.print(int(distance/10));
  lcdSerial.print(" cm / ");
  lcdSerial.print(int(pourcent));
  lcdSerial.print(" %");

  // Wait before next loop
  delay(1000);
}
