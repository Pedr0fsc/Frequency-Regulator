// Regulador de Frequência com Arduino UNO
// Potenciômetro controla frequência de buzzer (100Hz - 5000Hz)
// Frequência mostrada em 4 displays de 7 segmentos

// Definição dos pinos
const int potenciometro = A0;           // Potenciômetro
const int buzzer = A1;        // Buzzer passivo
const int botao = 9;          // Botão liga/desliga buzzer

// Pinos dos displays (cátodo comum)
const int display_unidade = 13;   // Display das unidades
const int display_dezena = 12;    // Display das dezenas  
const int display_centena = 11;   // Display das centenas
const int display_milhar = 10;    // Display dos milhares

// Pinos dos segmentos (A, B, C, D, E, F, G)
const int segmento_A = 2;
const int segmento_B = 3;
const int segmento_C = 4;
const int segmento_D = 5;
const int segmento_E = 6;
const int segmento_F = 7;
const int segmento_G = 8;

// Array com os pinos dos displays
int displays[] = {display_milhar, display_centena, display_dezena, display_unidade};

// Array com os pinos dos segmentos
int segmentos[] = {segmento_A, segmento_B, segmento_C, segmento_D, segmento_E, segmento_F, segmento_G};

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
  pinMode(buzzer, OUTPUT);
  
  // Configurar pino do botão (sem pull-up interno)
  pinMode(botao, INPUT);
}

void loop() {
  // Verificar botão liga/desliga
  verificarBotao();
  
  // Ler potenciômetro apenas a cada 50ms para estabilizar
  if(millis() - ultimaLeitura > 50) {
    int valorPot = analogRead(potenciometro);
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
      tone(buzzer, frequencia);
    } else {
      noTone(buzzer);
    }
    frequenciaAnterior = frequencia;
  }
  
  // Atualizar displays (multiplexação rápida)
  atualizarDisplays();
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

// Função para verificar o botão liga/desliga
void verificarBotao() {
  bool botaoAtual = digitalRead(botao);
  
  // Detecção simples: se botão mudou para HIGH e passou 300ms
  if(botaoAtual == HIGH && botaoAnterior == LOW) {
    if(millis() - ultimoPressionamento > 300) {
      buzzerLigado = !buzzerLigado;
      
      if(buzzerLigado) {
        tone(buzzer, frequencia);
      } else {
        noTone(buzzer);
      }
      
      ultimoPressionamento = millis();
    }
  }
  
  botaoAnterior = botaoAtual;
}