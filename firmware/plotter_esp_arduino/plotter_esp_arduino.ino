#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "5817e2d6-642e-4523-8098-259279a002f2"
#define CHARACTERISTIC_UUID "5ed7832a-308b-427b-b99c-77a56e362349"


static const int SERVO_FREQ       = 50;
static const int SERVO_RESOLUTION = 14;
static const int SERVO_MIN_US     = 544;
static const int SERVO_MAX_US     = 2400;
static const int PERIOD_US        = 20000;


int YIN1 = 4;
int YIN2 = 5;
int YIN3 = 6;
int YIN4 = 7;

int XIN1 = 15;
int XIN2 = 16;
int XIN3 = 17;
int XIN4 = 18;

int SR1 = 2;

int step_delay = 4;

int seq[4][4] = {
  {1,0,0,0},
  {0,1,0,0},
  {0,0,1,0},
  {0,0,0,1}
};

static uint32_t us2ticks(int us) {
  return (uint32_t)((uint64_t)us * ((1 << SERVO_RESOLUTION) - 1) / PERIOD_US);
}

void feed(int dir){
  for(int i = 0; i < sizeof(seq)/sizeof(seq[0]); i++){
    int j = (dir<0) ? (3-i) : i;
    digitalWrite(YIN1, seq[j][0]);
    digitalWrite(YIN2, seq[j][1]);
    digitalWrite(YIN3, seq[j][2]);
    digitalWrite(YIN4, seq[j][3]);
    delay(step_delay);
  }
}

void head(int dir){
  for(int i = 0; i < sizeof(seq)/sizeof(seq[0]); i++){
    int j = (dir>0) ? (3-i) : i;
    digitalWrite(XIN1, seq[j][0]);
    digitalWrite(XIN2, seq[j][1]);
    digitalWrite(XIN3, seq[j][2]);
    digitalWrite(XIN4, seq[j][3]);
    delay(step_delay);
  }
}

void moveby(int x1, int y1){
  int x0 = 0;
  int y0 = 0;
  int dx = abs(x1 - x0);
  int sx = (x0 < x1) ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = (y0 < y1) ? 1 : -1;
  int error = dx + dy;
  while (true){
    int e2 = 2 * error;
    if (e2 >= dy){
      if (x0 == x1) break;
      error += dy;
      x0 += sx;
      head(sx);
    }
    if (e2 <= dx){
      if (y0 == y1) break;
      error += dx;
      y0 += sy;
      feed(sy);
    }
  }
}

void sr_write(int pin, int ang){
  int n = map(ang, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
  for (int i = 0; i < 25; i++){
    digitalWrite(pin, HIGH);
    delayMicroseconds(n);
    digitalWrite(pin, LOW);
    delayMicroseconds(PERIOD_US-n);
  }

  // ledcWrite(pin, us2ticks(n));
  // delay(500);
}

void sr_attach(int pin){
  pinMode(SR1,  OUTPUT);
  // ledcSetClockSource(LEDC_USE_APB_CLK);
  // ledcAttach(pin, SERVO_FREQ, SERVO_RESOLUTION);
}

char* parse_run(const char* buffer){
  if (buffer[0] == 'l'){
    char *comma = strchr(buffer, ',');
    if (comma != NULL) {
      *comma = '\0';             
      int x = atoi(buffer + 1);    
      int y = atoi(comma + 1);
      moveby(x,y);
      return "OK";
    }
  }else if (buffer[0] == 'Z'){
    int z = atoi(buffer+1);
    sr_write(SR1,z);
    return "OK";
  }
}


class BleCb: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic){
    String value = pCharacteristic->getValue();
    if (value.length() > 0){
      pCharacteristic->setValue(parse_run(value.c_str()));
    }
  }
};

class BleSv : public BLEServerCallbacks {
  void onDisconnect(BLEServer* pServer){
    pServer->getAdvertising()->start();
  }
};

void ble_init(){
  BLEDevice::init("BLEPLOTTER");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BleSv());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic * pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new BleCb());
  pService->start();  
  BLEAdvertising * pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}


const int BUFFER_SIZE = 64;
char buffer[BUFFER_SIZE];
int bufidx = 0;

void setup() {
  Serial.begin(115200);
  pinMode(XIN1, OUTPUT);
  pinMode(XIN2, OUTPUT);
  pinMode(XIN3, OUTPUT);
  pinMode(XIN4, OUTPUT);
  pinMode(YIN1, OUTPUT);
  pinMode(YIN2, OUTPUT);
  pinMode(YIN3, OUTPUT);
  pinMode(YIN4, OUTPUT);
  
  sr_attach(SR1);
  sr_write(SR1,150);
  ble_init();
}


void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      buffer[bufidx] = '\0';
      Serial.print(parse_run(buffer));
      Serial.print("\n");
      bufidx = 0;
    }
    else {
      if (bufidx < BUFFER_SIZE - 1) {
        buffer[bufidx++] = c;
      } else {
        bufidx = 0;
      }
    }
  }
}
