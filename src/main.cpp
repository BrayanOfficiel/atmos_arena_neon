#include <BluetoothSerial.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     26
#define LED_COUNT   60        // nombre de LEDs PAR bandeau
#define BRIGHTNESS  80        // max 255 (reste safe)

BluetoothSerial SerialBT;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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

/* ---------- UTILS ---------- */
void showColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

/* ---------- EFFECTS ---------- */
void effectIdle() {
  static int dir = 1;
  static int val = 10;

  val += dir;
  if (val > 40 || val < 5) dir *= -1;

  showColor(0, 0, val);
}

void effectWin() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0, 255, 0);
  }
  strip.show();
}

void effectBlue() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0, 0, 255);
  }
  strip.show();
}

void effectLose() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 255, 0, 0);
  }
  strip.show();
}

void effectHype() {
  strip.clear();
  strip.setPixelColor(stepIndex, 255, 255, 255);
  strip.show();

  stepIndex++;
  if (stepIndex >= LED_COUNT) stepIndex = 0;
}

/* ---------- LOOP EFFECT ---------- */
void runEffect() {
  unsigned long now = millis();

  switch (currentEffect) {
    case IDLE:
      if (now - lastUpdate > 30) {
        lastUpdate = now;
        effectIdle();
        break;
      }

    case R:
      effectLose();
      break;

    case G:
      effectWin();
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

/* ---------- SETUP ---------- */
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ARENA_LED"); // nom Bluetooth

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  Serial.println("ESP32 READY");
}

/* ---------- LOOP ---------- */
void loop() {
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();

    if (cmd == "IDLE") currentEffect = IDLE;
    else if (cmd == "G") currentEffect = G;
    else if (cmd == "R") currentEffect = R;
    else if (cmd == "B") currentEffect = B;
    else if (cmd == "HYPE") currentEffect = HYPE;
    else if (cmd == "RESET") {
      currentEffect = IDLE;
      stepIndex = 0;
    }

    Serial.println("CMD: " + cmd);
  }

  runEffect();
}