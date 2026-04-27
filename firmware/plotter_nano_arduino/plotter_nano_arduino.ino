#include <Servo.h>

Servo sr;

int YIN1 = 2;
int YIN2 = 3;
int YIN3 = 4;
int YIN4 = 5;

int XIN1 = 9;
int XIN2 = 8;
int XIN3 = 7;
int XIN4 = 6;

int SR1 = 10;

int step_delay = 4;

int seq[4][4] = {
  {1,0,0,0},
  {0,1,0,0},
  {0,0,1,0},
  {0,0,0,1}
};

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



const int BUFFER_SIZE = 64;
char buffer[BUFFER_SIZE];
int index = 0;

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
  sr.attach(SR1);
  sr.write(150);
  
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      buffer[index] = '\0';
      if (buffer[0] == 'l'){
        char *comma = strchr(buffer, ',');
        if (comma != NULL) {
          *comma = '\0';             
          int x = atoi(buffer + 1);    
          int y = atoi(comma + 1);
          moveby(x,y);
          // delay(10);
          Serial.print("OK\n");
        }
      }else if (buffer[0] == 'Z'){
        int z = atoi(buffer+1);
        sr.write(z);
        delay(500);
        Serial.print("OK\n");
      }
      index = 0;
    }
    else {
      if (index < BUFFER_SIZE - 1) {
        buffer[index++] = c;
      } else {
        index = 0;
      }
    }
  }

}
