// const byte ledPin = 13;
// const byte interruptPin16 = 16;
// const byte interruptPin17 = 17;
volatile byte state = LOW;

void setup() {
  // pinMode(ledPin, OUTPUT);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(16), blink16, CHANGE);
  attachInterrupt(digitalPinToInterrupt(17), blink17, CHANGE);
}

void loop() {
  // digitalWrite(ledPin, state);
}

void blink16() {
  Serial.print("16 ");
  Serial.println(digitalRead(16));
}
void blink17() {
  Serial.print("17 ");
  Serial.println(digitalRead(17));
}
