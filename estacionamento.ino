#include <Servo.h>

const int TRIG = 12;
const int ECHO = 13;
const int SERVO_PIN = 11;

// segmentos a, b, c, d, e, f, g
const byte SEG[7] = {2, 3, 4, 5, 6, 7, 8};

const byte DIGITOS[10][7] = {
  {1,1,1,1,1,1,0},  // 0
  {0,1,1,0,0,0,0},  // 1
  {1,1,0,1,1,0,1},  // 2
  {1,1,1,1,0,0,1},  // 3
  {0,1,1,0,0,1,1},  // 4
  {1,0,1,1,0,1,1},  // 5
  {1,0,1,1,1,1,1},  // 6
  {1,1,1,0,0,0,0},  // 7
  {1,1,1,1,1,1,1},  // 8
  {1,1,1,1,0,1,1}   // 9
};

const int ANGULO_FECHADO = 0;
const int ANGULO_ABERTO  = 90;
const int VAGAS_MAX = 9;
const int LIMITE_DIST = 20;         // cm

Servo cancela;
int vagas = VAGAS_MAX;

long lerDistanciaCm() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long d = pulseIn(ECHO, HIGH);
  return d * 0.034 / 2;
}

void mostraNumero(int n) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(SEG[i], DIGITOS[n][i]);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  for (int i = 0; i < 7; i++) pinMode(SEG[i], OUTPUT);

  cancela.attach(SERVO_PIN);
  cancela.write(ANGULO_FECHADO);
  mostraNumero(vagas);
}

void loop() {
  long dist = lerDistanciaCm();

  if (dist < LIMITE_DIST) {
    if (vagas > 0) {
      cancela.write(ANGULO_ABERTO);
      vagas--;
      mostraNumero(vagas);
      Serial.print("Vagas: ");
      Serial.println(vagas);
      delay(3000);
      cancela.write(ANGULO_FECHADO);
      delay(1000);
    } else {
      Serial.println("Estacionamento lotado");
      delay(1000);
    }
  }

  delay(100);
}
