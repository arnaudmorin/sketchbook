

const int CLOCK = 11;  // DIN Pin 2 - Clock
const int DATA = 3;    // DIN Pin 1 - Data
const int STROBE = 2;  // DIM Pin 4 - Strob

// dispData - 0xcb, <disc>, <track>, <min>, <sec>, <state>, <unknown>, 0xc3
// Can all be zeros apart from the 0xcb at the start and the 0xc3 at the end
byte bytes[] = { 0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0xc3,0x00,0x00,0x00,0x00 };
 

void setup(){
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(STROBE, OUTPUT);
  digitalWrite(CLOCK, HIGH);
  digitalWrite(DATA, HIGH);
  digitalWrite(STROBE, HIGH);

  cli();      // disable all interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  // f(hz) = 1/T(s)
  // 1us = 0.000001s = 1/1000000s
  // f = 1/0.000001 = 1000000 = 10^6 = 1Mhz
  // donc 1us ==> 1Mhz
  // Je veux une clock de 8us (4us on / 4us off)
  // f = 1/0.000004 = 250khz
  // La clock de larduino cest 16Mhz
  // On met un prescaler de 8
  // donc, le tick prendre 8/16000000 sec
  // soit 0.5us
  // Nous on veut que l'interrupt arrive tous les 4us
  // soit 8 ticks, or timer2 est sur 8 bits
  // il peut donc compter theoriquement jusqu'a 256.
  // en mode CTC on va lui dire de s'arreter de compter avant
  // OCR2A cest le registre qui contient la valeur a laquelle
  // on doit comparer justement
  OCR2A = 2;
  // par contre, a priori une interrupt prend jusqu'a 4us a etre traitee
  // donc bon, jarrive pas a descendre en dessous de 16us par cycle
  // turn on CTC mode (Clear Timer on Compare Match Mode)
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);
  // enable timer compare interrupt
  // parce qu'on utilise CTC, il nous faut activer cet interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();             // enable all interrupts
}

ISR(TIMER2_COMPA_vect){         // timer compare interrupt service routine
  static boolean clock_state = HIGH;
  static int c_bytes = 0;
  static int c_bit = 0;
  
  if (clock_state == HIGH){
    PORTB = PORTB | B00001000;  // pin 11 is pb3 on chip
  } else {
    PORTB = PORTB & B11110111;
  }
  
  if (!(bitRead(bytes[c_bytes], 7-c_bit)) == HIGH) {
    PORTD = PORTD | B00001000;  // pin 3 in pd3 on chip
  } else {
    PORTD = PORTD & B11110111;
  }
  //digitalWrite(DATA, !(bitRead(bytes[c_bytes], 7-c_bit)));    // Set Data = 7-c_bit to send MSB first
  
  // Strobe?
  if (((c_bytes == 1) && (c_bit == 0)) || ((c_bytes == 8) && (c_bit == 0))){ // After first byte and after last byte (before padding)
    PORTD = PORTD & B11111011;  // pin 2 in pd2 on chip
  }
  else{
    PORTD = PORTD | B00000100;
  }
  clock_state = !clock_state;
}

// the loop function runs over and over again forever
void loop() {

}
