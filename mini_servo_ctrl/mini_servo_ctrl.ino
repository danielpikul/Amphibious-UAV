#include <Servo.h>


Servo left_motor_ctrl;
Servo right_motor_ctrl;
const int servo_left_out  = 9;
const int servo_right_out = 10;
const int APM_in          = 4;

void setup() {
  // put your setup code here, to run once:
  pinMode(APM_in, INPUT);
  pinMode(13, OUTPUT);
  left_motor_ctrl.attach(servo_left_out);
  right_motor_ctrl.attach(servo_right_out);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(APM_in)) {
    left_motor_ctrl.write(178);
    right_motor_ctrl.write(2);
    digitalWrite(13, LOW);
  }
  else {
    left_motor_ctrl.write(90);
    right_motor_ctrl.write(90);
    digitalWrite(13, HIGH);
  }
  delay(2);
}
