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
const int LIMITE_DIST = 60;                 // cm
const int LIMITE_LUZ = 500;                 // calibrar!
const unsigned long ESPERA_FECHAR = 2000;   // ms livre antes de fechar
const unsigned long TIMEOUT = 20000;        // ms

Servo servoEntrada;
Servo servoSaida;

int vagas = VAGAS_MAX;

// estado da ENTRADA
bool entradaAberta = false;
unsigned long entradaAbriu = 0;
unsigned long entradaLivreDesde = 0;

// estado da SAÍDA
bool saidaAberta = false;
unsigned long saidaAbriu = 0;
unsigned long saidaLivreDesde = 0;

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

int lerLuz() {
  long soma = 0;
  for (int i = 0; i < 4; i++) soma += analogRead(LDR_PIN);
  return soma / 4;
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

void atualizaEntrada(bool carro) {
  unsigned long agora = millis();

  if (!entradaAberta) {
    if (carro && vagas > 0) {
      servoEntrada.write(ANGULO_ABERTO);
      entradaAberta = true;
      entradaAbriu = agora;
      entradaLivreDesde = 0;
      Serial.println("Entrada: abriu");
    }
  } else {
    if (carro) {
      entradaLivreDesde = 0;
    } else {
      if (entradaLivreDesde == 0) entradaLivreDesde = agora;
      if (agora - entradaLivreDesde >= ESPERA_FECHAR) {
        servoEntrada.write(ANGULO_FECHADO);
        entradaAberta = false;
        vagas = constrain(vagas - 1, 0, VAGAS_MAX);
        Serial.println("Entrada: fechou");
        atualizaDisplay();
        return;
      }
    }
    if (agora - entradaAbriu > TIMEOUT) {
      servoEntrada.write(ANGULO_FECHADO);
      entradaAberta = false;
      Serial.println("Entrada: sensor travado, fechou sem contar");
    }
  }
}

void atualizaSaida(bool carro) {
  unsigned long agora = millis();

  if (!saidaAberta) {
    if (carro) {
      servoSaida.write(ANGULO_ABERTO);
      saidaAberta = true;
      saidaAbriu = agora;
      saidaLivreDesde = 0;
      Serial.println("Saida: abriu");
    }
  } else {
    if (carro) {
      saidaLivreDesde = 0;
    } else {
      if (saidaLivreDesde == 0) saidaLivreDesde = agora;
      if (agora - saidaLivreDesde >= ESPERA_FECHAR) {
        servoSaida.write(ANGULO_FECHADO);
        saidaAberta = false;
        vagas = constrain(vagas + 1, 0, VAGAS_MAX);
        Serial.println("Saida: fechou");
        atualizaDisplay();
        return;
      }
    }
    if (agora - saidaAbriu > TIMEOUT) {
      servoSaida.write(ANGULO_FECHADO);
      saidaAberta = false;
      Serial.println("Saida: sensor travado, fechou sem contar");
    }
  }
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
  int luz = lerLuz();

  atualizaEntrada(dist < LIMITE_DIST);
  atualizaSaida(luz < LIMITE_LUZ);

  delay(20);
}
