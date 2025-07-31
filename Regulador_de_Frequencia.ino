// Regulador de Frequência com Arduino UNO
// Potenciômetro controla frequência de buzzer (100Hz - 5000Hz)
// Frequência mostrada em 4 displays de 7 segmentos

// Definição dos pinos
const int POT_PIN = A0;           // Potenciômetro
const int BUZZER_PIN = A1;        // Buzzer passivo
const int BOTAO_PIN = 9;          // Botão liga/desliga buzzer

// Pinos dos displays (cátodo comum)
const int DISPLAY_UNIDADE = 13;   // Display das unidades
const int DISPLAY_DEZENA = 12;    // Display das dezenas  
const int DISPLAY_CENTENA = 11;   // Display das centenas
const int DISPLAY_MILHAR = 10;    // Display dos milhares

// Pinos dos segmentos (A, B, C, D, E, F, G)
const int SEGMENTO_A = 2;
const int SEGMENTO_B = 3;
const int SEGMENTO_C = 4;
const int SEGMENTO_D = 5;
const int SEGMENTO_E = 6;
const int SEGMENTO_F = 7;
const int SEGMENTO_G = 8;

// Array com os pinos dos displays
int displays[] = {DISPLAY_MILHAR, DISPLAY_CENTENA, DISPLAY_DEZENA, DISPLAY_UNIDADE};

// Array com os pinos dos segmentos
int segmentos[] = {SEGMENTO_A, SEGMENTO_B, SEGMENTO_C, SEGMENTO_D, SEGMENTO_E, SEGMENTO_F, SEGMENTO_G};

// Padrões para cada dígito (0-9) nos 7 segmentos
// Ordem: A, B, C, D, E, F, G (1 = aceso, 0 = apagado)
bool digitos[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

// Variáveis globais
int frequencia = 100;
int frequenciaAnterior = 0;
unsigned long ultimoTempo = 0;
unsigned long ultimaLeitura = 0;
int displayAtual = 0;

// Variáveis do botão
bool buzzerLigado = false;
bool botaoAnterior = false;
unsigned long ultimoPressionamento = 0;

void setup() {
  // Configurar pinos dos displays como saída
  for(int i = 0; i < 4; i++) {
    pinMode(displays[i], OUTPUT);
    digitalWrite(displays[i], HIGH); // Desliga todos os displays inicialmente
  }
  
  // Configurar pinos dos segmentos como saída
  for(int i = 0; i < 7; i++) {
    pinMode(segmentos[i], OUTPUT);
    digitalWrite(segmentos[i], LOW); // Apaga todos os segmentos inicialmente
  }
  
  // Configurar pino do buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Configurar pino do botão (sem pull-up interno)
  pinMode(BOTAO_PIN, INPUT);
  
  Serial.begin(9600); // Para debug (opcional)
}

void loop() {
  // Verificar botão liga/desliga
  verificarBotao();
  
  // Ler potenciômetro apenas a cada 50ms para estabilizar
  if(millis() - ultimaLeitura > 50) {
    int valorPot = analogRead(POT_PIN);
    int novaFrequencia = map(valorPot, 0, 1023, 100, 5000);
    
    // Aplicar filtro simples para evitar oscilações
    if(abs(novaFrequencia - frequencia) > 10) {
      frequencia = novaFrequencia;
    }
    
    ultimaLeitura = millis();
  }
  
  // Atualizar buzzer apenas quando frequência mudar E estiver ligado
  if(frequencia != frequenciaAnterior) {
    if(buzzerLigado) {
      tone(BUZZER_PIN, frequencia);
    } else {
      noTone(BUZZER_PIN);
    }
    frequenciaAnterior = frequencia;
  }
  
  // Atualizar displays (multiplexação rápida)
  atualizarDisplays();
  
  // Debug no monitor serial (opcional)
  if(millis() - ultimoTempo > 500) {
    Serial.print("Frequência: ");
    Serial.print(frequencia);
    Serial.print(" Hz - Buzzer: ");
    Serial.println(buzzerLigado ? "LIGADO" : "DESLIGADO");
    ultimoTempo = millis();
  }
}

void atualizarDisplays() {
  // Separar a frequência em dígitos individuais
  int milhar = frequencia / 1000;
  int centena = (frequencia % 1000) / 100;
  int dezena = (frequencia % 100) / 10;
  int unidade = frequencia % 10;
  
  // Array com os dígitos
  int digitos_freq[] = {milhar, centena, dezena, unidade};
  
  // Multiplexação - mostra um display por vez
  // Desliga todos os displays primeiro
  for(int i = 0; i < 4; i++) {
    digitalWrite(displays[i], HIGH);
  }
  
  // Apaga todos os segmentos
  for(int i = 0; i < 7; i++) {
    digitalWrite(segmentos[i], LOW);
  }
  
  // Verifica se deve mostrar o dígito (evita zeros à esquerda)
  bool mostrarDigito = false;
  
  if(displayAtual == 0 && milhar > 0) mostrarDigito = true;      // Milhar
  else if(displayAtual == 1 && (milhar > 0 || centena > 0)) mostrarDigito = true; // Centena
  else if(displayAtual == 2 && frequencia >= 10) mostrarDigito = true; // Dezena
  else if(displayAtual == 3) mostrarDigito = true; // Unidade (sempre mostra)
  
  if(mostrarDigito) {
    // Configurar segmentos para o dígito atual
    for(int j = 0; j < 7; j++) {
      digitalWrite(segmentos[j], digitos[digitos_freq[displayAtual]][j]);
    }
    
    // Liga o display atual
    digitalWrite(displays[displayAtual], LOW);
  }
  
  // Delay muito menor para multiplexação mais suave
  delayMicroseconds(500);
  
  // Próximo display
  displayAtual++;
  if(displayAtual >= 4) {
    displayAtual = 0;
  }
}

// Função para verificar o botão liga/desliga (VERSÃO SIMPLES)
void verificarBotao() {
  bool botaoAtual = digitalRead(BOTAO_PIN);
  
  // Debug simples
  static unsigned long ultimoDebug = 0;
  if(millis() - ultimoDebug > 500) {
    Serial.print("Pino 9: ");
    Serial.print(botaoAtual ? "HIGH (5V)" : "LOW (0V)");
    Serial.print(" | Buzzer: ");
    Serial.println(buzzerLigado ? "LIGADO" : "DESLIGADO");
    ultimoDebug = millis();
  }
  
  // Detecção simples: se botão mudou para HIGH e passou 300ms
  if(botaoAtual == HIGH && botaoAnterior == LOW) {
    if(millis() - ultimoPressionamento > 300) {
      buzzerLigado = !buzzerLigado;
      
      Serial.println("*** BOTÃO DETECTADO! ***");
      
      if(buzzerLigado) {
        tone(BUZZER_PIN, frequencia);
        Serial.println("Buzzer LIGADO");
      } else {
        noTone(BUZZER_PIN);
        Serial.println("Buzzer DESLIGADO");
      }
      
      ultimoPressionamento = millis();
    }
  }
  
  botaoAnterior = botaoAtual;
}