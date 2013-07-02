/*
 * stepper_3AX.h
 *
 *  Created on: Jun 9, 2013
 *      Author: 28918
 */

#ifndef STEPPER_3AX_H_
#define STEPPER_3AX_H_

/*
 *
 * Connection scheme
 * STEP		1.0 - one step pin - we'll pick which motor runs by disabling/enabling
 * Enable
 * DIR X	2.3
 * DIR Y	2.4
 * DIR Z	2.5
 *
 * M0/M1 to be hardwired for now.  Can add software with an IO expander at a later time if needed
 * Sticking with 1/32 steps = M1 high, M0 not connected
 *
 */

//on P1
#define STEP_X	BIT0
#define STEP_Y	BIT3
#define STEP_Z	BIT4
#define ENBL	BIT6 //global enable pin
//On P2
#define DIR_X	BIT3
#define DIR_Y	BIT4
#define DIR_Z	BIT5

struct Point
{
	int x;
	int y;
	int z;
};

extern int xMax;
extern int yMax;
extern int zMax;
extern const int zHold;
extern struct Point currentLoc;
extern struct Point targetLoc;

#define SLOW_STEPS 3000
#define FAST_STEPS 1000




//Function prototypes

void StepperInit();
void StepperZeroOut(); //defines current location as zero - used for physical calibration

void StepperGoToGCode(struct Point location);
void StepperGoTo(int X_Coord, int Y_Coord, int Z_Coord);
void StepperCircle(int Mid_X, int Mid_Y, int Radius);

void StepperStep(int i_stepWaitTime); //makes one additional step towards target

char StepperIsAtTarget();

//functions to reset pins
void StepperResetDir();

void StepperDisable();
void StepperEnable();

#endif /* STEPPER_3AX_H_ */
