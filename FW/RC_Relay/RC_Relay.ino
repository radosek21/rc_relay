#include <RCSwitch.h>
#include <arduino-timer.h>

// ***** DEFIDES *****
#define DIP_SWITCH_INPUTS_CNT   ( 6 )
#define RELAY_PULSE_DURATION    ( 1000 ) // ms
#define RELAY_1_PORT            ( 10 ) // D10
#define RELAY_2_PORT            ( 9 ) // D9
#define DIP_SWITCH_READ_PERIOD  ( 1000 ) // ms
#define BUILDIN_LED_TOGGLE      ( 1000 ) // ms

// ***** TYPEDEFS *****
typedef enum {
  BTN_1 = 0,
  BTN_2,
  BTN_3,
  BTN_4,
  BTN_5,
  BTN_6,
  BTN_7,
  BTN_8,
  BTN_COUNT
} Button_t;

// ***** CONSTANTS *****
const uint32_t buttonsCodeMap[BTN_COUNT] = {
  0x2ECA8, // BTN_1
  0x2ECA4, // BTN_2
  0x2ECAC, // BTN_3
  0x2ECA2, // BTN_4
  0x2ECAA, // BTN_5
  0x2ECA6, // BTN_6
  0x2ECAE, // BTN_7
  0x2ECA1  // BTN_8
};

const int dipSwitchPins[DIP_SWITCH_INPUTS_CNT] = { 3, 4, 5, 6, 7, 8 };

// ***** GLOBAL VARIBLES *****
RCSwitch receiver = RCSwitch();
auto timer = timer_create_default();
uint8_t ch1Channel;
uint8_t ch2Channel;


// ***** GLOBAL FUNCTIONS *****
void setup() {
  pinMode(RELAY_1_PORT, OUTPUT);
  pinMode(RELAY_2_PORT, OUTPUT);
  digitalWrite(RELAY_1_PORT, LOW);
  digitalWrite(RELAY_1_PORT, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < DIP_SWITCH_INPUTS_CNT; i++) {
    pinMode(dipSwitchPins[i], INPUT_PULLUP);
  }
  readDipSwitch(NULL);
  Serial.begin(9600);
  receiver.enableReceive(0);
  timer.every(DIP_SWITCH_READ_PERIOD, readDipSwitch);
  timer.every(BUILDIN_LED_TOGGLE, buildinLedBlink);
}

void loop() {
  timer.tick();
  if (receiver.available()) {
    if (receiver.getReceivedBitlength() == 24) {
      uint32_t btnCode = receiver.getReceivedValue();
      for(int i = 0; i < BTN_COUNT; i++) {
        if(btnCode == buttonsCodeMap[i]) {
          Serial.print("Pressed button ");
          Serial.println(i+1);
          if (i == ch1Channel) {
            digitalWrite(RELAY_1_PORT, HIGH);
            timer.in(RELAY_PULSE_DURATION, relay1Off);
            Serial.println("Relay 1 ON");
          }
          if (i == ch2Channel) {
            digitalWrite(RELAY_2_PORT, HIGH);
            timer.in(RELAY_PULSE_DURATION, relay2Off);
            Serial.println("Relay 2 ON");
          }
        }
      }
    }
    receiver.resetAvailable();
  }
}

// ***** LOCAL FUNCTIONS *****
static bool readDipSwitch(void *) 
{
  uint8_t addr = 0;
  for (int i = 0; i < DIP_SWITCH_INPUTS_CNT; i++) {
    if (!digitalRead(dipSwitchPins[i])) {
      addr |= 1 << i;
    }
  }
  ch1Channel = addr & 0x7;
  ch2Channel = (addr >> 3) & 0x7;
  return true; // to repeat the action - false to stop
}

static bool relay1Off(void *)
{
  digitalWrite(RELAY_1_PORT, LOW);
  Serial.println("Relay 1 OFF");
  return true;
}

static bool relay2Off(void *)
{
  digitalWrite(RELAY_2_PORT, LOW);
  Serial.println("Relay 1 OFF");
  return true;
}

static bool buildinLedBlink(void *)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  return true;
}
