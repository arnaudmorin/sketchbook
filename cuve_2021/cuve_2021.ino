#include <SoftwareSerial.h>

// Parametres
float cuve_pleine = 164;  // hauteur en cm quand la cuve est pleine
float volume_pleine = 4000 ; // volume cuve pleine
float capteur_pos = 170;    // hauteur en cm du capteur par rapport a 0 (cuve vide)

SoftwareSerial ultraSerial(11, 10); // RX(blanc), TX(jaune)
SoftwareSerial lcdSerial(3, 2);  // RX(unused), TX - See https://www.sparkfun.com/tutorials/246

void setup()
{
  Serial.begin(57600);
  ultraSerial.begin(9600);
  lcdSerial.begin(9600);
  delay(1000);            // wait for display to boot up
}

/*
   Fonction calcul volume cuve horizontale
   h = hauteur du liquide dans la cuve
   d = diametre de la cuve
*/
float volCuve(float h, float d) {
  //surfaceSecteur = 1/2*r**2*(2*alpha - sin(2*alpha))
  float r = 0.0;
  float alpha = 0.0;
  float volPourc = 0.0;
  r = d / 2;
  alpha = acos((r - h) / r);
  volPourc = ((pow(r, 2) / 2 * (2 * alpha - sin(2 * alpha))) / (3.14159 * pow(r, 2))) * 100.0;
  //ceinture et bretelles
  if (volPourc <= 0.0)
    volPourc = 0.0;
  else if (volPourc > 100.0)
    volPourc = 100.0;
  return volPourc;
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
  float volume;

  ultraSerial.listen();
  Serial.println("");

  // Discard/flush first bytes
  while (ultraSerial.available() > 0) {
    ultraSerial.read();
  }

  // Wait for data to be ready from sensor
  while (ultraSerial.available() == 0) {
    Serial.println("Waiting for data");
    delay(100);
  }

  Serial.println("Reading data");
  while (ultraSerial.available() > 0 && i < 4) {
    data[i] = ultraSerial.read();
    i++;
    if (data[0] != 0xff) {
      // We start over if the data first byte is not a header
      i = 0;
    }
  }

  Serial.println("Data received");
  // Being here mean we should have 4 bytes in data
  if (i != 4) {
    Serial.println("Error with data");
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
  pourcent = volCuve(capteur_pos - distance/10, cuve_pleine);
  volume = volume_pleine * pourcent / 100;   // en litre
  Serial.print("pourcentage=");
  Serial.println(pourcent);
  Serial.print("volume=");
  Serial.println(volume);

  lcdClear();
  lcdPosition(0, 0);

  // Affiche les valeurs
  lcdSerial.print(int(distance/10));
  lcdSerial.print(" cm / ");
  lcdSerial.print(int(pourcent));
  lcdSerial.print(" %");
  lcdPosition(1, 0);
  lcdSerial.print(volume);
  lcdSerial.print(" L");

  // Wait before next loop
  delay(1000);
}
