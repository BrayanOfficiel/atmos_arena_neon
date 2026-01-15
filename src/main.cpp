#include <BluetoothSerial.h>
#include <Adafruit_NeoPixel.h>

/* ====== CONFIG ====== */
#define STRIP_PIN      26
#define STRIP_COUNT    60

#define MATRIX_PIN     27
#define MATRIX_COUNT   192   // 3 matrices 8x8

#define BRIGHTNESS     80

BluetoothSerial SerialBT;

/* ====== LED OBJECTS ====== */
Adafruit_NeoPixel strip(STRIP_COUNT, STRIP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel matrix(MATRIX_COUNT, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

/* ====== EFFECTS ====== */
enum Effect {
  IDLE,
  R,
  G,
  B,
  HYPE
};

Effect currentEffect = IDLE;

unsigned long lastUpdate = 0;
int stepIndex = 0;

/* ====== SCORES ====== */
int scoreP1 = 0;
int scoreP2 = 0;

/* ====== DIGITS 8x8 (0–9) ====== */
const uint8_t digits[10][8] = {
  {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, //0
  {0x18,0x38,0x18,0x18,0x18,0x18,0x3C,0x00}, //1
  {0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0x00}, //2
  {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, //3
  {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00}, //4
  {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, //5
  {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, //6
  {0x7E,0x66,0x06,0x0C,0x18,0x18,0x18,0x00}, //7
  {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, //8
  {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}  //9
};

/* ====== UTILS ====== */
void showColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < STRIP_COUNT; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

/* ====== STRIP EFFECTS ====== */
void effectIdle() {
  static int dir = 1;
  static int val = 10;

  val += dir;
  if (val > 40 || val < 5) dir *= -1;

  showColor(0, 0, val);
}

void effectRed()   { showColor(255, 0, 0); }
void effectGreen() { showColor(0, 255, 0); }
void effectBlue()  { showColor(0, 0, 255); }

void effectHype() {
  strip.clear();
  strip.setPixelColor(stepIndex, 255, 255, 255);
  strip.show();

  stepIndex++;
  if (stepIndex >= STRIP_COUNT) stepIndex = 0;
}

/* ====== MATRIX ====== */
void clearMatrix(int offset) {
  for (int i = 0; i < 64; i++) {
    matrix.setPixelColor(offset + i, 0);
  }
}

void drawDigit(int digit, int offset, uint32_t color) {
  if (digit < 0 || digit > 9) return;

  clearMatrix(offset);

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (digits[digit][y] & (1 << (7 - x))) {
        int i = offset + y * 8 + x;
        matrix.setPixelColor(i, color);
      }
    }
  }
}

void updateScores() {
  drawDigit(scoreP1, 0,   matrix.Color(0, 255, 0));   // matrice 1
  drawDigit(scoreP2, 64,  matrix.Color(255, 0, 0));   // matrice 2
  matrix.show();
}

/* ====== RUN EFFECT ====== */
void runEffect() {
  unsigned long now = millis();

  switch (currentEffect) {
    case IDLE:
      if (now - lastUpdate > 30) {
        lastUpdate = now;
        effectIdle();
      }
      break;

    case R:
      effectRed();
      break;

    case G:
      effectGreen();
      break;

    case B:
      effectBlue();
      break;

    case HYPE:
      if (now - lastUpdate > 20) {
        lastUpdate = now;
        effectHype();
      }
      break;
  }
}

/* ====== SETUP ====== */
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ARENA_LED");

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  matrix.begin();
  matrix.setBrightness(BRIGHTNESS);
  matrix.show();

  updateScores();
}

/* ====== LOOP ====== */
void loop() {
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();

    if (cmd == "IDLE") currentEffect = IDLE;
    else if (cmd == "R") currentEffect = R;
    else if (cmd == "G") currentEffect = G;
    else if (cmd == "B") currentEffect = B;
    else if (cmd == "HYPE") currentEffect = HYPE;

    else if (cmd.startsWith("P1")) {
      scoreP1 = cmd.substring(2).toInt();
      updateScores();
    }
    else if (cmd.startsWith("P2")) {
      scoreP2 = cmd.substring(2).toInt();
      updateScores();
    }

    Serial.println("CMD: " + cmd);
  }

  runEffect();
}