#include "pca9685.h"
#include <errno.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#define PIN_BASE 300
#define MAX_PWM 4096
#define HERTZ 50
#define BUFSIZE 512
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define Trig 28
#define Echo 29
void PWM_write(int servonum, float x);
int PWMA = 1;
int AIN2 = 2;
int AIN1 = 3;
int PWMB = 4;
int BIN2 = 5;
int BIN1 = 6;
/*******************舵机定义*************************
*****************************************************/
int ultrasonic_servo = 0; // 舵机定义 超声波摇头
void forward(unsigned int speed, unsigned int t_time) {
  digitalWrite(AIN2, 0);
  digitalWrite(AIN1, 1);
  softPwmWrite(PWMA, speed);
  digitalWrite(BIN2, 0);
  digitalWrite(BIN1, 1);
  softPwmWrite(PWMB, speed);
  delay(t_time);
}
void stop(unsigned int t_time) {
  digitalWrite(AIN2, 0);
  digitalWrite(AIN1, 0);
  softPwmWrite(PWMA, 0);
  digitalWrite(BIN2, 0);
  digitalWrite(BIN1, 0);
  softPwmWrite(PWMB, 0);
  delay(t_time);
}
void backward(unsigned int speed, unsigned int t_time) {
  digitalWrite(AIN2, 1);
  digitalWrite(AIN1, 0);
  softPwmWrite(PWMA, speed);
  digitalWrite(BIN2, 1);
  digitalWrite(BIN1, 0);
  softPwmWrite(PWMB, speed);
  delay(t_time);
}
void turn_left(unsigned int speed, unsigned int t_time) {
  digitalWrite(AIN2, 1);
  digitalWrite(AIN1, 0);
  softPwmWrite(PWMA, speed);
  digitalWrite(BIN2, 0);
  digitalWrite(BIN1, 1);
  softPwmWrite(PWMB, speed);
  delay(t_time);
}
void turn_right(unsigned int speed, unsigned int t_time) {
  digitalWrite(AIN2, 0);
  digitalWrite(AIN1, 1);
  softPwmWrite(PWMA, speed);
  digitalWrite(BIN2, 1);
  digitalWrite(BIN1, 0);
  softPwmWrite(PWMB, speed);
  delay(t_time);
}
void ultraInit(void) {
  pinMode(Echo, INPUT);
  pinMode(Trig, OUTPUT);
}
float distMeasure(void) {
  struct timeval tv1;
  struct timeval tv2;
  long start, stop;
  float dist;
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  // 发出超声波脉冲
  digitalWrite(Trig, LOW);
  while (!(digitalRead(Echo) == 1))
    ;
  gettimeofday(&tv1, NULL);
  // 获取当前时间
  while (!(digitalRead(Echo) == 0))
    ;
  gettimeofday(&tv2, NULL);
  // 获取当前时间
  start = tv1.tv_sec * 1000000 + tv1.tv_usec;
  // 微秒级的时间
  stop = tv2.tv_sec * 1000000 + tv2.tv_usec;
  dist = (float)(stop - start) / 1000000 * 34990 / 2;
  // 求出距离
  return dist;
}
/******************超声波前方向避障***********************/
float front_detection() {
  float dis_f;
  PWM_write(ultrasonic_servo, 90);
  delay(500);
  dis_f = distMeasure();
  return dis_f;
}
/******************超声波左方向避障***********************/
float left_detection() {
  float dis_l;
  PWM_write(ultrasonic_servo, 175);
  delay(500);
  dis_l = distMeasure();
  return dis_l;
}
/*******************右侧方向避障***************************/
float right_detection() {
  float dis_r;
  PWM_write(ultrasonic_servo, 5);
  delay(500);
  dis_r = distMeasure();
  return dis_r;
}
int main(int argc, char *argv[]) {
  float dis1, dis2, dis3;
  // char buf[BUFSIZE] = {0xff, 0x00, 0x00, 0x00, 0xff};
  // int time = 1;
  /*RPI*/
  wiringPiSetup();
  /*WiringPi GPIO*/
  pinMode(1, OUTPUT); // PWMA
  pinMode(2, OUTPUT); // AIN2
  pinMode(3, OUTPUT); // AIN1
  pinMode(4, OUTPUT); // PWMB
  pinMode(5, OUTPUT); // BIN2
  pinMode(6, OUTPUT); // BIN1
  ultraInit();        // 超声波初始化
  /*PWM output*/
  softPwmCreate(PWMA, 0, 100);
  softPwmCreate(PWMB, 0, 100);
  // Setup with pinbase 300 and i2c location 0x40
  int fd = pca9685Setup(PIN_BASE, 0x40, HERTZ);
  if (fd < 0) {
    printf("Error in setup\n");
    return fd;
  }
  // Reset all output
  pca9685PWMReset(fd);
  printf("ok");
  while (1) {
    dis1 = front_detection();
    if (dis1 < 40) {
      stop(200);
      backward(50, 500);
      stop(200);
      dis2 = left_detection();
      dis3 = right_detection();
      if (dis2 < 40 && dis3 < 40) {
        turn_left(50, 1000);
      } else if (dis2 > dis3) {
        turn_left(50, 300);
        stop(100);
      } else {
        turn_right(50, 300);
        stop(100);
      }
    } else {
      forward(60, 0);
    }
  }
  return 0;
}
/**
 * Calculate the number of ticks the signal should be high for the required
 * amount of time
 */
int calcTicks(float impulseMs, int hertz) {
  float cycleMs = 1000.0f / hertz;
  return (int)(MAX_PWM * impulseMs / cycleMs + 0.5f);
}
/********************  舵机摇头*****************************/
void PWM_write(int servonum, float x) {
  float y;
  int tick;
  y = x / 90.0 + 0.5;
  y = max(y, 0.5);
  y = min(y, 2.5);
  tick = calcTicks(y, HERTZ);
  pwmWrite(PIN_BASE + servonum, tick);
}
