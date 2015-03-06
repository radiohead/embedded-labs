#include <math.h>

#define RED_PIN 15
#define YELLOW_PIN 18
#define GREEN_PIN 19

#define XLERO_X_PIN 5
#define XLERO_Y_PIN 6
#define XLERO_Z_PIN 7

static int x_0 = 2068;
static int y_0 = 2048;
static int z_0 = 1472;

static double acceleration = 0.015;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  
  pinMode(XLERO_X_PIN, INPUT);
  pinMode(XLERO_Y_PIN, INPUT);
  pinMode(XLERO_Z_PIN, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  int x, y, z;
  double result;

  x = analogRead(XLERO_X_PIN);
  y = analogRead(XLERO_Y_PIN);
  z = analogRead(XLERO_Z_PIN);
  result = acceleration * sqrt(pow((x - x_0), 2) + pow((y - y_0), 2) + pow((z - z_0), 2));

  if (result <= 9.81) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
  }
  else if (result <= 12) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);    
  }
  else {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(YELLOW_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
  }
}

