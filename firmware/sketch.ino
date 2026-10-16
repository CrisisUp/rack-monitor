#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ==========================================
// CONFIGURAÇÃO DE REDE E SERVIDOR
// ==========================================
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// URL do Cloudflare Tunnel (muda a cada reinicio do cloudflared)
const char* SERVIDOR_URL = "https://potentially-sin-hop-syntax.trycloudflare.com";

// Intervalos de comunicação
constexpr unsigned long INTERVALO_ENVIO_STATUS_MS = 20000;   // 20s
constexpr unsigned long INTERVALO_BUSCA_CMD_MS    = 15000;   // 15s

// Fallback AP
constexpr bool HABILITAR_FALLBACK_AP = true;
const char* AP_SSID     = "RackMonitor";
const char* AP_PASSWORD = "rack1234";

// ==========================================
// DEFINIÇÃO DOS PINOS
// ==========================================
namespace Pins {
  constexpr uint8_t RFID_SS   = 5;
  constexpr uint8_t RFID_RST  = 4;
  constexpr uint8_t RFID_SCK  = 18;
  constexpr uint8_t RFID_MISO = 19;
  constexpr uint8_t RFID_MOSI = 23;

  constexpr uint8_t PIR       = 27;
  constexpr uint8_t TRIG      = 17;
  constexpr uint8_t ECHO      = 16;
  constexpr uint8_t LDR_AO    = 34;

  constexpr uint8_t BOTAO     = 14;
  constexpr uint8_t SWITCH    = 25;

  constexpr uint8_t BUZZER_E  = 15;
  constexpr uint8_t BUZZER_D  = 2;

  constexpr uint8_t LED_R     = 32;
  constexpr uint8_t LED_G     = 33;
  constexpr uint8_t LED_B     = 0;
}

// ==========================================
// CONSTANTES GERAIS
// ==========================================
constexpr uint32_t PWM_FREQ       = 5000;
constexpr uint8_t  PWM_RESOLUTION = 8;
constexpr uint8_t  LARGURA_TELA   = 128;
constexpr uint8_t  ALTURA_TELA    = 64;
constexpr uint8_t  ENDERECO_OLED  = 0x3C;

// ==========================================
// FILTROS
// ==========================================
constexpr int  LDR_LIMIAR_ACENDE = 2200;
constexpr int  LDR_LIMIAR_APAGA  = 1800;
constexpr unsigned long DEBOUNCE_LDR_MS = 800;

constexpr int   US_AMOSTRAS        = 5;
constexpr float US_DIST_MAX_VALIDA = 400.0f;

constexpr unsigned long DEBOUNCE_ESTADO_MS = 300;

constexpr unsigned long DEBOUNCE_BOTAO_MS       = 250;
constexpr unsigned long JANELA_CLIQUE_DUPLO_MS  = 400;
constexpr unsigned long DURACAO_CLIQUE_LONGO_MS = 3000;

constexpr unsigned long DEBOUNCE_PORTA_MS = 200;

// ==========================================
// CARTÕES AUTORIZADOS
// ==========================================
const String CARTAO_AUTORIZADO_1 = "01020304";
const String CARTAO_AUTORIZADO_2 = "55667788";

// ==========================================
// INSTÂNCIAS
// ==========================================
Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, -1);
RTC_DS3231 rtc;
MFRC522 rfid(Pins::RFID_SS, Pins::RFID_RST);

// ==========================================
// ENUMS
// ==========================================
enum ModoVigilante { ARMADO, DESARMADO };

enum EstadoSistema {
  SEGURO,
  AVISO_SALA,
  APROXIMACAO_RACK,
  EMERGENCIA_RACK,
  INVASAO_SALA,
  ACESSO_RFID
};

// ==========================================
// ESTADO GLOBAL — MODO VIGILANTE
// ==========================================
ModoVigilante modoVigilante = ARMADO;
unsigned long timeoutDesarme       = 30UL * 60UL * 1000UL;
unsigned long tempoInicioDesarme   = 0;
bool          aviso5minDisparado   = false;
String        uidUltimoDesarme     = "";
String        uidUltimoRearme      = "";
unsigned int  contadorDesarmes     = 0;
unsigned long duracaoUltimoDesarme = 0;
unsigned int  contadorRearmesBloqueados = 0;

// ==========================================
// ESTADO GLOBAL — MÁQUINA DE ALARME
// ==========================================
EstadoSistema estadoAtual    = SEGURO;
EstadoSistema estadoAnterior = SEGURO;
EstadoSistema estadoCandidato      = SEGURO;
unsigned long tempoEstadoCandidato = 0;

// ==========================================
// ESTADO GLOBAL — FILTROS
// ==========================================
unsigned long tempoUltimaMudancaLDR = 0;
bool          estadoLuzFiltrado     = false;

bool          estadoPortaFiltrado     = false;
bool          portaBrutaAnterior      = false;
unsigned long tempoUltimaMudancaPorta = 0;

// ==========================================
// ESTADO GLOBAL — SISTEMA
// ==========================================
bool rtcDisponivel    = false;
bool cartaoAutorizado = false;

unsigned long tempoUltimoBip   = 0;
unsigned long tempoUltimoPisca = 0;
unsigned long tempoUltimoRFID  = 0;
bool          estadoPiscaLED   = false;

bool estadoAnteriorPIR = false;
bool estadoAnteriorLDR = false;

// ==========================================
// ESTADO GLOBAL — BOTÃO
// ==========================================
bool          botaoBrutoAnterior      = false;
bool          botaoFiltrado           = false;
unsigned long tempoUltimaMudancaBotao = 0;
unsigned long tempoBotaoPressionado   = 0;
int           cliquesBotao            = 0;
unsigned long tempoUltimoClique       = 0;
unsigned long tempoUltimaAcaoBotao    = 0;

// ==========================================
// ESTADO GLOBAL — REDE E SERVIDOR
// ==========================================
bool          wifiConectado   = false;
bool          modoAP          = false;
bool          servidorOnline  = false;

unsigned long tempoUltimoEnvioStatus = 0;
unsigned long tempoUltimaBuscaCmd    = 0;
int           ultimoCodigoHTTP       = 0;

unsigned int  contadorEnviosOk      = 0;
unsigned int  contadorEnviosFalha   = 0;
unsigned int  contadorCmdsRecebidos = 0;

// ==========================================
// ESTADO GLOBAL — OPERACIONAL
// ==========================================
float  limiarDistancia = 50.0f;
String ultimoUIDLido   = "";

unsigned int contadorAberturasPorta       = 0;
unsigned int contadorInvasoes             = 0;
unsigned int contadorAproximacoes         = 0;
unsigned int contadorAcessosValidos       = 0;
unsigned int contadorMovimentosManutencao = 0;

// ==========================================
// AVISO DE BLOQUEIO
// ==========================================
String        motivoBloqueio     = "";
unsigned long tempoBloqueioAtivo = 0;
constexpr unsigned long DURACAO_AVISO_BLOQUEIO_MS = 4000;

String bufferSerial = "";

// ==========================================
// PROTÓTIPOS
// ==========================================
void logSerial(const String &nivel, const String &tag, const String &mensagem);
String obterTimestamp();
String nomeEstado(EstadoSistema estado);
String nomeModo(ModoVigilante modo);
void definirCorLED(int r, int g, int b);
void atualizarLED();
float lerDistanciaBruta();
float lerDistanciaFiltrada();
bool  lerLDRFiltrado();
bool  lerPortaFiltrada();
bool  verificarRFID();
bool  isCartaoValido(const String &uid);
void  exibirTelaStatus();
void  exibirTelaDesarmado();
void  exibirTelaBloqueio();
void  desenharIcone(int tipo, int x, int y);
void  tocarMelodia(EstadoSistema estado);
void  pararBuzzers();
void  processarComandoSerial(String cmd);
void  alternarModoVigilante();
void  rearmarPorTimeout();
void  processarBotao();
void  executarModoTeste();
bool  tentarArmar();

// Rede
void iniciarWiFi();
void enviarStatusParaServidor();
void buscarComandosDoServidor();
void executarDiagnostico();
void testarHTTPS();

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(Pins::PIR, INPUT);
  pinMode(Pins::TRIG, OUTPUT);
  pinMode(Pins::ECHO, INPUT);
  pinMode(Pins::LDR_AO, INPUT);
  pinMode(Pins::BUZZER_E, OUTPUT);
  pinMode(Pins::BUZZER_D, OUTPUT);
  pinMode(Pins::BOTAO, INPUT_PULLUP);
  pinMode(Pins::SWITCH, INPUT_PULLDOWN);

  SPI.begin();
  rfid.PCD_Init();

  ledcAttach(Pins::LED_R, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(Pins::LED_G, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(Pins::LED_B, PWM_FREQ, PWM_RESOLUTION);

  definirCorLED(255, 0, 0); delay(150);
  definirCorLED(0, 255, 0); delay(150);
  definirCorLED(0, 0, 255); delay(150);
  definirCorLED(0, 0, 0);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, ENDERECO_OLED)) {
    Serial.println(F("[ERRO] Falha ao inicializar SSD1306"));
  }
  display.clearDisplay();
  display.display();

  if (rtc.begin()) {
    rtcDisponivel = true;
    if (rtc.lostPower()) {
      Serial.println(F("[WARN] RTC perdeu energia. Ajustando para hora de compilacao."));
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }

  Serial.println();
  Serial.println(F("================================================="));
  Serial.println(F("  SISTEMA DE MONITORAMENTO DE RACK - v7.2"));
  Serial.println(F("  Vigilante + RFID + Filtros + RTC + Cloudflare"));
  Serial.println(F("================================================="));
  logSerial("INFO", "SISTEMA", "Sistema inicializado. MODO VIGILANTE: ARMADO.");
  logSerial("INFO", "RFID", "Cartao 1 Autorizado: " + CARTAO_AUTORIZADO_1);
  logSerial("INFO", "RFID", "Cartao 2 Autorizado: " + CARTAO_AUTORIZADO_2);
  logSerial("INFO", "RTC", rtcDisponivel ? "RTC DS3231 ativo." : "RTC ausente (usando millis).");
  logSerial("INFO", "VIGILANTE", "Timeout de desarme: " + String(timeoutDesarme / 60000) + " min");
  logSerial("INFO", "VIGILANTE", "Politica: rearme BLOQUEADO se porta estiver aberta.");
  logSerial("INFO", "FILTROS",
    "LDR: " + String(LDR_LIMIAR_APAGA) + "-" + String(LDR_LIMIAR_ACENDE) +
    " deb " + String(DEBOUNCE_LDR_MS) + "ms | US media: " + String(US_AMOSTRAS));
  logSerial("INFO", "FILTROS",
    "Botao: deb " + String(DEBOUNCE_BOTAO_MS) + "ms | duplo <" +
    String(JANELA_CLIQUE_DUPLO_MS) + "ms | longo >" + String(DURACAO_CLIQUE_LONGO_MS) + "ms");
  logSerial("INFO", "FILTROS", "Porta: deb " + String(DEBOUNCE_PORTA_MS) + "ms");
  logSerial("INFO", "SRV", "Servidor remoto: " + String(SERVIDOR_URL));
  logSerial("INFO", "SISTEMA", "Digite HELP no Serial para comandos disponiveis.");
  Serial.println(F("-------------------------------------------------"));

  iniciarWiFi();

  exibirTelaStatus();
}

// ==========================================
// LOOP PRINCIPAL
// ==========================================
void loop() {
  // 1. Comunicação com servidor
  enviarStatusParaServidor();
  buscarComandosDoServidor();

  // 2. Comandos Serial
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (bufferSerial.length() > 0) {
        processarComandoSerial(bufferSerial);
        bufferSerial = "";
      }
    } else {
      bufferSerial += c;
    }
  }

  // 3. Sensores
  bool  portaAberta        = lerPortaFiltrada();
  bool  movimentoDetectado = (digitalRead(Pins::PIR) == HIGH);
  bool  luzAcesa           = lerLDRFiltrado();
  float distancia          = lerDistanciaFiltrada();

  // 4. RFID
  bool rfidDetectado = verificarRFID();

  // 5. Botão
  processarBotao();

  // 6. Detecção de borda para logs
  if (movimentoDetectado != estadoAnteriorPIR) {
    if (modoVigilante == DESARMADO && movimentoDetectado) {
      contadorMovimentosManutencao++;
      logSerial("INFO", "MANUT", "Movimento em modo manutencao (auditoria).");
    } else {
      logSerial("INFO", "SENSOR", movimentoDetectado ? "Movimento detectado pelo PIR." : "Movimento cessou.");
    }
    estadoAnteriorPIR = movimentoDetectado;
  }

  if (luzAcesa != estadoAnteriorLDR) {
    logSerial("INFO", "SENSOR", luzAcesa ? "Luzes da sala ACESAS." : "Luzes da sala APAGADAS.");
    estadoAnteriorLDR = luzAcesa;
  }

  static bool portaEstadoAnterior = false;
  if (portaAberta != portaEstadoAnterior) {
    if (portaAberta) {
      contadorAberturasPorta++;
      if (modoVigilante == ARMADO) {
        logSerial("ALERT", "PORTA", "!!! PORTA ABERTA (total: " + String(contadorAberturasPorta) + ") !!!");
      } else {
        logSerial("INFO", "PORTA", "Porta aberta em modo manutencao (total: " + String(contadorAberturasPorta) + ")");
      }
    } else {
      logSerial("INFO", "PORTA", "Porta fechada.");
    }
    portaEstadoAnterior = portaAberta;
  }

  // 7. Timeout do desarme
  if (modoVigilante == DESARMADO) {
    unsigned long decorrido = millis() - tempoInicioDesarme;
    unsigned long restante  = (timeoutDesarme > decorrido) ? (timeoutDesarme - decorrido) : 0;

    if (!aviso5minDisparado && restante <= 5UL * 60UL * 1000UL && timeoutDesarme > 5UL * 60UL * 1000UL) {
      aviso5minDisparado = true;
      logSerial("WARN", "VIGILANTE", "AVISO: 5 minutos para rearme automatico!");
      tone(Pins::BUZZER_E, 1500, 200);
    }

    if (decorrido >= timeoutDesarme) {
      rearmarPorTimeout();
    }
  }

  // 8. Aviso de bloqueio expira
  if (motivoBloqueio.length() > 0 &&
      (millis() - tempoBloqueioAtivo) > DURACAO_AVISO_BLOQUEIO_MS) {
    motivoBloqueio = "";
  }

  // 9. Máquina de estados
  if (modoVigilante == ARMADO) {
    EstadoSistema candidato;
    if (rfidDetectado || (millis() - tempoUltimoRFID < 2500)) {
      candidato = ACESSO_RFID;
    } else if (portaAberta) {
      candidato = EMERGENCIA_RACK;
    } else if (movimentoDetectado && luzAcesa) {
      candidato = INVASAO_SALA;
    } else if (movimentoDetectado && !luzAcesa) {
      candidato = AVISO_SALA;
    } else if (distancia < limiarDistancia && distancia > 0) {
      candidato = APROXIMACAO_RACK;
    } else {
      candidato = SEGURO;
    }

    bool critico = (candidato == EMERGENCIA_RACK || candidato == INVASAO_SALA);
    bool urgente = (candidato == ACESSO_RFID);

    if (critico || urgente) {
      estadoAtual          = candidato;
      estadoCandidato      = candidato;
      tempoEstadoCandidato = millis();
    } else {
      if (candidato != estadoCandidato) {
        estadoCandidato      = candidato;
        tempoEstadoCandidato = millis();
      } else if (candidato != estadoAtual &&
                 (millis() - tempoEstadoCandidato >= DEBOUNCE_ESTADO_MS)) {
        estadoAtual = candidato;
      }
    }

    if (estadoAtual != estadoAnterior) {
      String nivel = "INFO";
      if (estadoAtual == EMERGENCIA_RACK || estadoAtual == INVASAO_SALA) nivel = "CRIT";
      else if (estadoAtual == APROXIMACAO_RACK) nivel = "ALERT";
      else if (estadoAtual == AVISO_SALA)       nivel = "WARN";

      logSerial(nivel, "ESTADO",
        "Transicao: " + nomeEstado(estadoAnterior) + " -> " + nomeEstado(estadoAtual));

      if (estadoAtual == INVASAO_SALA)     contadorInvasoes++;
      if (estadoAtual == APROXIMACAO_RACK) contadorAproximacoes++;

      pararBuzzers();
      estadoAnterior = estadoAtual;
    }

    if (motivoBloqueio.length() > 0) {
      exibirTelaBloqueio();
      if (millis() - tempoUltimoBip >= 800) {
        tone(Pins::BUZZER_D, 500, 100);
        tempoUltimoBip = millis();
      }
    } else {
      switch (estadoAtual) {
        case SEGURO:           exibirTelaStatus(); pararBuzzers();                break;
        case ACESSO_RFID:      exibirTelaStatus();                                 break;
        case AVISO_SALA:       exibirTelaStatus(); tocarMelodia(AVISO_SALA);       break;
        case APROXIMACAO_RACK: exibirTelaStatus(); tocarMelodia(APROXIMACAO_RACK); break;
        case INVASAO_SALA:     exibirTelaStatus(); tocarMelodia(INVASAO_SALA);     break;
        case EMERGENCIA_RACK:  exibirTelaStatus(); tocarMelodia(EMERGENCIA_RACK);  break;
      }
    }
  } else {
    exibirTelaDesarmado();
    pararBuzzers();
  }

  atualizarLED();
  delay(20);
}

// ==========================================
// REDE — WiFi
// ==========================================
void iniciarWiFi() {
  Serial.print(F("[WIFI] Conectando a "));
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - inicio) < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiConectado = true;
    modoAP = false;
    Serial.print(F("[WIFI] Conectado! IP: "));
    Serial.println(WiFi.localIP());
    logSerial("INFO", "WIFI", "Conectado. IP: " + WiFi.localIP().toString());
  } else if (HABILITAR_FALLBACK_AP) {
    Serial.println(F("[WIFI] Falha. Ativando Access Point..."));
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    wifiConectado = true;
    modoAP = true;
    Serial.print(F("[WIFI] AP ativo. IP: "));
    Serial.println(WiFi.softAPIP());
    logSerial("WARN", "WIFI", "Fallback AP ativo. IP: " + WiFi.softAPIP().toString());
  } else {
    wifiConectado = false;
    Serial.println(F("[WIFI] Sem conexao."));
    logSerial("WARN", "WIFI", "Sem conexao.");
  }
}

// ==========================================
// SERVIDOR REMOTO — envio de status
// ==========================================
void enviarStatusParaServidor() {
  if (!wifiConectado || modoAP) return;
  if (millis() - tempoUltimoEnvioStatus < INTERVALO_ENVIO_STATUS_MS) return;
  tempoUltimoEnvioStatus = millis();

  String json = "{";
  json += "\"timestamp\":\"" + obterTimestamp() + "\",";
  json += "\"modo\":\"" + nomeModo(modoVigilante) + "\",";
  json += "\"estado\":\"" + nomeEstado(estadoAtual) + "\",";
  json += "\"porta\":\"" + String(estadoPortaFiltrado ? "ABERTA" : "fechada") + "\",";
  json += "\"armado\":" + String(modoVigilante == ARMADO ? "true" : "false") + ",";
  json += "\"acessos\":" + String(contadorAcessosValidos) + ",";
  json += "\"bloqueios\":" + String(contadorRearmesBloqueados) + ",";
  json += "\"invasoes\":" + String(contadorInvasoes) + ",";
  json += "\"uid\":\"" + (ultimoUIDLido.length() ? ultimoUIDLido : "-") + "\"";
  json += "}";

  HTTPClient http;
  http.begin(String(SERVIDOR_URL) + "/api/status");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-Rack-Monitor/7.2");
  http.setTimeout(10000);

  int code = http.POST(json);
  ultimoCodigoHTTP = code;

  if (code == 200) {
    if (contadorEnviosOk == 0) {
      logSerial("INFO", "SRV", "Conexao com servidor estabelecida!");
    }
    contadorEnviosOk++;
    servidorOnline = true;
  } else {
    contadorEnviosFalha++;
    servidorOnline = false;
    if (contadorEnviosFalha % 10 == 1) {
      logSerial("WARN", "SRV", "Falha ao enviar. HTTP " + String(code) +
        " | " + http.errorToString(code));
    }
  }
  http.end();
}

// ==========================================
// SERVIDOR REMOTO — busca de comandos
// ==========================================
void buscarComandosDoServidor() {
  if (!wifiConectado || modoAP) return;
  if (millis() - tempoUltimaBuscaCmd < INTERVALO_BUSCA_CMD_MS) return;
  tempoUltimaBuscaCmd = millis();

  HTTPClient http;
  http.begin(String(SERVIDOR_URL) + "/api/comando");
  http.addHeader("User-Agent", "ESP32-Rack-Monitor/7.2");
  http.setTimeout(10000);

  int code = http.GET();
  if (code == 200) {
    String payload = http.getString();
    if (payload.indexOf("\"comando\":\"") >= 0) {
      int inicio = payload.indexOf("\"comando\":\"") + 11;
      int fim = payload.indexOf("\"", inicio);
      if (fim > inicio) {
        String cmd = payload.substring(inicio, fim);
        cmd.trim();
        cmd.toUpperCase();

        if (cmd.length() > 0 && cmd != "NULL") {
          contadorCmdsRecebidos++;
          logSerial("INFO", "SRV", "Comando remoto: " + cmd);
          processarComandoSerial(cmd);
        }
      }
    }
  }
  http.end();
}

// ==========================================
// DIAGNÓSTICO DE REDE
// ==========================================
void executarDiagnostico() {
  Serial.println();
  Serial.println(F("===== DIAGNOSTICO DE REDE ====="));

  Serial.print(F("WiFi Status: "));
  Serial.println(WiFi.status());
  Serial.print(F("Modo: "));
  Serial.println(modoAP ? "AP" : "STA");
  Serial.print(F("IP local: "));
  Serial.println(modoAP ? WiFi.softAPIP() : WiFi.localIP());
  Serial.print(F("Gateway: "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(F("DNS: "));
  Serial.println(WiFi.dnsIP());
  Serial.print(F("RSSI: "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));

  // Teste DNS
  IPAddress ip;
  Serial.print(F("Resolvendo DNS do Cloudflare... "));
  String host = String(SERVIDOR_URL);
  host.replace("https://", "");
  host.replace("http://", "");
  int slash = host.indexOf("/");
  if (slash > 0) host = host.substring(0, slash);
  if (WiFi.hostByName(host.c_str(), ip)) {
    Serial.print(F("OK -> "));
    Serial.println(ip);
  } else {
    Serial.println(F("FALHOU!"));
  }

  // Teste HTTP simples
  Serial.println(F("\nTeste HTTP (example.com)..."));
  HTTPClient http;
  http.begin("http://example.com");
  http.setTimeout(5000);
  int code = http.GET();
  Serial.print(F("HTTP code: "));
  Serial.println(code);
  http.end();

  // Teste HTTPS do servidor
  Serial.println(F("\nTeste HTTPS servidor..."));
  HTTPClient https;
  https.begin(String(SERVIDOR_URL) + "/api/health");
  https.addHeader("User-Agent", "ESP32-Rack-Monitor/7.2");
  https.setTimeout(10000);
  int code2 = https.GET();
  Serial.print(F("HTTP code: "));
  Serial.print(code2);
  if (code2 == 200) {
    Serial.print(F(" | Resposta: "));
    Serial.println(https.getString());
  } else {
    Serial.print(F(" | Erro: "));
    Serial.println(https.errorToString(code2));
  }
  https.end();

  Serial.println(F("================================"));
  Serial.println();
}

// ==========================================
// TESTE HTTPS PARA 3 SITES
// ==========================================
void testarHTTPS() {
  logSerial("INFO", "HTTPS", "Testando 3 sites HTTPS...");

  const char* sites[] = {
    "https://www.google.com",
    "https://api.github.com",
    "https://httpbin.org/get"
  };

  for (int i = 0; i < 3; i++) {
    HTTPClient http;
    http.begin(sites[i]);
    http.addHeader("User-Agent", "ESP32-Rack-Monitor/7.2");
    http.setTimeout(8000);
    int code = http.GET();
    logSerial("INFO", "HTTPS", String(sites[i]) + " -> " + String(code));
    http.end();
    delay(300);
  }
  logSerial("INFO", "HTTPS", "Teste concluido.");
}

// ==========================================
// FILTROS
// ==========================================
bool lerLDRFiltrado() {
  int leitura = analogRead(Pins::LDR_AO);

  if (!estadoLuzFiltrado && leitura > LDR_LIMIAR_ACENDE) {
    if (millis() - tempoUltimaMudancaLDR >= DEBOUNCE_LDR_MS) {
      estadoLuzFiltrado     = true;
      tempoUltimaMudancaLDR = millis();
    }
  } else if (estadoLuzFiltrado && leitura < LDR_LIMIAR_APAGA) {
    if (millis() - tempoUltimaMudancaLDR >= DEBOUNCE_LDR_MS) {
      estadoLuzFiltrado     = false;
      tempoUltimaMudancaLDR = millis();
    }
  }

  return estadoLuzFiltrado;
}

bool lerPortaFiltrada() {
  bool bruto = (digitalRead(Pins::SWITCH) == HIGH);

  if (bruto != portaBrutaAnterior) {
    tempoUltimaMudancaPorta = millis();
    portaBrutaAnterior = bruto;
  }

  if ((millis() - tempoUltimaMudancaPorta) >= DEBOUNCE_PORTA_MS &&
      bruto != estadoPortaFiltrado) {
    estadoPortaFiltrado = bruto;
  }

  return estadoPortaFiltrado;
}

float lerDistanciaBruta() {
  digitalWrite(Pins::TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(Pins::TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(Pins::TRIG, LOW);
  long duracao = pulseIn(Pins::ECHO, HIGH, 25000);
  float d = duracao * 0.0343f / 2.0f;
  return (d <= 0.0f) ? 999.0f : d;
}

float lerDistanciaFiltrada() {
  float soma    = 0.0f;
  int   validas = 0;
  for (int i = 0; i < US_AMOSTRAS; i++) {
    float d = lerDistanciaBruta();
    if (d > 0.0f && d < US_DIST_MAX_VALIDA) {
      soma += d;
      validas++;
    }
    delayMicroseconds(60);
  }
  return (validas > 0) ? (soma / validas) : 999.0f;
}

// ==========================================
// MODO VIGILANTE
// ==========================================
void alternarModoVigilante() {
  if (modoVigilante == ARMADO) {
    modoVigilante        = DESARMADO;
    tempoInicioDesarme   = millis();
    uidUltimoDesarme     = ultimoUIDLido;
    aviso5minDisparado   = false;
    contadorDesarmes++;
    motivoBloqueio       = "";
    logSerial("INFO", "VIGILANTE",
      "DESARMADO por UID: " + ultimoUIDLido +
      " | Timeout: " + String(timeoutDesarme / 60000) + " min");
    tone(Pins::BUZZER_E, 800, 100); delay(120);
    tone(Pins::BUZZER_E, 600, 150);
  } else {
    tentarArmar();
  }
}

bool tentarArmar() {
  bool portaAberta = estadoPortaFiltrado;

  if (portaAberta) {
    contadorRearmesBloqueados++;
    motivoBloqueio     = "Feche a porta!";
    tempoBloqueioAtivo = millis();

    logSerial("WARN", "VIGILANTE",
      "REARME BLOQUEADO: porta aberta | " +
      String(contadorRearmesBloqueados) + " bloqueio(s) ate agora.");
    logSerial("WARN", "VIGILANTE",
      "Feche a porta e passe o cartao novamente para armar.");

    tone(Pins::BUZZER_D, 500, 200); delay(250);
    tone(Pins::BUZZER_D, 400, 200);
    return false;
  }

  duracaoUltimoDesarme = (millis() - tempoInicioDesarme) / 1000UL;
  modoVigilante        = ARMADO;
  uidUltimoRearme      = ultimoUIDLido;
  motivoBloqueio       = "";

  logSerial("INFO", "VIGILANTE",
    "REARMADO por UID: " + ultimoUIDLido +
    " | Duracao do desarme: " + String(duracaoUltimoDesarme) + "s");
  tone(Pins::BUZZER_E, 600, 100);  delay(120);
  tone(Pins::BUZZER_E, 800, 100);  delay(120);
  tone(Pins::BUZZER_E, 1000, 150);

  estadoAtual     = SEGURO;
  estadoAnterior  = SEGURO;
  estadoCandidato = SEGURO;
  return true;
}

void rearmarPorTimeout() {
  bool portaAberta = estadoPortaFiltrado;

  if (portaAberta) {
    logSerial("WARN", "VIGILANTE",
      "TIMEOUT IGNORADO: porta aberta impede rearme automatico. Reiniciando contagem.");
    tempoInicioDesarme = millis();
    aviso5minDisparado = false;
    return;
  }

  duracaoUltimoDesarme = (millis() - tempoInicioDesarme) / 1000UL;
  modoVigilante        = ARMADO;
  uidUltimoRearme      = "TIMEOUT";
  logSerial("WARN", "VIGILANTE",
    "REARME AUTOMATICO por timeout | Duracao: " + String(duracaoUltimoDesarme) + "s");
  tone(Pins::BUZZER_D, 1500, 200); delay(250);
  tone(Pins::BUZZER_D, 2000, 300);
  estadoAtual     = SEGURO;
  estadoAnterior  = SEGURO;
  estadoCandidato = SEGURO;
}

// ==========================================
// RFID
// ==========================================
bool isCartaoValido(const String &uid) {
  return (uid == CARTAO_AUTORIZADO_1 || uid == CARTAO_AUTORIZADO_2);
}

bool verificarRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return false;
  }

  ultimoUIDLido = "";
  for (uint8_t i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) ultimoUIDLido += "0";
    ultimoUIDLido += String(rfid.uid.uidByte[i], HEX);
  }
  ultimoUIDLido.toUpperCase();

  tempoUltimoRFID  = millis();
  cartaoAutorizado = isCartaoValido(ultimoUIDLido);

  if (cartaoAutorizado) {
    contadorAcessosValidos++;
    logSerial("INFO", "RFID", "Cartao AUTORIZADO: " + ultimoUIDLido);
    alternarModoVigilante();
  } else {
    logSerial("WARN", "RFID", "Acesso NEGADO! Cartao nao cadastrado: " + ultimoUIDLido);
    tone(Pins::BUZZER_D, 400, 350);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return true;
}

// ==========================================
// BOTÃO
// ==========================================
void processarBotao() {
  bool bruto = (digitalRead(Pins::BOTAO) == LOW);

  if (bruto != botaoBrutoAnterior) {
    tempoUltimaMudancaBotao = millis();
    botaoBrutoAnterior = bruto;
  }

  if ((millis() - tempoUltimaMudancaBotao) < DEBOUNCE_BOTAO_MS) return;
  if (bruto == botaoFiltrado) return;

  botaoFiltrado = bruto;

  if (botaoFiltrado) {
    tempoBotaoPressionado = millis();
  } else {
    unsigned long duracao = millis() - tempoBotaoPressionado;
    if (millis() - tempoUltimaAcaoBotao < DEBOUNCE_BOTAO_MS) return;

    if (duracao >= DURACAO_CLIQUE_LONGO_MS) {
      tempoUltimaAcaoBotao = millis();
      cliquesBotao = 0;
      logSerial("INFO", "BOTAO", "Clique LONGO (" + String(duracao) + "ms): rearme forçado.");
      if (modoVigilante == DESARMADO) {
        tentarArmar();
      } else {
        logSerial("WARN", "BOTAO", "Ja esta ARMADO.");
      }
      tone(Pins::BUZZER_E, 1000, 200);
    } else if (duracao >= 50) {
      cliquesBotao++;
      tempoUltimoClique = millis();
    }
  }

  if (cliquesBotao >= 2 && (millis() - tempoUltimoClique < JANELA_CLIQUE_DUPLO_MS)) {
    tempoUltimaAcaoBotao = millis();
    cliquesBotao = 0;
    logSerial("INFO", "BOTAO", "Clique DUPLO: modo teste.");
    executarModoTeste();
  } else if (cliquesBotao == 1 && (millis() - tempoUltimoClique > JANELA_CLIQUE_DUPLO_MS)) {
    tempoUltimaAcaoBotao = millis();
    cliquesBotao = 0;
    contadorAberturasPorta       = 0;
    contadorInvasoes             = 0;
    contadorAproximacoes         = 0;
    contadorAcessosValidos       = 0;
    contadorMovimentosManutencao = 0;
    logSerial("INFO", "BOTAO", "Clique CURTO: contadores zerados.");
    tone(Pins::BUZZER_E, 1200, 100);
  }
}

void executarModoTeste() {
  logSerial("INFO", "TESTE", "Iniciando modo teste...");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 20);
  display.println("MODO TESTE");
  display.setCursor(10, 40);
  display.println("Aguarde...");
  display.display();

  definirCorLED(255, 0, 0);     delay(300);
  definirCorLED(0, 255, 0);     delay(300);
  definirCorLED(0, 0, 255);     delay(300);
  definirCorLED(255, 255, 255); delay(300);
  definirCorLED(0, 0, 0);

  tone(Pins::BUZZER_E, 500, 150);  delay(200);
  tone(Pins::BUZZER_E, 1000, 150); delay(200);
  tone(Pins::BUZZER_E, 2000, 150); delay(200);
  tone(Pins::BUZZER_D, 800, 300);  delay(350);

  logSerial("TESTE", "SENSOR", "PIR: " + String(digitalRead(Pins::PIR) ? "ATIVO" : "inativo"));
  logSerial("TESTE", "SENSOR",
    "LDR: " + String(analogRead(Pins::LDR_AO)) +
    " (" + String(estadoLuzFiltrado ? "ACESA" : "APAGADA") + ")");
  logSerial("TESTE", "SENSOR", "Porta: " + String(estadoPortaFiltrado ? "ABERTA" : "fechada"));
  logSerial("TESTE", "SENSOR", "Distancia: " + String(lerDistanciaFiltrada(), 1) + " cm");

  logSerial("INFO", "TESTE", "Modo teste concluido.");
  tone(Pins::BUZZER_E, 1500, 100); delay(120);
  tone(Pins::BUZZER_E, 2000, 150);
}

// ==========================================
// MELODIAS
// ==========================================
void tocarMelodia(EstadoSistema estado) {
  unsigned long agora = millis();

  switch (estado) {
    case SEGURO:
    case ACESSO_RFID:
      noTone(Pins::BUZZER_E);
      noTone(Pins::BUZZER_D);
      break;
    case AVISO_SALA:
      if (agora - tempoUltimoBip >= 700) {
        tone(Pins::BUZZER_E, 523, 100);
        tempoUltimoBip = agora;
      }
      break;
    case APROXIMACAO_RACK:
      if (agora - tempoUltimoBip >= 400) {
        tone(Pins::BUZZER_E, 750, 80);
        tempoUltimoBip = agora;
      }
      break;
    case INVASAO_SALA:
      if (agora - tempoUltimoBip >= 25) {
        static int freq  = 400;
        static int passo = 25;
        tone(Pins::BUZZER_D, freq, 20);
        freq += passo;
        if (freq >= 900 || freq <= 400) passo = -passo;
        tempoUltimoBip = agora;
      }
      break;
    case EMERGENCIA_RACK:
      if (agora - tempoUltimoBip >= 160) {
        static bool alterna = false;
        tone(Pins::BUZZER_D, alterna ? 900 : 1300, 140);
        alterna = !alterna;
        tempoUltimoBip = agora;
      }
      break;
  }
}

void pararBuzzers() {
  noTone(Pins::BUZZER_E);
  noTone(Pins::BUZZER_D);
}

// ==========================================
// LED RGB
// ==========================================
void definirCorLED(int r, int g, int b) {
  ledcWrite(Pins::LED_R, 255 - constrain(r, 0, 255));
  ledcWrite(Pins::LED_G, 255 - constrain(g, 0, 255));
  ledcWrite(Pins::LED_B, 255 - constrain(b, 0, 255));
}

void atualizarLED() {
  unsigned long agora = millis();

  if (motivoBloqueio.length() > 0) {
    if ((agora / 250) % 2 == 0) definirCorLED(255, 200, 0);
    else                         definirCorLED(0, 0, 0);
    return;
  }

  if (modoVigilante == DESARMADO) {
    float fase   = (agora % 2000) / 2000.0f;
    int   brilho = (int)(127.5f * (1.0f + sin(fase * 2.0f * PI)));
    definirCorLED(0, 0, brilho);
    return;
  }

  switch (estadoAtual) {
    case SEGURO:           definirCorLED(0, 255, 0);   break;
    case ACESSO_RFID:      definirCorLED(0, 255, 0);   break;
    case AVISO_SALA:       definirCorLED(255, 200, 0); break;
    case APROXIMACAO_RACK: definirCorLED(255, 100, 0); break;
    case INVASAO_SALA:
      if (agora - tempoUltimoPisca >= 250) {
        estadoPiscaLED   = !estadoPiscaLED;
        tempoUltimoPisca = agora;
      }
      definirCorLED(estadoPiscaLED ? 255 : 0, 0, 0);
      break;
    case EMERGENCIA_RACK:  definirCorLED(255, 0, 0);   break;
  }
}

// ==========================================
// DISPLAY OLED
// ==========================================
void desenharIcone(int tipo, int x, int y) {
  switch (tipo) {
    case 1:
      display.drawRect(x, y + 6, 12, 9, SSD1306_WHITE);
      display.fillRect(x + 2, y + 8, 8, 5, SSD1306_WHITE);
      display.drawCircle(x + 6, y + 6, 4, SSD1306_WHITE);
      break;
    case 2:
      display.drawTriangle(x + 8, y, x, y + 14, x + 16, y + 14, SSD1306_WHITE);
      display.drawLine(x + 8, y + 5, x + 8, y + 9, SSD1306_WHITE);
      display.drawPixel(x + 8, y + 12, SSD1306_WHITE);
      break;
    case 3:
      display.drawRect(x, y + 6, 12, 9, SSD1306_WHITE);
      display.drawRect(x + 2, y + 8, 8, 5, SSD1306_WHITE);
      display.drawCircle(x + 6, y + 3, 4, SSD1306_WHITE);
      display.fillRect(x + 2, y + 2, 5, 4, SSD1306_BLACK);
      display.drawLine(x + 19, y + 2, x + 19, y + 8, SSD1306_WHITE);
      display.drawPixel(x + 19, y + 11, SSD1306_WHITE);
      break;
    case 4:
      display.drawTriangle(x + 8, y, x, y + 14, x + 16, y + 14, SSD1306_WHITE);
      display.drawLine(x + 4, y + 7, x + 8, y + 11, SSD1306_WHITE);
      display.drawLine(x + 8, y + 11, x + 12, y + 5, SSD1306_WHITE);
      break;
    case 5:
      display.drawRect(x, y + 2, 12, 16, SSD1306_WHITE);
      display.fillRect(x + 8, y + 9, 2, 3, SSD1306_WHITE);
      display.drawLine(x + 15, y + 5, x + 15, y + 12, SSD1306_WHITE);
      display.drawPixel(x + 15, y + 15, SSD1306_WHITE);
      break;
  }
}

void exibirTelaStatus() {
  display.clearDisplay();
  String linha1, linha2;
  int    icone = 1;

  switch (estadoAtual) {
    case SEGURO:           linha1 = "VIGILANTE ARMADO"; linha2 = "Porta Fechada"; icone = 1; break;
    case ACESSO_RFID:
      linha1 = cartaoAutorizado ? "ACESSO LIBERADO" : "ACESSO NEGADO!";
      linha2 = "UID: " + ultimoUIDLido;
      icone  = cartaoAutorizado ? 1 : 3;
      break;
    case AVISO_SALA:       linha1 = "AVISO: Movimento"; linha2 = "Rack Fechado"; icone = 2; break;
    case APROXIMACAO_RACK:
      linha1 = "APROXIMACAO"; linha2 = "Dist: " + String(lerDistanciaFiltrada(), 1) + "cm"; icone = 2; break;
    case INVASAO_SALA:     linha1 = "INVASAO NA SALA!"; linha2 = "Mov+Luz | #" + String(contadorInvasoes); icone = 3; break;
    case EMERGENCIA_RACK:  linha1 = "EMERGENCIA: RACK!"; linha2 = "Porta Aberta #" + String(contadorAberturasPorta); icone = 3; break;
  }

  desenharIcone(icone, 2, 2);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(24, 4);
  display.println(linha1);

  display.drawLine(0, 20, 128, 20, SSD1306_WHITE);
  display.setCursor(0, 26);
  display.println(linha2);

  if (servidorOnline) {
    display.fillCircle(122, 6, 3, SSD1306_WHITE);
  } else {
    display.drawCircle(122, 6, 3, SSD1306_WHITE);
  }

  display.drawLine(0, 46, 128, 46, SSD1306_WHITE);
  display.setCursor(0, 52);
  display.print(obterTimestamp());
  display.print(" AB:");
  display.print(contadorAberturasPorta);

  display.display();
}

void exibirTelaDesarmado() {
  display.clearDisplay();
  desenharIcone(4, 2, 2);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(24, 4);
  display.println("MANUTENCAO");

  if (servidorOnline) {
    display.fillCircle(122, 6, 3, SSD1306_WHITE);
  } else {
    display.drawCircle(122, 6, 3, SSD1306_WHITE);
  }

  display.drawLine(0, 20, 128, 20, SSD1306_WHITE);

  unsigned long decorrido = millis() - tempoInicioDesarme;
  unsigned long restante  = (timeoutDesarme > decorrido) ? (timeoutDesarme - decorrido) : 0;
  unsigned long min = restante / 60000UL;
  unsigned long seg = (restante % 60000UL) / 1000UL;

  display.setCursor(0, 26);
  display.print("Rearme em: ");
  display.print(min);
  display.print("m");
  display.print(seg);
  display.print("s");

  display.setCursor(0, 36);
  display.print("UID: ");
  display.print(uidUltimoDesarme);

  display.drawLine(0, 46, 128, 46, SSD1306_WHITE);
  display.setCursor(0, 52);
  display.print(obterTimestamp());
  display.print("  MANUT:");
  display.print(contadorMovimentosManutencao);

  display.display();
}

void exibirTelaBloqueio() {
  display.clearDisplay();
  desenharIcone(5, 2, 2);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(24, 4);
  display.println("REARME BLOQUEADO");

  display.drawLine(0, 20, 128, 20, SSD1306_WHITE);
  display.setCursor(0, 26);
  display.println(motivoBloqueio);

  display.setCursor(0, 38);
  display.print("Bloqueios: ");
  display.print(contadorRearmesBloqueados);

  display.drawLine(0, 46, 128, 46, SSD1306_WHITE);
  display.setCursor(0, 52);
  display.print("UID: ");
  display.print(ultimoUIDLido);

  display.display();
}

// ==========================================
// LOGS
// ==========================================
String obterTimestamp() {
  if (rtcDisponivel) {
    DateTime agora = rtc.now();
    char buf[20];
    sprintf(buf, "%02d/%02d %02d:%02d:%02d",
            agora.day(), agora.month(),
            agora.hour(), agora.minute(), agora.second());
    return String(buf);
  } else {
    unsigned long ms = millis();
    unsigned long s  = ms / 1000;
    unsigned long m  = s / 60;
    unsigned long h  = m / 60;
    s %= 60; m %= 60;
    char buf[12];
    sprintf(buf, "%02lu:%02lu:%02lu", h, m, s);
    return String(buf);
  }
}

void logSerial(const String &nivel, const String &tag, const String &mensagem) {
  Serial.print("[");
  Serial.print(obterTimestamp());
  Serial.print("] [");
  Serial.print(nivel);
  Serial.print("]");
  if (nivel.length() == 4) Serial.print(" ");
  Serial.print(" [");
  Serial.print(tag);
  Serial.print("] ");
  Serial.println(mensagem);
}

String nomeEstado(EstadoSistema estado) {
  switch (estado) {
    case SEGURO:           return "SEGURO";
    case ACESSO_RFID:      return "ACESSO_RFID";
    case AVISO_SALA:       return "AVISO_SALA";
    case APROXIMACAO_RACK: return "APROXIMACAO_RACK";
    case EMERGENCIA_RACK:  return "EMERGENCIA_RACK";
    case INVASAO_SALA:     return "INVASAO_SALA";
    default:               return "DESCONHECIDO";
  }
}

String nomeModo(ModoVigilante modo) {
  return (modo == ARMADO) ? "ARMADO" : "DESARMADO";
}

// ==========================================
// COMANDOS SERIAL
// ==========================================
void processarComandoSerial(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  logSerial("INFO", "CMD", "Comando recebido: " + cmd);

  if (cmd.startsWith("SET LIMIAR ")) {
    float novo = cmd.substring(11).toFloat();
    if (novo >= 5 && novo <= 400) {
      limiarDistancia = novo;
      logSerial("INFO", "CMD", "Limiar ajustado para " + String(limiarDistancia) + " cm.");
    } else {
      logSerial("WARN", "CMD", "Valor fora da faixa (5-400 cm).");
    }
  }
  else if (cmd.startsWith("SET TIMEOUT ")) {
    long minutos = cmd.substring(12).toInt();
    if (minutos >= 0 && minutos <= 1440) {
      timeoutDesarme = (unsigned long)minutos * 60UL * 1000UL;
      logSerial("INFO", "CMD", "Timeout de desarme: " + String(minutos) + " min.");
      if (modoVigilante == DESARMADO) {
        tempoInicioDesarme = millis();
        aviso5minDisparado = false;
      }
    } else {
      logSerial("WARN", "CMD", "Timeout fora da faixa (0-1440 min).");
    }
  }
  else if (cmd == "STATUS") {
    Serial.println(F("----------- STATUS ATUAL -----------"));
    Serial.print(F("  Modo Vigilante:       ")); Serial.println(nomeModo(modoVigilante));
    Serial.print(F("  Estado Alarme:        ")); Serial.println(nomeEstado(estadoAtual));
    Serial.print(F("  Porta (filtrada):     ")); Serial.println(estadoPortaFiltrado ? "ABERTA" : "fechada");
    Serial.print(F("  WiFi:                 "));
    if (wifiConectado) {
      Serial.print(modoAP ? "AP" : "STA");
      Serial.print(F(" | IP: "));
      Serial.println(modoAP ? WiFi.softAPIP() : WiFi.localIP());
    } else {
      Serial.println(F("OFF"));
    }
    Serial.print(F("  Servidor remoto:      ")); Serial.println(servidorOnline ? "ONLINE" : "OFFLINE");
    Serial.print(F("  Envios OK/Falha:      "));
    Serial.print(contadorEnviosOk); Serial.print("/"); Serial.println(contadorEnviosFalha);
    Serial.print(F("  Comandos recebidos:   ")); Serial.println(contadorCmdsRecebidos);
    Serial.print(F("  Timeout Desarme:      ")); Serial.print(timeoutDesarme / 60000UL); Serial.println(F(" min"));
    if (modoVigilante == DESARMADO) {
      unsigned long dec = millis() - tempoInicioDesarme;
      unsigned long rst = (timeoutDesarme > dec) ? (timeoutDesarme - dec) / 1000UL : 0;
      Serial.print(F("  Rearme em:            ")); Serial.print(rst); Serial.println(F(" s"));
      Serial.print(F("  UID Desarme:          ")); Serial.println(uidUltimoDesarme);
    } else {
      Serial.print(F("  UID Ultimo Rearme:    "));
      Serial.println(uidUltimoRearme.length() ? uidUltimoRearme : "-");
    }
    Serial.print(F("  Contador Desarmes:    ")); Serial.println(contadorDesarmes);
    Serial.print(F("  Rearmes Bloqueados:   ")); Serial.println(contadorRearmesBloqueados);
    Serial.print(F("  Duracao ult. desarme: ")); Serial.print(duracaoUltimoDesarme); Serial.println(F(" s"));
    Serial.print(F("  Aberturas da Porta:   ")); Serial.println(contadorAberturasPorta);
    Serial.print(F("  Acessos RFID:         ")); Serial.println(contadorAcessosValidos);
    Serial.print(F("  Invasoes:             ")); Serial.println(contadorInvasoes);
    Serial.print(F("  Aproximacoes:         ")); Serial.println(contadorAproximacoes);
    Serial.print(F("  Mov. em manutencao:   ")); Serial.println(contadorMovimentosManutencao);
    Serial.print(F("  Ultimo UID:           "));
    Serial.println(ultimoUIDLido.length() ? ultimoUIDLido : "NENHUM");
    Serial.println(F("-------------------------------------"));
  }
  else if (cmd == "ARMAR") {
    if (modoVigilante == DESARMADO) {
      logSerial("INFO", "CMD", "Tentativa de ARMAR.");
      tentarArmar();
    } else {
      logSerial("WARN", "CMD", "Ja esta armado.");
    }
  }
  else if (cmd == "DESARMAR") {
    if (modoVigilante == ARMADO) {
      modoVigilante      = DESARMADO;
      tempoInicioDesarme = millis();
      uidUltimoDesarme   = "REMOTO";
      aviso5minDisparado = false;
      logSerial("INFO", "CMD", "Modo Vigilante DESARMADO via comando.");
    } else {
      logSerial("WARN", "CMD", "Ja esta desarmado.");
    }
  }
  else if (cmd == "TESTE") {
    logSerial("INFO", "CMD", "Executando modo teste.");
    executarModoTeste();
  }
  else if (cmd == "TESTE_HTTPS") {
    testarHTTPS();
  }
  else if (cmd == "TESTE_SRV") {
    logSerial("INFO", "TUNEL", "Testando " + String(SERVIDOR_URL) + "/api/health");
    HTTPClient http;
    http.begin(String(SERVIDOR_URL) + "/api/health");
    http.addHeader("User-Agent", "ESP32-Rack-Monitor/7.2");
    http.setTimeout(12000);
    int code = http.GET();
    if (code == 200) {
      logSerial("INFO", "TUNEL", "OK! " + http.getString());
    } else {
      logSerial("WARN", "TUNEL", "Falhou. Codigo: " + String(code) +
        " | " + http.errorToString(code));
    }
    http.end();
  }
  else if (cmd == "DIAG") {
    logSerial("INFO", "CMD", "Executando diagnostico de rede...");
    executarDiagnostico();
  }
  else if (cmd == "RESET") {
    contadorAberturasPorta       = 0;
    contadorInvasoes             = 0;
    contadorAproximacoes         = 0;
    contadorAcessosValidos       = 0;
    contadorMovimentosManutencao = 0;
    contadorDesarmes             = 0;
    contadorRearmesBloqueados    = 0;
    contadorEnviosOk             = 0;
    contadorEnviosFalha          = 0;
    contadorCmdsRecebidos        = 0;
    logSerial("INFO", "CMD", "Contadores zerados.");
  }
  else if (cmd.startsWith("RTC ")) {
    int ano, mes, dia, hora, min, seg;
    if (sscanf(cmd.c_str(), "RTC %d-%d-%d %d:%d:%d",
               &ano, &mes, &dia, &hora, &min, &seg) == 6) {
      rtc.adjust(DateTime(ano, mes, dia, hora, min, seg));
      logSerial("INFO", "RTC", "RTC ajustado.");
    } else {
      logSerial("WARN", "CMD", "Formato: RTC YYYY-MM-DD HH:MM:SS");
    }
  }
  else if (cmd == "RTC") {
    DateTime agora = rtc.now();
    char buf[30];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            agora.year(), agora.month(), agora.day(),
            agora.hour(), agora.minute(), agora.second());
    logSerial("INFO", "RTC", "Hora atual: " + String(buf));
  }
  else if (cmd == "HELP") {
    Serial.println(F("----------- COMANDOS DISPONIVEIS -----------"));
    Serial.println(F("  SET LIMIAR <5-400>      - Ajusta limiar ultrassonico"));
    Serial.println(F("  SET TIMEOUT <0-1440>    - Timeout de desarme (min)"));
    Serial.println(F("  STATUS                  - Status completo"));
    Serial.println(F("  ARMAR                   - Forca modo ARMADO (se porta fechada)"));
    Serial.println(F("  DESARMAR                - Forca modo DESARMADO"));
    Serial.println(F("  TESTE                   - Modo teste"));
    Serial.println(F("  TESTE_HTTPS             - Testa HTTPS para 3 sites"));
    Serial.println(F("  TESTE_SRV               - Testa conexao com servidor remoto"));
    Serial.println(F("  DIAG                    - Diagnostico de rede detalhado"));
    Serial.println(F("  RTC                     - Mostra hora do RTC"));
    Serial.println(F("  RTC YYYY-MM-DD HH:MM:SS - Ajusta RTC"));
    Serial.println(F("  RESET                   - Zera contadores"));
    Serial.println(F("  HELP                    - Lista comandos"));
    Serial.println(F("---------------------------------------------"));
    Serial.println(F("  Dashboard: acesse a URL do Cloudflare no navegador"));
  }
  else {
    logSerial("WARN", "CMD", "Comando desconhecido. Digite HELP.");
  }
}