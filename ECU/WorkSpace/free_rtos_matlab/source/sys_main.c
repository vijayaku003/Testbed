
#include "sys_pmu.h"
#include "sys_common.h"
#include "system.h"
#include "stdint.h"
#include "os_projdefs.h"
#include "can.h"
#include "esm.h"
#include "sci.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sys_core.h"
#include "sys_vim.h"

/* Include FreeRTOS scheduler files */
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_semphr.h"
#include "os_portmacro.h"

/* Include HET header file - types, definitions and function declarations for system driver */
#include "het.h"
#include "gio.h"

/* Define Task Handles */

xTaskHandle canTaskTcb;
xTaskHandle CAWarningTaskTcb;
xTaskHandle CATcb;
xTaskHandle steerTaskTcb;
xTaskHandle radarTaskTcb;
xTaskHandle vehToVehTaskTcb;
xTaskHandle laneChangePilotTcb;
xTaskHandle cruiseControlTaskTcb;
xTaskHandle CANPeriodicTxTaskTcb;
xTaskHandle blinkLEDTaskTcb;

int *intvectreg = (int *) 0xFFFFFE70;
int *intindexreg = (int *) 0xFFFFFE00;

#define LOW  		0
#define HIGH 		1
#define DATA_LEN	8

#define LED_TASK 0

#define UART_ABS scilinREG //Tx ABS data
#define UART_STEER sciREG //Tx steering data
#define UART_STACK_SIZE	  ( ( unsigned portSHORT ) 256 )

//#define f_HCLK (float) 180.0 // f in [MHz]; HCLK (depends on device setup)



//uint8_t statsBuffer[40*5]; // Enough space for 5 tasks - this needs to be global, since task stack is too small

/*---------------Mode/Scenario/ResourceShortage Change Flags----------------------*/

// Resource Shortage Type Flags




uint8 rx_str_data[5];
uint8 rx_data[DATA_LEN];
char rxMbox1[5];
char rxMbox2[5];
int rxIntMbox1;

SemaphoreHandle_t canIntRxSem;
SemaphoreHandle_t accSem;
SemaphoreHandle_t ultrasonicSem;
SemaphoreHandle_t uartAccTxSem;
SemaphoreHandle_t uartUltrasonicTxSem;
SemaphoreHandle_t radarSem;
SemaphoreHandle_t cruiseControlSem;
SemaphoreHandle_t vehToVehSem;

uint32 mailBox;

/* Degradation cruise control task PID ; ONOFF ; Lateral Control */

//int PID = 1;
//int ONOFF = 0;
//int ISA = 1;

int ccdegradePeriod = 0;
int degradeSteer =0;

static int V2Venabled = 1;
static int degradeRadarPeriod = 1;
static int degradeV2VPeriod = 1;
static float curveVelocity = 18.0;


/* Curve Coordinates for Oval track E-Track 5 */

float curve_1_x_start = 314; // 720 < follow_pos[0] < 710 entering curve1
//float curve_2_x_start = 388; // 729 < follow_pos[0] < 720  entering curve2
float curve_2_x_start = 387.5; // 655 < follow_pos[0] < 670 entering curve3
float curve_3_x_start = 114; //  316 < follow_pos[0] < 330  entering curve4
float curve_4_x_start = 40.3;

//float curve_5_x_start = 40; //  188 < follow_pos[0] < 200 entering curve5
//float curve_6_x_start = 40; // 7 < follow_pos[0] < 20 entering curve6
//float curve_7_x_start = 114; //  90 < follow_pos[0] < 80  entering curve7

float curve_1_y_start = 14; // 720 < follow_pos[0] < 710 entering curve1
//float curve_2_y_start = 181; // 729 < follow_pos[0] < 720  entering curve2
float curve_2_y_start = 316.8; // 655 < follow_pos[0] < 670 entering curve
float curve_3_y_start = 484; //  316 < follow_pos[0] < 330  entering curve4
float curve_4_y_start = 181;

//float curve_5_y_start = 316; //  188 < follow_pos[0] < 200 entering curve5
//float curve_6_y_start = 181; // 7 < follow_pos[0] < 20 entering curve6
//float curve_7_y_start = 13; //  90 < follow_pos[0] < 80  entering curve7

int curve_xpos[] = {314,388,114,40};
int curve_ypos[] = {14,316,316,13};
int curve_length[] = {231,231,231,231};

/* Variable for implementing Dynamic Speed Adaptation according to the paper
 *
 * Paper : Dynamic Speed Adaptation for Path Tracking Based on Curvature Information and Speed Limits by
 * Citlalli Gamez Serna and Yassine Ruichek
 *
 * */

int curvePassedIndex = 4;  // default
int curveAheadIndex = 0;
int curveCurrentIndex =0;
int maintainCurveSpeed = 0;

float foltravelledDistance=0;


int curveaheadxPos = 0;
int curveaheadyPos = 0;

int curvepassedxPos = 0;
int curvepassedyPos = 0;

// Track : Oval track A-speedway
//static int curve1Start = 255;
//static int curve2Start = 545;
//static int curve3Start = 1200;
//static int curve4Start = 1490;

static int curve1Start = 225;
static int curve2Start = 515;
static int curve3Start = 1160;
static int curve4Start = 1450;

static int curve1End = 411;
static int curve2End = 713;
static int curve3End = 1370;
static int curve4End = 1656;


// Track : Oval track t track 5
/*
static int curve1Start = 100;
static int curve2Start = 480;
static int curve3Start = 900;
static int curve4Start = 1290;

static int curve1End = 310;
static int curve2End = 690;
static int curve3End = 1150;
static int curve4End = 1500;

*/

static int curve1TrigDist;
static int curve2TrigDist;
static int curve3TrigDist;
static int curve4TrigDist;

int distanceBetweenCurves[] = {379,431,379,431}; // 0th index gives distance between curve 0 and curve 1;  3th index gives distance between curve 3 and curve 0
int distanceValueCurve[] = {100, 479, 910, 1290};
int curveDistanceTrigger[] = {90,459,900,1280};

int distanceCurveMap[] = {0,0,0,0 }; // Map each curve  to 'distance' variable value - value should be got from scope after running simulation



void delay_ms(unsigned int  delay);
int calculateWheelSlip(int *wheelSpin);
int calculateSteerSensitivity(int pubSpeed);

//MATLAB Applications
void send_float (float arg);
float numericalDerivativeDist(float dist);
float accelerationDemand(float dist, float follower_velX, float headway);
float accelerationDemand2(float dist, float follower_velX, float leader_velX, float headway);
void simpleThrottleBrake(float dist, float follower_velX, float leader_velX, float headway);
int calculateEuclidianDistance(float follower_pos[3], float leader_pos[3]);
float calculateEuclidianDistanceCurve(float xpos1, float xpos2, float ypos1, float ypos2);

int checkFieldOfViewRadar(float follower_pos[3], float leader_pos[3], float dist);
int checkFieldOfViewV2V(float dist);

float numericalDerivativeKP(float kp);
float numericalDerivativeKD(float kd);
float steering (float angular_vel, float headingError, float lateralError);
void accelerationControl(float accelDemand, float follower_accelX, int sensor);
void accelerationControl_radar(float accelDemand, float follower_accelX);
void accelerationControl_isa(float accelDemand, float follower_accelX);
void accelerationControlVeh(float accelDemand, float follower_accelX);
double int_simpson(double from, double to, double n, double m, double c);
float min(float a, float b);
float max(float a, float b);

//MATLAB Variables8
//Inputs
#define highCrit 1
float prevTime, currentTime;
float timeStep;
static float lead_pos[3], lead_pos_delayed[3];
static float fol_pos[]={179,10,0};
static float folo_pos_previous[] = {179,10,0};
//static float lead_pos_x, lead_pos_y, lead_pos_z, fol_pos_x, fol_pos_y, fol_pos_z;
static float fol_vel, lead_vel, lead_vel_delayed;
static float fol_ang_vel_z, fol_head_error, fol_lat_error, fol_acc_x, fol_yaw, dummy;
static uint32 ans,ans2;
//outputs
static int vehicleAhead;
static int steerCount;
static int radarCount;
static int v2vCount;

static int canOverrun = 0;
static int canOverrunCount=0;

static int ccOverrun = 0;
static int ccOverrunCount = 0;

static int radarOverrun = 0;
static int radarOverrunCount = 0;

static int caOverrun = 0;
static int caOverrunCount = 0;

static int steerOverrun = 0;
static int steerOverrunCount = 0;

static int v2vOverrun = 0;
static int v2vOverrunCount = 0;


static float distApartVV, accelDemand, accelDemand_ISA;
static int distApartRadar;
static float throttle, brake, steer;

static float throttle_radar, throttle_v2v, throttle_isa;
static float gear=0;

//Ultrasonic Variables
static int overrunTrigger;


static float curvatureThreshold=0.02;
static float downcurvatureThreshold=-0.02;

static int xPosIndex=0;
static int yPosIndex=0;

static unsigned long  WCET[] = {300000, 300000, 300000, 300000, 300000 }; // CAN, CC, V2V, Steer, CA

static int state = 0;

static uint32_t processorUtilization=0;


void vApplicationIdleHook()
{
    systemState = 0;
    budgetOverrunDetected = 0;
    int i =1;
    for(i=1;i<=configNumberOfTasks;i++){
        globalBehaviourTable[i][0] = 3; // Normal Behaviour is assigned
        globalRuntimeBudgetTable[i][0] = globalBudgetTable[i][0]; // Normal Budget is assigned
    }
}

void initializeProfiler()
{
	/* Enable PMU Cycle Counter for Profiling */

	_pmuInit_();
	_pmuEnableCountersGlobal_();
	_pmuResetCycleCounter_();
	_pmuStartCounters_(pmuCYCLE_COUNTER);

}

uint32_t getProfilerTimerCount()
{
	return _pmuGetCycleCount_();
}



inline void sciDisplayText(sciBASE_t *sci, uint8 *text,uint32 length)
{
	while(length--)
	{
		while ((sci->FLR & 0x4) == 4); /* wait until busy */
		sciSendByte(sci,*text++);      /* send out text   */
	};
}

unsigned long getWCET(long index){
	return WCET[index];
}

void runTimeUtilization(float utilization){
	processorUtilization =  utilization;
}


// Pass priority of a task and budget consumed by a task as input parameters

void stateServiceLevelManager(unsigned long priority, uint32_t runTime){
	if(priority > 2){
		if(runTime > getWCET(priority)){
			// Change to safe state;
			state = priority;
		}
	}
}
// Vehicle to Vehicle

void vehToVehTask(void *pvParameters)
{
	//For periodicity
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	/*while(1)
	{

		if(degradeV2VPeriod == 0){
		vTaskDelayUntil(&xLastWakeTime, 2000); //3ms
		}

			else{
				vTaskDelayUntil(&xLastWakeTime, 10000); //3ms

			}*/
	if(V2Venabled == 1){
		if(xSemaphoreTake(vehToVehSem, 0) == pdTRUE)
		{
			//distApartVV = calculateEuclidianDistance(fol_pos, lead_pos_delayed);
			vehicleAhead = checkFieldOfViewV2V(distApartRadar);
		}
	}

	vTaskDelete(vehToVehTaskTcb);

	//}
}

// Blink LED Task :

void blinkLEDTask(void *pvParameters){

	//For periodicity
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		vTaskDelayUntil(&xLastWakeTime, 10); //3ms

		gioSetPort(hetPORT1, gioGetPort(hetPORT1) ^ 0x80000021);


	}
}

//Collision Avoidance Task :

void CATask(void *pvParameters){
	//For periodicity
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	int i=0;
	/*while(1)
	{
		vTaskDelayUntil(&xLastWakeTime, 2000); //3ms
		// CANStart=xTaskGetTickCount ();*/

	// Apply emergency brakes
	if (distApartRadar<10 )
	{
		if(fol_vel > lead_vel){
			brake = 1;
			throttle = 0;
			send_float(throttle);
			send_float(steer);
			send_float(brake);
			//send_float(processorUtilization);
		}
	}

	// Issue warning

	if (distApartRadar<40)
	{
		if(fol_vel > 0){
			//gioSetPort(hetPORT1, gioGetPort(hetPORT1) ^ 0x80000021);
		}
	}

	if(caOverrun == 1&& caOverrunCount<2){
		int ca=0;
		caOverrunCount++;
		for(ca =0;ca<20000;ca++){

		}
	}

	vTaskDelete(CATcb);
	//	}

}

// Steer Control :

void steerTask(void *pvParameters)
{
	//For periodicity
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	//while(1)
	//{

	//		vTaskDelayUntil(&xLastWakeTime, 2000); //5ms

	//if(dummy > 0.005 && steerCount < 1000){
	//		vTaskDelayUntil(&xLastWakeTime, 6000); //5ms
	//		steerCount++;


	//else{

	//vTaskDelayUntil(&xLastWakeTime, 2000); //5ms

	//	if(fol_vel < 19){
	//		V2Venabled = 0;
	//		ISA = 0;
	//	}

	/*if(fol_vel > 20){
	degradeV2VPeriod = 1;
	}*/

	//	if(fol_vel < 19 && V2Venabled == 0 ){
	//		V2Venabled = 1;
	//		PID = 1;
	//		ONOFF = 0;
	//		ISA = 0;
	//		degradeV2VTemp = 0;
	//ONOFF = 1;
	//	}

	/*if(foltravelledDistance > 1150.0 && foltravelledDistance< 1250){

	V2Venabled = 0;
	degradeV2VTemp = 1;
	v2vCount++;

}

if(v2vCount > 180){
	V2Venabled = 1;
	PID = 0;
	ONOFF = 1;
}
	 */

	//if(v2vCount > 200 && fol_vel <18.5 ){
	//	V2Venabled = 1;
	//	degradeV2VTemp = 0;
	//}

	//	if(dummy > 0.005 && v2vCount < 1000){
	//		degradeV2V=1;
	//	radarEnable = 0;
	//radarCount++;
	//		v2vCount++;
	//		if(v2vCount > 1000){
	//			PID = 0;
	//		ONOFF = 1;
	//	}
	//	}
	//	else{
	//radarEnable = 1;
	//		degradeV2V=0;

	//	}



	steer = steering(fol_ang_vel_z, fol_head_error, fol_lat_error);

	if(steerOverrun == 1&& steerOverrunCount<2){
		int steer=0;
		for(steer=0;steer<8000;steer++){
		}
	}
	vTaskDelete(steerTaskTcb);
	//}

}


void radarTask(void *pvParameters)
{
	//For periodicity
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	/*while(1)
	{
		if(degradeRadarPeriod == 1){
		vTaskDelayUntil(&xLastWakeTime, 2000); //3ms
		}
		else{
			vTaskDelayUntil(&xLastWakeTime, 10000); //3ms
		}
	 */
	if(xSemaphoreTake(radarSem, 0) == pdTRUE)
	{

		//	if(radarEnable == 1){
		//Check for radar
		distApartRadar = calculateEuclidianDistance(fol_pos, lead_pos);
		if(V2Venabled != 1){
			vehicleAhead = checkFieldOfViewRadar(fol_pos, lead_pos, distApartRadar);
		}
		//	}

		if(xSemaphoreGive(vehToVehSem) == pdTRUE)
		{
			//Go for v-v task
		}

		//	if(fol_vel > 24){
		//		ISA = 0;
		//		V2Venabled = 0; // Degrade V2V till velocity is reduced from 28 to 20 m/s before disabling ISA
		//degradeV2VTemp = 1;
		//		v2vCount++;
		//	}
	}

	if(radarOverrun == 1 && radarOverrunCount<2){
		int radar =0;
		for(radar =0;radar<8000;radar++){

		}
	}
	//}
	vTaskDelete(radarTaskTcb);
}


//Cruise Control Algorithm   :

void cruiseControlTask(void *pvParameters)
{
	//For periodicity
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	/*while(1)
	{
		if(foltravelledDistance>1000.0 && foltravelledDistance<1200.0) {
			//degradePeriod = 1;
		}
		if(ccdegradePeriod == 0){
			vTaskDelayUntil(&xLastWakeTime, 2000); //with 100 Khz frequency, 1 ticks of ostickctr =  0.01 ms; 1000 ticks = 10 ms
		}
		else{
			vTaskDelayUntil(&xLastWakeTime, 10000); //with 100 Khz frequency, 1 ticks of ostickctr =  0.01 ms; 1000 ticks = 10 ms
		}*/

	if(xSemaphoreTake(cruiseControlSem, 0) == pdTRUE)
	{

		/************************ Implementation of acceleration based on Radar ************************/

		//	distApartRadar = calculateEuclidianDistance(fol_pos, lead_pos);
		//	vehicleAhead = checkFieldOfView(fol_pos, lead_pos, distApartRadar);

		//				if(vehicleAhead ==1){

		// commented for ON-OFF

		//if(foltravelledDistance > 1600.0 && foltravelledDistance < 1650.0){

		//	V2Venabled = 0;
		//}
		gioSetPort(hetPORT1, gioGetPort(hetPORT1) ^ 0x80000021);

		if(PID == 0){

			//		if(ONOFF == 1 && maintainCurveSpeed==0){
			if(vehicleAhead == 1){
				simpleThrottleBrake(distApartRadar,  fol_vel, lead_vel, 1.0);
			}
			else{
				accelDemand = -((fol_vel - 28.0));
				accelerationControl(accelDemand, fol_acc_x,1);
			}
			/*if (distApartRadar<15){
					brake = 0.5;
				}
				else
				{
					throttle = 1.0;
					brake = 0;
				}*/

		}

		/*			if (distApartRadar<25)
												brake = 0.5;
											else
												{
													throttle = 0.5;
													brake = 0;
												}
										}

		 */

		//		else if(PID == 1 && maintainCurveSpeed==0){
		else if(PID == 1){

			if(vehicleAhead == 1){
				accelDemand = accelerationDemand2((10-distApartRadar), fol_vel, lead_vel, 0.4);
				accelerationControl(accelDemand, fol_acc_x,1);
			}


			/*else if(vehicleAhead == 0 && V2Venabled == 1){
				//	accelDemand = accelerationDemand2((10-distApartVV), fol_vel, lead_vel, 0.8);
					accelDemand = accelerationDemand2((10-distApartRadar), fol_vel, lead_vel, 0.8);
					accelerationControl(accelDemand, fol_acc_x,1);
					//accelerationControlVeh(accelDemand, fol_acc_x);
				}
			 */
			//		else if (degradeV2VTemp == 1){
			//throttle = 2.0/(1.0+exp(fol_vel-20.0));
			//brake = 0;
			//			accelDemand = accelerationDemand2((10-distApartVV), fol_vel, 19.0, 0.8);
			//			accelerationControl(accelDemand, fol_acc_x,1);
			//		}


			else { // Constant velocity
				//throttle = 2.0/(1.0+exp(fol_vel-25.0));
				//brake = 0;
				accelDemand = -((fol_vel - 28.0));
				accelerationControl(accelDemand, fol_acc_x,1);
				//		}

				//accelerationControl_radar(accelDemand, fol_acc_x);

			}
		}

		if(ISA == 1 && fol_vel > curveVelocity){

			int i;
			for(i =0; i<4; i++){
				curve1TrigDist = curve1Start - ((fol_vel*fol_vel) - (18*18))/(4) ;
				curve2TrigDist = curve2Start - ((fol_vel*fol_vel) - (18*18))/(4) ;
				curve3TrigDist = curve3Start - ((fol_vel*fol_vel) - (18*18))/(4) ;
				curve4TrigDist = curve4Start - ((fol_vel*fol_vel) - (18*18))/(4) ;
			}



			// Curve 1 :
			// integral Distance value at curve  1 start : 400 ; curve 1 end : 587.5;

			// if(foltravelledDistance > 86 && foltravelledDistance < 310 ){
			if(foltravelledDistance > curve1TrigDist && foltravelledDistance < curve1End ){
				//if(integralDistance > 400 ){
				curveCurrentIndex = 1;
				maintainCurveSpeed = 1;
				curveAheadIndex = 2;
			}


			// Curve 2 :
			// integral Distance value at curve  2 start : 709 ; curve 2 end : 897;

			//	else if(foltravelledDistance > 410 && foltravelledDistance < 690 ){
			else if(foltravelledDistance > curve2TrigDist && foltravelledDistance < curve2End ){
				curveCurrentIndex = 2;
				maintainCurveSpeed = 1;
				curveAheadIndex = 3;
			}

			// Curve 3 :
			// integral Distance value at curve  3 start : 1059 ; curve 3 end : 1247;

			// else if(foltravelledDistance > 850 && foltravelledDistance < 1150 ){
			else if(foltravelledDistance > curve3TrigDist && foltravelledDistance < curve3End ){
				curveCurrentIndex = 3;
				maintainCurveSpeed = 1;
				curveAheadIndex = 4;
			}

			// Curve 4 :
			// integral Distance value at curve  4 start : 1369  ; curve 4 end : 1557 ;


			//else if(foltravelledDistance > 1210 && foltravelledDistance < 1500 ){

			else if(foltravelledDistance > curve4TrigDist && foltravelledDistance < curve4End ){
				curveCurrentIndex = 4;
				maintainCurveSpeed = 1;
				curveAheadIndex = 1;
			}

			else {
				maintainCurveSpeed = 0;
			}

			if(maintainCurveSpeed == 1){

				accelDemand_ISA = -((fol_vel - curveVelocity)); // Ideally 18
				//if(accelDemand_ISA < accelDemand){
				if(accelDemand_ISA < accelDemand){
					accelerationControl(accelDemand_ISA, fol_acc_x,2);
				}

				//}
			}

			//	if(integralDistance > 3269){
			//		integralDistance =0;
			//	}
			//}

			else if(vehicleAhead == 0){
				accelDemand = -((fol_vel - 28.0));
				accelerationControl(accelDemand, fol_acc_x,1);
			}


		}

		//accelDemand = accelerationDemand((10-distApartRadar), fol_vel, 0.5);
		//				}
		// Constant velocity
		//		else{
		//			accelDemand = -((fol_vel - 25.0));
		//		}

		//accelerationControl_radar(accelDemand, fol_acc_x);

		/********************* Implementation of ISA *************************************/

		//				if(((fol_vel*dummy) > 0.35 ||(fol_vel*dummy) < -0.3 && vehicleAhead == 0)){
		//					accelDemand = -(abs(fol_vel) - (0.25/dummy));
		//					int isa_sensor = 1;
		//					accelerationControl(accelDemand, fol_acc_x,isa_sensor);
		//accelerationControl_isa(accelDemand, fol_acc_x);
		//				}

		/************************* Implementation of acceleration control based on  V2V euclidean distance *************************************/


		/******************** Sending via UART ******************************/




		send_float(throttle);
		send_float(steer);
		send_float(brake);



		if(xSemaphoreGive(vehToVehSem) == pdTRUE)
		{
			//Go for v-v task
		}


		//	 CANStop=xTaskGetTickCount ();
		//	CANUtil= CANStop-CANStart;

		if(ccOverrun == 1 && ccOverrunCount<2){
			ccOverrunCount++;

			int cc = 0;
			for(cc=0;cc<4000;cc++){

			}

		}


	}
	vTaskDelete(cruiseControlTaskTcb);
	//}
}

/* Task1 */
void canTask(void *pvParameters)
{
	//for periodicity
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	/*	volatile TickType_t CANwaitTime = 0;
	volatile TickType_t CANstartTime = 0;
	volatile TickType_t CANstopTime = 0;
	volatile TickType_t CANexecutionTime [500];
	 */

	static int i = 0;
	int laneChangeCount =0;
	//	stepCount++;

	//prevTime = xTaskGetTickCount ();

	while (1)
	{
		vTaskDelayUntil(&xLastWakeTime, 10); //

		if(xSemaphoreTake(canIntRxSem, 0) == pdTRUE)
		{


			//     CANStart=xTaskGetTickCount ();

			if(mailBox == canMESSAGE_BOX1 || mailBox == canMESSAGE_BOX21 || mailBox == canMESSAGE_BOX31)
			{

				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data); // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);

				memcpy(&fol_pos[0], &ans, 4);
				memcpy(&lead_pos[0], &ans2, 4);

				// memcpy(&fol_pos[1], &ans2, 4);

				if(xSemaphoreGive(radarSem) == pdTRUE)
				{
					//Can happen if the semaphore is not released from CAN task, but will not
				}


				if(xSemaphoreGive(cruiseControlSem) == pdTRUE)
				{
					//Can happen if the semaphore is not released from CAN task, but will not
				}

				if(xSemaphoreGive(uartAccTxSem) == pdTRUE)
				{
					//Can happen if the semaphore is not released from CAN task, but will not
				}


			}

			if(mailBox == canMESSAGE_BOX2 || mailBox == canMESSAGE_BOX12 || mailBox == canMESSAGE_BOX22 || mailBox == canMESSAGE_BOX32)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);
				//memcpy(&fol_pos[2], &ans, 4);
				memcpy(&lead_vel, &ans, 4);
				memcpy(&fol_ang_vel_z, &ans2, 4);


			}

			if(mailBox == canMESSAGE_BOX3 || mailBox == canMESSAGE_BOX13 || mailBox == canMESSAGE_BOX23 || mailBox == canMESSAGE_BOX33)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);
				memcpy(&fol_head_error, &ans, 4);
				memcpy(&fol_lat_error, &ans2, 4);

			}

			if(mailBox == canMESSAGE_BOX4 || mailBox == canMESSAGE_BOX14 || mailBox == canMESSAGE_BOX24 || mailBox == canMESSAGE_BOX34)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);
				memcpy(&fol_acc_x, &ans, 4);
				memcpy(&fol_vel, &ans2, 4);


			}

			if(mailBox == canMESSAGE_BOX5 || mailBox == canMESSAGE_BOX15 || mailBox == canMESSAGE_BOX25 || mailBox == canMESSAGE_BOX35)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);
				memcpy(&fol_yaw, &ans, 4);
				//memcpy(&dummy, &ans2, 4);
				memcpy(&foltravelledDistance, &ans2, 4);

				if(dummy > curvatureThreshold || dummy < downcurvatureThreshold){
					//xPos[xPosIndex]=fol_pos[0];
					//yPos[yPosIndex]=fol_pos[1];
					//xPosIndex++;
					//yPosIndex++;
				}

			}

			if(mailBox == canMESSAGE_BOX6 || mailBox == canMESSAGE_BOX16 || mailBox == canMESSAGE_BOX26 || mailBox == canMESSAGE_BOX36)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));
				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);
			//	memcpy(&lead_vel, &ans, 4);
			//	memcpy(&lead_pos[0], &ans2, 4);


			}

			if(mailBox == canMESSAGE_BOX7 || mailBox == canMESSAGE_BOX17 || mailBox == canMESSAGE_BOX27 || mailBox == canMESSAGE_BOX37)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);
				// memcpy(&lead_pos[1], &ans, 4);

				// memcpy(&lead_pos[2], &ans2, 4);
				// Replace this also in simulink. Instead of lead_pos[2] send foltravelledDistance as the value.
				//memcpy(&foltravelledDistance, &ans2, 4);


			}

			if(mailBox == canMESSAGE_BOX8 || mailBox == canMESSAGE_BOX18 || mailBox == canMESSAGE_BOX28 || mailBox == canMESSAGE_BOX38)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);
				memcpy(&lead_vel_delayed, &ans, 4);
				memcpy(&lead_pos_delayed[0], &ans2, 4);


			}

			if(mailBox == canMESSAGE_BOX9 || mailBox == canMESSAGE_BOX19 || mailBox == canMESSAGE_BOX29 || mailBox == canMESSAGE_BOX39)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);  // Receive Completed

				//First float value from CAN msg
				ans = ((rx_data[0]<<24)|(rx_data[1]<<16)|(rx_data[2]<<8)|rx_data[3]);
				//Second float value from CAN msg
				ans2 = ((rx_data[4]<<24)|(rx_data[5]<<16)|(rx_data[6]<<8)|rx_data[7]);

					memcpy(&lead_pos_delayed[1], &ans, 4);
					memcpy(&lead_pos_delayed[2], &ans2, 4);

				// change this since 0x88 is not being sent due to over congestion
				//memcpy(&lead_pos_delayed[0], &ans, 4);
				//memcpy(&lead_pos_delayed[1], &ans2, 4);

				currentTime = xTaskGetTickCount ();
				timeStep = (currentTime - prevTime)/100000;
				prevTime = currentTime;

				if (xTaskCreate(radarTask,"Radar Task", configMINIMAL_STACK_SIZE, NULL, 6, &radarTaskTcb) != pdTRUE)
						{
							/* Task could not be created */
							while(1);
						}

					if (xTaskCreate(steerTask,"Steer Task", configMINIMAL_STACK_SIZE, NULL, 5, &steerTaskTcb) != pdTRUE)
						{
							/* Task could not be created */
							while(1);
						}

					if (xTaskCreate(cruiseControlTask,"Cruise Control Task", configMINIMAL_STACK_SIZE, NULL, 4, &cruiseControlTaskTcb) != pdTRUE)
						{
							/* Task could not be created */
							while(1);
						}



					if (xTaskCreate(CATask,"Collision Avoidance Task", UART_STACK_SIZE, NULL, 3, &CATcb) != pdTRUE)
						{

							/* Task could not be created */
							while(1);
						}

					if (xTaskCreate(vehToVehTask,"Veh to Veh Task", configMINIMAL_STACK_SIZE, NULL, 2, &vehToVehTaskTcb) != pdTRUE)
						{
							/* Task could not be created */
							while(1);
						}


			}

			if(mailBox == canMESSAGE_BOX10|| mailBox == canMESSAGE_BOX20 || mailBox == canMESSAGE_BOX30 || mailBox == canMESSAGE_BOX40)
			{
				while(!canIsRxMessageArrived(canREG1, mailBox));
				if(canIsRxMessageArrived(canREG1, mailBox))
				{
					canGetData(canREG1, mailBox, rx_data);  // Receive Completed
					overrunTrigger = rx_data[0];

									if(overrunTrigger == 200){  // Down  Button for steer overrun
											steerOverrun = 1;
											steerOverrunCount++;
											if(steerOverrunCount % 2 == 0){
												steerOverrun = 0;
											}

										PID = 1;
										ONOFF = 0;
										ISA = 0;
									}

									if(overrunTrigger == 100){  // Up Button --> CA Overrun
											caOverrun = 1;
											caOverrunCount++;
												if(caOverrunCount % 2 == 0){
													caOverrun = 0;
												}

										PID = 0;
										ONOFF = 1;
										ISA=1;

									}

									if(overrunTrigger ==144){   // Right Button --> Radar Overrun

									    radarOverrun = 1;
											radarOverrunCount++;
												if(radarOverrunCount % 2 == 0){
													radarOverrun = 0;
											     }
									}

									if(overrunTrigger == 44){  // Left Button -- > ccOverun
											ccOverrun = 1;
											ccOverrunCount++;
												if(ccOverrunCount % 2 == 0){
													ccOverrun = 0;
												}
									}

									if(overrunTrigger == 244){  // Click Button --- V2V overrun
										v2vOverrun = 1;
										if(v2vOverrun == 1){
											v2vOverrunCount++;
											if(v2vOverrunCount % 2 == 0){
											    v2vOverrunCount = 0;
											}
										}

										if(xSemaphoreGive(ultrasonicSem) == pdTRUE)
										{
											//Can happen if the semaphore is not released from CAN task, but will not
										}
									}
			}

			// 0*8B message for Lane Change Trigger

			if(mailBox == canMESSAGE_BOX11  ){
				while(!canIsRxMessageArrived(canREG1, mailBox));

				canGetData(canREG1, mailBox, rx_data);
				//laneChangeCount = rx_data[0];

				if(laneChangeCount==0){  // Right Button for Lane Change Trigger

				}


				if(laneChangeCount == 246){  // Left Button --> Radar Overrun


				}

				if(laneChangeCount ==226){   // UP Button --> CAN Overrun



				}

				if(laneChangeCount == 236){  // DOWN Button --- Both radar and CAN overrun


				}


			}


			//	CANStop=xTaskGetTickCount ();
			//	CANUtil= CANStop-CANStart;

			/*

			CANstopTime = xTaskGetTickCount ();
			CANexecutionTime[i] = CANstopTime - CANstartTime;
			Budget += CANexecutionTime[i];
			i++;
			if (i == 500)
			{
				i = 0;
				BudgetAvg = Budget/1000;
				Budget = 0;
			}
			 */
		}
		//1000 is 10 ms. Conversion to x ms = (x/0.01)
		//TODO: Periodic task works but this does not capture all the incoming data.

	}

	// Simulation of Budget Overrun for CAN task using for loop


}

}

void main(void)
{

	/* Set high end timer GIO port hetPort pin direction to all output */
	gioSetDirection(hetPORT1, 0xFFFFFFFF);

	/* UART init */
	sciInit();
	sciSetBaudrate(scilinREG, 115200U);

	/* Configuring CAN1: MB1, Msg ID-0x82 to recieve from ABS Gateway; MB2, Msg ID-0x81 to recieve from Suspension/OBD Gateway */
	canInit();

	//printf("Time %f us\n", time_PMU_code);
	//taskDISABLE_INTERRUPTS();
	//_disable_interrupt_();
	vimDisableInterrupt(16);

	canIntRxSem = xSemaphoreCreateBinary();
	if(NULL == canIntRxSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}
	uartAccTxSem = xSemaphoreCreateBinary();
	if(NULL == uartAccTxSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}
	uartUltrasonicTxSem = xSemaphoreCreateBinary();
	if(NULL == uartUltrasonicTxSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}

	accSem = xSemaphoreCreateBinary();
	if(NULL == accSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}

	ultrasonicSem = xSemaphoreCreateBinary();
	if(NULL == ultrasonicSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}

	radarSem = xSemaphoreCreateBinary();
	if(NULL == radarSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}

	cruiseControlSem = xSemaphoreCreateBinary();
	if(NULL == cruiseControlSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}

	vehToVehSem = xSemaphoreCreateBinary();
	if(NULL == vehToVehSem)
	{
		/* Failed to create Semaphore */
		while(1);
	}


	if (xTaskCreate(canTask,"CAN Task", configMINIMAL_STACK_SIZE, NULL, 7, &canTaskTcb) != pdTRUE)
	{
		/* Task could not be created */
		while(1);
	}



	//	if (xTaskCreate(blinkLEDTask,"Blink LED Task", configMINIMAL_STACK_SIZE, NULL, 2, &blinkLEDTaskTcb) != pdTRUE)
	//			{
	/* Task could not be created */
	//				while(1);
	//		}

	//		 if (xTaskCreate(CANPeriodicTxTask,"CANPeriodicTxTask", configMINIMAL_STACK_SIZE , NULL, 3, &CANPeriodicTxTaskTcb) != pdTRUE)
	//	        {
	/* Task could not be created */
	//            while(1);
	//      }


	//taskENABLE_INTERRUPTS();
	//_enable_interrupt_();
	vimEnableInterrupt(16, SYS_IRQ);



#if 0
	_pmuInit_();
	_pmuEnableCountersGlobal_();
	_pmuSetCountEvent_(pmuCOUNTER0, PMU_CYCLE_COUNT); // PMU_INST_ARCH_EXECUTED

	_pmuResetCounters_();
	_pmuStartCounters_(pmuCOUNTER0);
	cycles_PMU_start = _pmuGetEventCount_(pmuCOUNTER0);

	/* Place the task here to measure the time */

	//for(i = 0 ; i < 100000; i++);
	for(i = 0 ; i < 10000; i++)
	{
		for(j = 0 ; j < 10000; j++);
	}
	/*
	for(i = 0 ; i < 10000; i++)
	{
		for(j = 0 ; j < 10000; i++);
	}
	 */
	_pmuStopCounters_(pmuCOUNTER0);
	cycles_PMU_end = _pmuGetEventCount_(pmuCOUNTER0);
	cycles_PMU_measure = cycles_PMU_end - cycles_PMU_start;

	/* Measure the time compensation */
	_pmuResetCounters_();
	_pmuStartCounters_(pmuCOUNTER0);
	cycles_PMU_start = _pmuGetEventCount_(pmuCOUNTER0);

	_pmuStopCounters_(pmuCOUNTER0);
	cycles_PMU_end = _pmuGetEventCount_(pmuCOUNTER0);
	cycles_PMU_comp = cycles_PMU_end - cycles_PMU_start;

	/* Calculate Time */
	cycles_PMU_code = cycles_PMU_measure - cycles_PMU_comp;
	time_PMU_code = cycles_PMU_code / (f_HCLK); // time_code [us], f_HCLK [MHz]
	//time_PMU_code = cycles_PMU_code / (f_HCLK * loop_Count_max); //
#endif
	/* Start Scheduler */
	vTaskStartScheduler();

	/* Run forever */
	while(1);
	/* USER CODE END */
}

/* can interrupt notification */
void canMessageNotification(canBASE_t *node, uint32 messageBox)
{

	//static int sensitivity = 0;
	//printf("Intrpt\n");
	if(node == canREG1)
	{
		//vimDisableInterrupt(16);
		mailBox = messageBox;


		if(xSemaphoreGiveFromISR(canIntRxSem, NULL) == pdTRUE)
		{

		}

	}
}

#if 0
while(!canIsRxMessageArrived(canREG1, canMESSAGE_BOX1));
canGetData(canREG1, canMESSAGE_BOX1, rx_data); /* Recieve Completed */
strncpy(rxMbox1, (char *)rx_data, 4);
//printf("Concatenated String %s\n", rxMbox1);
rxIntMbox1 = atoi(rxMbox1);
//printf("Converted int %d\n", rxIntMbox1);
wheelSpin[countbox1++] = rxIntMbox1;
if(countbox1 == 4)
{
	countbox1 = 0;
	antiwheelSlip = calculateWheelSlip(wheelSpin);
	//printf("Brake Integer %d\n", brake);
	snprintf((char *)rx_str_data, 5,"%d", antiwheelSlip);
	//printf("Concatenated Brake String %s\n", rx_str_data);
	sciSend(UART_ABS, 4, rx_str_data);
	gioSetPort(hetPORT1, gioGetPort(hetPORT1) ^ 0x00FFF001);/* Toggle HET pin 0.                                    */
	//vTaskDelay(50);
}
vimEnableInterrupt(16, SYS_IRQ);
}

if(messageBox == canMESSAGE_BOX2)
{
	vimDisableInterrupt(16);
	while(!canIsRxMessageArrived(canREG1, canMESSAGE_BOX2));
	canGetData(canREG1, canMESSAGE_BOX2, rx_data); /* Recieve Completed */
	strncpy(rxMbox2, (char *)rx_data, 4);
	rxIntMbox2 = (atoi(rxMbox2));
	pubSpeed = rxIntMbox2;
	//if(countbox2 == 4)
	{
		//countbox2 = 0;
		sensitivity = calculateSteerSensitivity(pubSpeed);
		//printf("Brake Integer %d\n", brake);
		snprintf((char *)rx_str_data, 5,"%d", sensitivity);
		//printf("Concatenated Brake String %s\n", rx_str_data);
		sciSend(UART_STEER, 1, rx_str_data);
		gioSetPort(hetPORT1, gioGetPort(hetPORT1) ^ 0xA2000000);/* Toggle HET pin 0.                                    */
		//vTaskDelay(50);
	}
	vimEnableInterrupt(16, SYS_IRQ);
}
#endif

float max(float a, float b){
	if(a>b){
		return a;
	}
	if(b>a){
		return b;
	}

}

float min(float a, float b){

	if(a<b){
		return a;
	}
	if(b<a){
		return b;
	}
}


void send_float (float arg)
{
	uint8 header[2] = {'(',')'};
	uint8 terminator[2] = {'\r','\n'};
	// get access to the float as a byte-array:
	uint8 *data = (uint8*)&arg;
	uint8 *data3 = (uint8*)&arg+3;
	uint8 *data2 = (uint8*)&arg+2;
	uint8 *data1 = (uint8*)&arg+1;

	// write the data to the serial (little endian since matlab only reads it this way)
	//sciSend (scilinREG, 2, &header[0]);
	sciSend (scilinREG, 1, data3);
	sciSend (scilinREG, 1, data2);
	sciSend (scilinREG, 1, data1);
	sciSend (scilinREG, 1, data);
	//sciSend (scilinREG, 2, &terminator[0]);
}

/***********************************Speed Control Start***********************************/

/* Calculate Euclidian Distance between the 2 vehicles */
int calculateEuclidianDistance(float follower_pos[3], float leader_pos[3])
{
	int diff1 = (follower_pos[0]-leader_pos[0])*(follower_pos[0]-leader_pos[0]);
	int diff2 = (follower_pos[1]-leader_pos[1])*(follower_pos[1]-leader_pos[1]);
	//	float diff3 = (follower_pos[2]-leader_pos[2])*(follower_pos[2]-leader_pos[2]);
	int dist = sqrt(diff1+diff2);


	return (dist);

}


/* Calculate Euclidian Distance between the 2 vehicles */
float calculateEuclidianDistanceCurve(float xpos1, float xpos2, float ypos1, float ypos2)
{
	float diff1 = (xpos1-xpos2)*(xpos1-xpos2);
	float diff2 = (ypos1-ypos2)*(ypos1-ypos2);
	//	float diff3 = (follower_pos[2]-leader_pos[2])*(follower_pos[2]-leader_pos[2]);
	float dist = sqrt(diff1+diff2);
	return (dist);
}


/*
float calculate_Euclidean_v2v(curve_1_x[j], curve_1_y[j], curve_1_z[j], curve_1_x[j+1], curve_1_y[j+1], curve_1_z[j+1]){
	return sqrt(curve_1_x[j]*curve_1_x[j+1] + curve_1_y[j]*curve_1_y[j+1] + curve_1_z[j]*curve_1_z[j+1]);
}
 */

//Substitute derivative with vel difference btw the 2 vehicles
//Also means we need to transmit lead vehicle velocity.
float numericalDerivativeDist(float dist)
{
	static float oldDist = 0.0;

	float result = (dist - oldDist)/timeStep;
	oldDist = dist;
	return result;

}

float accelerationDemand(float dist, float follower_velX, float headway)
{
	//	static int i = 0;
	float headway_vel = headway * follower_velX;
	float gain = (headway_vel + dist ) * 0.1;
	float vel = numericalDerivativeDist(dist);
	float accelDemand = -(gain + vel);
	//	accelRecord[i] = vel;
	//	i++;
	//	if (i == 50)
	//	{
	//		i = 0;
	//	}
	return accelDemand;
}

float accelerationDemand2(float dist, float follower_velX, float leader_velX, float headway)
{
	static int i = 0;
	//Calculates the separation wanted btw the 2 vehicles based on follower's current speed
	float headway_dist = headway * follower_velX;
	// Separation wanted + Current distance apart
	float gain = (headway_dist + dist ) * 0.1;
	// Fraction of total distance apart plus difference in velocity
	float accelDemand = -(gain + (follower_velX - leader_velX));
	//		accelRecord[i] = xTaskGetTickCount ();
	//		i++;
	//		if (i == 50)
	//		{
	//			i = 0;
	//		}
	return accelDemand;
}


void simpleThrottleBrake(float dist, float follower_velX, float leader_velX, float headway)
{

	//float headway_dist = headway * follower_velX;
	//float headway_dist = 25.0;
	//Simple speed control for low budget


	// if (dist<headway_dist){




	if (dist<15.0){
		if(follower_velX > leader_velX)
		{
			brake = 1.0;
			throttle = 0;
		}
		else
		{
			//throttle = 1.0;
			throttle = 2.0/(1.0+exp(fol_vel-25.0));
			brake = 0;
		}
	}

	else
	{
		throttle = 1.0;
		brake = 0;
	}


	//Taken from: A Modular Parametric Architecture for the TORCS Racing Engine
	//Works slight better than the crude method above

	//throttle = 2/(1+exp(follower_velX-leader_velX+2));
	//brake = 0;


	//Taken from: Efficieny analysis of formally verifired adaptive cruise controller
	//Very conservative such that it doesnt keep up on hilly roads
	//	throttle = (sqrt(timeStep*timeStep - 4*follower_velX*timeStep + 8*dist + 4*leader_velX*leader_velX) - timeStep - 2*follower_velX) / 2*timeStep;
	//	brake = (-(follower_velX*follower_velX))/(2*(dist+(leader_velX*leader_velX)/2));




}

int checkFieldOfViewRadar(float follower_pos[3], float leader_pos[3], float dist)
{
	//67.5 degrees in radians = 1.1781
	//45 deg in rad = 0.7854
	//	float viewAngle = 0.2854;
	float viewAngle = 0.07;
	//Range - 50m
	float x = leader_pos[0]-follower_pos[0];
	float y = leader_pos[1]-follower_pos[1];
	float targetAngle = atan2(y,x);
	float leftBound = fol_yaw + viewAngle;
	float rightBound = fol_yaw - viewAngle;



	//	if (targetAngle <= leftBound && targetAngle >= rightBound || (dist > 10 && dist < 15))
	if (targetAngle <= leftBound && targetAngle >= rightBound &&  dist < 50)
	{

		return 1;
	}
	else
		return 0;


}

int checkFieldOfViewV2V(float dist)
{
	//67.5 degrees in radians = 1.1781
	//45 deg in rad = 0.7854
	//	float viewAngle = 0.2854;
	float viewAngle = 0.0;
	//	if (targetAngle <= leftBound && targetAngle >= rightBound || (dist > 10 && dist < 15))
	if (dist < 50)
	{

		return 1;
	}
	else
		return 0;
}

/***********************************Speed Control End***********************************/

/***********************************Steering Control Start***********************************/

float numericalDerivativeKP(float kp)
{
	static float oldkp = 0.0;

	float result = (kp - oldkp)/timeStep;
	oldkp = 1.6*kp;
	return result;

}

float numericalDerivativeKD(float kd)
{
	static float oldkd = 0.0;

	float result = (kd - oldkd)/timeStep;
	oldkd = kd;
	return result;

}




float laneChangeSteering (float angular_vel, float headingError, float lateralError)
{

	float kp = 0.08 * headingError;
	float kd = 1 * lateralError;
	float k = 0.5 * (angular_vel + kp + kd);
	float kpDerivative = 0.75 * numericalDerivativeKP(kp);
	float kdDerivative = 0.25 * numericalDerivativeKD(kd);
	float gain = 0.5 * (-kpDerivative - kdDerivative -k);
	float bias = 0.42;
	float biasStart =0.42;
	float biasIncrement=0.07;
	int i=0;
	const TickType_t xDelay = 20 / portTICK_PERIOD_MS;
	/*for(i =0;i<7;i++){
		biasStart = biasStart + biasIncrement;
		gain = gain + biasStart;
	}*/



	/*
	//Need to bound the output for steering
	if(gain>0.5)
		return 0.5;
	else if(gain<(-0.5))
		return (-0.5);
	else
		return gain;*/


}



float steering (float angular_vel, float headingError, float lateralError)
{

	float kp = 0.08 * headingError;
	float kd = 1 * lateralError;
	float k = 0.5 * (angular_vel + kp + kd);
	float kpDerivative = 0.75 * numericalDerivativeKP(kp);
	float kdDerivative = 0.25 * numericalDerivativeKD(kd);


	float gain = 0.5 * (-kpDerivative - kdDerivative -k);

	//if(laneChangeFlag==1){
	//	gain = gain+ 0.5;  // For giving bias to the car trajectory from its original path.
	//}

	/*
		if(laneChangeResolution < 0.42){
			laneChangeResolution = laneChangeResolution + 0.07;
			gain = gain + laneChangeResolution;
		}
		if(laneChangeResolution > 0.42){
			laneChangeResolution =0;
			laneChangeTriggerFlag=2;
		}*/





	//Need to bound the output for steering
	if(gain>0.5)
		return 0.5;
	else if(gain<(-0.5))
		return (-0.5);
	else
		return gain;


}

/***********************************Steering Control End***********************************/

/***********************************Acceleration Control Start***********************************/

#define CONSTANT_COMPARATOR -0.1

//requires accel demand
//Full braking available (RADAR)
void accelerationControl(float accelDemand, float follower_accelX, int sensor)
{
	float m,c,integral,result;
	if(accelDemand >= (-0.1))
	{
		static float oldGainNsum = 0;	//y-coordinate

		float accelDemandGain = 1.5 * accelDemand;
		float gainNsum = 10 *(accelDemand - follower_accelX);
		//integrate?!
		//Find eqn of line (first point 0,0) Y = mX +C
		m = (gainNsum-oldGainNsum)/(timeStep);
		c = gainNsum - (m * timeStep);
		//func = m*X +C
		integral =  int_simpson(0.0, gainNsum, 10, m, c);
		if (integral > 1.0)
			integral = 1.0;
		else if (integral < (-1.0))
			integral = -1.0;

		result = integral+accelDemandGain;
		oldGainNsum = gainNsum;
		//return result;

		//throttle_radar = result;
		throttle = result;


		//throttle = result;
		brake = 0;


	}
	else
	{
		static float oldBrakeGainNsum = 0;
		float brakeGainNsum = follower_accelX-accelDemand;
		m = (brakeGainNsum-oldBrakeGainNsum)/(timeStep);
		c = brakeGainNsum - (m * timeStep);
		integral =  int_simpson(0.0, brakeGainNsum, 10, m, c);
		if (integral > 1.0)
			integral = 1.0;
		else if (integral < (-1.0))
			integral = -1.0;

		result = integral+ 0.005 * brakeGainNsum;
		oldBrakeGainNsum = brakeGainNsum;
		if (result <= 1.0)
		{
			//return 1.0;

				brake = result;

		}



		else if (result < (0.0))
		{
			//return (-1.0);

			brake = 0;
		}

		else if (result > (1.0))
		{
			//return result;
			if(result > brake){
				brake = 1;
			}
		}
		throttle = 0;
	}
}


//Veh to Veh Comms Control - Brake force limited
void accelerationControlVeh(float accelDemand, float follower_accelX)
{
	float m,c,integral,result;
	//float timeStep = 0.02;	//x-coordinate
	static int i = 0;
	//	static int j = 0;

	i++;
	if (i == 200){
		i = 0;
	}

	if(accelDemand >= (-0.1))
	{
		static float oldGainNsum = 0;	//y-coordinate

		float accelDemandGain = 1.5 * accelDemand;
		float gainNsum = 10 *(accelDemand - follower_accelX);
		//integrate?!
		//Find eqn of line (first point 0,0) Y = mX +C
		m = (gainNsum-oldGainNsum)/(timeStep);
		c = gainNsum - (m * timeStep);
		//func = m*X +C
		integral =  int_simpson(0.0, gainNsum, 10, m, c);
		if (integral > 1.0)
			integral = 1.0;
		else if (integral < (-1.0))
			integral = -1.0;

		result = integral+accelDemandGain;
		oldGainNsum = gainNsum;
		//return result;
		throttle = result;


	}
	else
	{
		static float oldBrakeGainNsum = 0;
		float brakeGainNsum = follower_accelX-accelDemand;
		m = (brakeGainNsum-oldBrakeGainNsum)/(timeStep);
		c = brakeGainNsum - (m * timeStep);
		integral =  int_simpson(0.0, brakeGainNsum, 10, m, c);
		if (integral > 1.0)
			integral = 1.0;
		else if (integral < (-1.0))
			integral = -1.0;

		result = integral+ 0.005 * brakeGainNsum;
		oldBrakeGainNsum = brakeGainNsum;
		if (result > 0.3)
		{
			//return 1.0;
			brake = 0.30;
		}

		else if (result < (-0.3))
		{
			//return (-1.0);
			brake = -0.30;
		}

		else
		{
			//return result;
			brake = result;
		}
		throttle = 0;
		//		accelRecord[j] = brake;
		//		j++;
		//		if (j == 50){
		//			j = 0;
		//		}

	}
}

double int_simpson(double from, double to, double n, double m, double c)
{
	double h = (to - from) / n;
	double sum1 = 0.0;
	double sum2 = 0.0;
	int i;

	for(i = 0;i < n;i++)
		//return eqn
		sum1 += m*(from + h * i + h / 2.0) + c;

	for(i = 1;i < n;i++)
		sum2 += m * (from + h * i) + c;

	return h / 6.0 * ((m*(from)+c) + (m*(to)+c) + 4.0 * sum1 + 2.0 * sum2);
}


/***********************************Acceleration Control End***********************************/


int calculateSteerSensitivity(int pubSpeed)
{
	float sensitivity;

	if(pubSpeed < 20)
		sensitivity = 2;//high sensitivity
	else
		sensitivity = 1;//low sensitivity

	return sensitivity;
}

int calculateWheelSlip(int *wheelSpin)
{
	int slip;
	float wheelradius[4] = {0.330600, 0.330600, 0.327600, 0.327600};
	int i;
	for(i = 0; i < 4; i++)
	{
		slip += wheelSpin[i] * wheelradius[i];
	}
	slip = slip/4;
	return slip;
}
