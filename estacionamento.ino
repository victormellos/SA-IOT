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

const bool DEBUG = true;                    // true = imprime leituras no Serial

const int ANGULO_FECHADO = 90;
const int ANGULO_ABERTO  = 180;

const int VAGAS_MAX = 9;
const int LIMITE_DIST = 60;                 // cm: menor que isso = carro
const int LIMITE_LUZ = 500;                 // acima disso = carro (calibre!)
const unsigned long CONFIRMA_ABRIR = 150;   // ms de detecção contínua antes de abrir
const unsigned long ESPERA_FECHAR = 2000;   // ms livre antes de fechar
const unsigned long TIMEOUT = 20000;        // ms: segurança da cancela aberta

// índice 0 = ENTRADA (ultrassônico), índice 1 = SAÍDA (LDR)
const int ENTRADA = 0;
const int SAIDA = 1;
const int DELTA[2] = {-1, +1};              // entrada diminui vagas, saída aumenta
const char *NOMES[2] = {"Entrada", "Saida"};

Servo servos[2];
bool aberta[2]            = {false, false};
bool pronta[2]            = {false, false}; // sensor já foi visto livre?
unsigned long ocupadoDesde[2] = {0, 0};
unsigned long tAbriu[2]       = {0, 0};
unsigned long livreDesde[2]   = {0, 0};

int vagas = VAGAS_MAX;

long lerDistanciaCm() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long d = pulseIn(ECHO, HIGH, 30000);
  if (d == 0) return 999;                   // sem eco = sem carro
  return d * 0.034 / 2;
}

// média de 4 leituras para reduzir ruído
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

// i = ENTRADA ou SAIDA
void atualizaCancela(int i, bool ocupado, bool podeAbrir) {
  unsigned long agora = millis();

  if (!aberta[i]) {
    if (!ocupado) {
      pronta[i] = true;                     // viu livre: pode disparar de novo
      ocupadoDesde[i] = 0;
    } else if (pronta[i] && podeAbrir) {
      if (ocupadoDesde[i] == 0) ocupadoDesde[i] = agora;
      if (agora - ocupadoDesde[i] >= CONFIRMA_ABRIR) {
        pronta[i] = false;
        ocupadoDesde[i] = 0;
        servos[i].write(ANGULO_ABERTO);
        aberta[i] = true;
        tAbriu[i] = agora;
        livreDesde[i] = 0;
        Serial.print(NOMES[i]);
        Serial.println(": abriu");
      }
    }
  }
  else {
    if (ocupado) {
      livreDesde[i] = 0;                    // carro ainda passando
    } else {
      if (livreDesde[i] == 0) livreDesde[i] = agora;
      if (agora - livreDesde[i] >= ESPERA_FECHAR) {
        // carro passou: fecha e conta
        servos[i].write(ANGULO_FECHADO);
        aberta[i] = false;
        vagas = constrain(vagas + DELTA[i], 0, VAGAS_MAX);
        Serial.print(NOMES[i]);
        Serial.println(": fechou");
        atualizaDisplay();
        return;
      }
    }

    // sensor preso: fecha sem contar
    if (agora - tAbriu[i] > TIMEOUT) {
      servos[i].write(ANGULO_FECHADO);
      aberta[i] = false;
      Serial.print(NOMES[i]);
      Serial.println(": sensor travado, fechou sem contar");
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  for (int i = 0; i < 7; i++) pinMode(SEG[i], OUTPUT);

  servos[ENTRADA].attach(SERVO_ENTRADA_PIN);
  servos[SAIDA].attach(SERVO_SAIDA_PIN);
  servos[ENTRADA].write(ANGULO_FECHADO);
  servos[SAIDA].write(ANGULO_FECHADO);

  atualizaDisplay();
}

void loop() {
  long dist = lerDistanciaCm();
  int luz = lerLuz();

  bool carroEntrando = (dist < LIMITE_DIST);
  bool carroSaindo   = (luz  < LIMITE_LUZ);

  if (DEBUG) {
    static unsigned long ultimo = 0;
    if (millis() - ultimo > 500) {
      ultimo = millis();
      Serial.print("Dist: ");
      Serial.print(dist);
      Serial.print(" cm | LDR: ");
      Serial.println(luz);
    }
  }

  // ENTRADA (ultrassônico): só abre se houver vaga
  atualizaCancela(ENTRADA, carroEntrando, vagas > 0);

  // SAÍDA (LDR): sempre abre, mas o contador nunca passa de 9
  atualizaCancela(SAIDA, carroSaindo, true);

  delay(20);
}