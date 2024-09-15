#include <RCSwitch.h>
#include <arduino-timer.h>

// ***** DEFIDES *****
#define DIP_SWITCH_INPUTS_CNT   ( 6 )
#define RELAY_ON_TIME           ( 1000 ) // ms
#define RELAY_OFF_TIME          ( 1000 ) // ms
#define RELAY_1_PORT            ( 10 ) // D10
#define RELAY_2_PORT            ( 9 ) // D9

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

class RelaySM {
  typedef enum {
    RELAY_STATE_IDLE,
    RELAY_STATE_ON,
    RELAY_STATE_OFF
  } RelayState_t;

  private:
    unsigned long previousMillis;
    long interval; 
    int relayPort;
    int id;
    RelayState_t state;

  public:
    RelaySM(int relayPort, int id) {
      this->relayPort = relayPort;
      this->id = id;
      this->previousMillis = millis();
      this->state = RELAY_STATE_IDLE;
      pinMode(this->relayPort, OUTPUT);
      digitalWrite(this->relayPort, LOW);
    }

    void handler() {
      unsigned long currentMillis = millis();
      switch( this->state) {
        case RELAY_STATE_ON:
          if (currentMillis - this->previousMillis >= RELAY_ON_TIME) {
            this->state = RELAY_STATE_OFF;
            digitalWrite(this->relayPort, LOW);
            this->previousMillis = millis();
            Serial.print("Relay ");
            Serial.print(this->id);
            Serial.println(" OFF");
          }
          break;
        case RELAY_STATE_OFF:
          if (currentMillis - this->previousMillis >= RELAY_OFF_TIME) {
            this->state = RELAY_STATE_IDLE;
          }
          break;
        default:
          break;
      };
    }

    void trig() {
      if (this->state == RELAY_STATE_IDLE) {
        this->state = RELAY_STATE_ON;
        digitalWrite(this->relayPort, HIGH);
        this->previousMillis = millis();
        Serial.print("Relay ");
        Serial.print(this->id);
        Serial.println(" ON");
      }
    }
};

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
auto timer1s = timer_create_default();
RelaySM relay1(RELAY_1_PORT, 1);
RelaySM relay2(RELAY_2_PORT, 2);
uint8_t ch1Channel;
uint8_t ch2Channel;

// ***** GLOBAL FUNCTIONS *****
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < DIP_SWITCH_INPUTS_CNT; i++) {
    pinMode(dipSwitchPins[i], INPUT_PULLUP);
  }
  readDipSwitch();
  Serial.begin(9600);
  receiver.enableReceive(0);
  timer1s.every(1000, [](void*){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    readDipSwitch();
    return true;
  });
}

void loop() {
  timer1s.tick();
  relay1.handler();
  relay2.handler();
  if (receiver.available()) {
    if (receiver.getReceivedBitlength() == 24) {
      uint32_t btnCode = receiver.getReceivedValue();
      for(int i = 0; i < BTN_COUNT; i++) {
        if(btnCode == buttonsCodeMap[i]) {
          Serial.print("Pressed button ");
          Serial.println(i+1);
          if (i == ch1Channel) {
            relay1.trig();
          }
          if (i == ch2Channel) {
            relay2.trig();
          }
        }
      }
    }
    receiver.resetAvailable();
  }
}

static void readDipSwitch()
{
  uint8_t addr = 0;
  for (int i = 0; i < DIP_SWITCH_INPUTS_CNT; i++) {
    if (!digitalRead(dipSwitchPins[i])) {
      addr |= 1 << i;
    }
  }
  ch1Channel = addr & 0x7;
  ch2Channel = (addr >> 3) & 0x7;
}

