/*
 * Detecteur de presence avec envoi de SMS
 * Auteur: Arnaud Morin
 */

#include <SoftwareSerial.h>

SoftwareSerial sim900(7, 8);    // SIM900 Tx/Rx connecte a Arduino 7/8
const byte detecteur = 2;       // PIN venant du detecteur de passage
const byte powerSim900 = 9 ;    // Pin pour allumer le SIM900
const byte ledPin = 13;         // Pin led interne, pour debug
volatile byte state = 0;        // Variable stockage etat passage
String numero = "33618671034";  // Numero a qui on envoi les SMS
String last_message = "";

/*
 * Setup
 */
void setup()
{
  // Attache une interrupton sur la pin du detecteur
  pinMode(detecteur, INPUT);
  pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(detecteur), alerte, RISING);
  
  // Debug monitor
  Serial.begin(9600);
  Serial.println("Demarrage...");
  
  // communication avec sim900
  sim900.begin(9600);

  // on verifie si le sim900 est deja allume
  while (checkpower() == 0){
    Serial.println("Allumage du module SIM...");
    powertoggle();
  }

  Serial.println("Module SIM OK...");

  // on se met en mode text (pour envoyer des SMS plus simplement)
  sim900.println("AT+CMGF=1");
  updateSerial();

  // Envoi d'un message pour dire qu'il est allume
  sendSMS(numero, "Coucou je suis le detecteur et je suis pret");
}

/*
 * Fonction executer quand passage devant detecteur
 */
void alerte(){
  state = 1;
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

void powertoggle()
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
 * Fonction pour verifier si le SIM900 est allume
 * Retourne 1 si allume
 * Retourne 0 si eteind
 */
byte checkpower()
{
  last_message = "";
  sim900.println("AT");
  updateSerial();
  delay(1000);
  sim900.println("AT");
  updateSerial();
  delay(1000);
  sim900.println("AT");
  updateSerial();
  if (last_message == "AT  OK"){
    return 1;
  }
  else {
    return 0;
  }
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
    last_message = sim900.readString();
    last_message.trim();
    last_message.replace("\n", " ");
    last_message.replace("\r", "");
    Serial.print("Recu du module SIM: ");
    Serial.println(last_message);
  }
}

/*
 * Boucle
 */
void loop()
{
  // Communication SMS
  updateSerial();
  digitalWrite(ledPin, state);
  if (state==1){
    Serial.println("Envoi SMS");
    sendSMS(numero, "Coucou, ya du monde chez nous!");
    state = 0;
  }
  //delay(500);
}
