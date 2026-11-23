#include <Servo.h>

const int TRIG = 12;
const int ECHO = 13;
const int LDR_PIN = A0;
const int SERVO_ENTRADA_PIN = 11;
const int SERVO_SAIDA_PIN = 10;

// segmentos a, b, c, d, e, f, g
const byte SEG[7] = {2, 3, 4, 5, 8, 6, 7};

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

const int ANGULO_FECHADO = 90;
const int ANGULO_ABERTO  = 180;
const int VAGAS_MAX = 9;
const int LIMITE_DIST = 60;         // cm
const int LIMITE_LUZ = 400;         // calibrar!

Servo servoEntrada;
Servo servoSaida;
int vagas = VAGAS_MAX;

long lerDistanciaCm() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long d = pulseIn(ECHO, HIGH, 30000);
  if (d == 0) return 999;
  return d * 0.034 / 2;
}

void mostraNumero(int n) {
  n = constrain(n, 0, 9);
  for (int i = 0; i < 7; i++) {
    digitalWrite(SEG[i], DIGITOS[n][i]);
  }
}

void atualizaDisplay() {
  mostraNumero(vagas);
  Serial.print("Vagas livres: ");
  Serial.println(vagas);
}

void abreEntrada() {
  servoEntrada.write(ANGULO_ABERTO);
  Serial.println("Entrada: abriu");
  delay(3000);
  servoEntrada.write(ANGULO_FECHADO);
  Serial.println("Entrada: fechou");
  vagas--;
  atualizaDisplay();
}

void abreSaida() {
  servoSaida.write(ANGULO_ABERTO);
  Serial.println("Saida: abriu");
  delay(3000);
  servoSaida.write(ANGULO_FECHADO);
  Serial.println("Saida: fechou");
  if (vagas < VAGAS_MAX) vagas++;
  atualizaDisplay();
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  for (int i = 0; i < 7; i++) pinMode(SEG[i], OUTPUT);

  servoEntrada.attach(SERVO_ENTRADA_PIN);
  servoSaida.attach(SERVO_SAIDA_PIN);
  servoEntrada.write(ANGULO_FECHADO);
  servoSaida.write(ANGULO_FECHADO);

  atualizaDisplay();
}

void loop() {
  long dist = lerDistanciaCm();
  int luz = analogRead(LDR_PIN);

  // ENTRADA
  if (dist < LIMITE_DIST && vagas > 0) {
    abreEntrada();
  }

  // SAÍDA
  if (luz < LIMITE_LUZ) {
    abreSaida();
  }

  delay(50);
}
