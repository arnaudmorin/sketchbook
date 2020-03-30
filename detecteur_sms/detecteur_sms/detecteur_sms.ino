/*
 * Detecteur de presence avec envoi de SMS
 * Auteur: Arnaud Morin
 */

#include <SoftwareSerial.h>

SoftwareSerial sim900(7, 8); //SIM900 Tx/Rx connecte a Arduino 7/8
int powerSim900 = 9 ;   // Pin pour allumer le SIM900
String numero = "33645360584";

/*
 * Setup
 */
void setup()
{
  // Debug monitor
  Serial.begin(9600);
  
  // communication avec sim900
  sim900.begin(9600);

  Serial.println("Initialisation...");
  //sim900power();

  // Verification si sim900 OK
  sim900.println("AT");
  updateSerial();

  sim900.println("AT+CMGF=1"); // TEXT mode
  updateSerial();

  // Envoi d'un message pour dire qu'il est allume
  //sendSMS(numero, "Coucou je suis le detecteur et je suis pret");
}

/*
 * Fonction pour envoyer un message
 */
void sendSMS(String numero, String message)
{
  sim900.println("AT+CMGS=\"+" + numero + "\"");
  updateSerial();
  sim900.print(message);
  updateSerial();
  sim900.write(26);
}

/*
 * Fonction pour allumer le SIM900
 */
void sim900power()
{
  pinMode(powerSim900, OUTPUT); 
  digitalWrite(powerSim900,LOW);
  delay(1000);
  digitalWrite(powerSim900,HIGH);
  delay(2000);
  digitalWrite(powerSim900,LOW);
  delay(3000);
}

/*
 * Affiche sur le debug serial ce que dit le SIM900
 * Aussi permet d'envoyer des commandes au SIM900
 */
void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    String s_text = Serial.readString();
    s_text.trim();
    // Si le serial contient test alors on essai l'envoi d'un sms
    if (s_text.startsWith("test")) {
      sendSMS(numero, s_text);
    }
    // Si le serial contient une commande AT, on la forward au SIM900
    if (s_text.startsWith("AT")) {
      // Forward what Serial received to Software Serial Port
      sim900.println(s_text);
    }
  }
  while(sim900.available())
  {
    // On print ce que dit le SIM900
    Serial.write(sim900.read());
  }
}

/*
 * Boucle
 */
void loop()
{
  updateSerial();
}
