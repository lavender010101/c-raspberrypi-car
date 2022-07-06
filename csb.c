#include "pca9685.h"
#include <stdio.h>
#include <stdlib.h>
#include <softPwm.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <signal.h>
#define PIN_BASE 300
#define MAX_PWM 4096
#define HERTZ 50
#define BUFSIZE 512
#define max(x,y) ((x)>(y)? (x):(y))
#define min(x,y) ((x)<(y)? (x):(y))

//
#define bto 24

//Ñ­¼£¸ÐÓ¦ 
#define f_r 23//ÓÒµÆ 
#define f_m 24//ÖÐ¼äµÆ 
#define f_l 25//×óµÆ

//³¬Éù²¨²â¾à 
#define trig 28//Æô¶¯ÐÅºÅ·¢Éä¶Ë 
#define echo 29//·µ»ØÐÅºÅ½ÓÊÜ¶Ë
 
//ºìÍâ±ÜÕÏ
#define red_l  26// ºìÍâ×ó¸ÐÓ¦µÆ 
#define red_r  27//ºìÍâÓÒ¸ÐÓ¦µÆ 

//×ó±ßÁ½¸öÂÖ×Ó 
#define L_Eble  1
#define ain1  3
#define ain2  2

//ÓÒ±ßÁ½¸öÂÖ×Ó 
#define R_Eble  4
#define bin1  6
#define bin2  5

void init();
void left_drive_up(int v);
void left_drive_down(int v);
void right_drive_up(int v);
void right_drive_down(int v);
void End(int sign);
void Stop();

float getdist();
float avoid_f();//Ç°·½±ÜÕÏ 
float avoid_l();//×ó·½±ÜÕÏ 
float avoid_r();//ÓÒ·½±ÜÕÏ 
void PWM_write(int servol, float x);
int myservol = 12;//¶æ»ú 

int main(){
    float distf,distl,distr;
    // int time = 1;
    // char buf[BUFSIZE]={0xff,0x00,0x00,0x00,0xff};
    int fd = pca9685Setup(PIN_BASE,0x40,HERTZ);
    if(fd < 0){
    	printf("reset error\n");
    	return fd;
	}
	pca9685PWMReset(fd);
	init();
 	  signal(SIGINT,End);	
    
    PWM_write(myservol,90);
	while(1){
		
		distf = avoid_f();
		if( distf < 20 ){
			Stop();
			delay(20);
			distl = avoid_l();
			distr = avoid_r();
			if( distl < 20 && distr < 20 ){
			   right_drive_up(25);
			   left_drive_down(25);
			}
		    else if( distl < distr){
		    	right_drive_down(30);
		    	left_drive_up(30);
			}
			else {
				right_drive_up(30);
		    	left_drive_down(30);
			}
		}
		else {
			   	right_drive_up(30);
		    	left_drive_up(30);
		}
	}
    	
	return 0;
}

float getdist(){
	float dist;
	struct timeval t1;
	struct timeval t2;
	long t1_us;
	long t2_us;
	digitalWrite(trig,0);
	delayMicroseconds(2);
	digitalWrite(trig,1);
	delayMicroseconds(10);
	digitalWrite(trig,0);
	while(!(digitalRead(echo)==1))
	  gettimeofday(&t1,NULL);
	while(!(digitalRead(echo)==0))
	  gettimeofday(&t2,NULL);
	t1_us = t1.tv_sec * 1000000 + t1.tv_usec;
	t2_us = t2.tv_sec * 1000000 + t2.tv_usec;
	dist = (float)(t2_us-t1_us) / 1000000 * 34000 /2;
//	printf("dist = %0.2f\n",dist);
 return dist;
}

float avoid_f(){
	float dist_f;
	PWM_write(myservol,90);
	delay(500);
	dist_f = getdist();
	return dist_f;
}

float avoid_l(){
	float dist_l;
	PWM_write(myservol,175);
	delay(500);
	dist_l = getdist();
	return dist_l;
}

float avoid_r(){
	float dist_r;
	PWM_write(myservol,175);
	delay(500);
	dist_r = getdist();
	return dist_r;
}

int calcTicks(float impulseMs, int hertz)
{
       float cycleMs = 1000.0f / hertz;
       return (int)(MAX_PWM * impulseMs / cycleMs + 0.5f);
}
//¶æ»úÒ¡Í·
void PWM_write(int servonum,float x)
{
  float y;
  int tick;
  y=x/90.0+0.5;
  y=max(y,0.5);
  y=min(y,2.5);
  tick = calcTicks(y, HERTZ);
  pwmWrite(PIN_BASE+servonum,tick);
}

void init(){
    wiringPiSetup();
    pinMode(bto,INPUT);
    
    softPwmCreate(L_Eble,0,100);
	pinMode(ain1,OUTPUT);
	pinMode(ain2,OUTPUT);
	
	softPwmCreate(R_Eble,0,100);
 	pinMode(bin1,OUTPUT);
	pinMode(bin2,OUTPUT);
	//Ñ­¼£¸ÐÓ¦µÆ 
	pinMode(f_l,INPUT);
	pinMode(f_r,INPUT);
	//ºìÍâ¸ÐÓ¦µÆ
	pinMode(red_l,INPUT);
	pinMode(red_r,INPUT); 
	//³¬Éù²¨¶Ë¿Ú
	 pinMode(trig,OUTPUT);
	 pinMode(echo,INPUT); 
}

//×óÂÖÍùÇ° 
void left_drive_up(int v){
	softPwmWrite(L_Eble,v);
	digitalWrite(ain1,1);
	digitalWrite(ain2,0);
//	delay(t);
}

//×óÂÖÍùºó 
void left_drive_down(int v){
	softPwmWrite(L_Eble,v);
	digitalWrite(ain1,0);
	digitalWrite(ain2,1);
	//delay(t);
}

//ÓÒÂÖÍùÇ°
 void right_drive_up(int v){
	softPwmWrite(R_Eble,v);
	digitalWrite(bin1,1);
	digitalWrite(bin2,0);
//	delay(t);
}
//ÓÒÂÖÍùºó 
 void right_drive_down(int v){
	softPwmWrite(R_Eble,v);
	digitalWrite(bin1,0);
	digitalWrite(bin2,1);
//	delay(t);
}

void End(int sign){
	Stop();
	exit(0);
}
void Stop(){
	digitalWrite(ain1,0);
	digitalWrite(ain2,0);
  softPwmWrite(L_Eble,0);
	digitalWrite(bin1,0);
	digitalWrite(bin2,0);
  softPwmWrite(R_Eble,0);
}
