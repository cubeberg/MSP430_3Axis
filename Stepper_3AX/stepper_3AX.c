/*
 * stepper_3AX.c
 *
 *  Created on: Jun 15, 2013
 *      Author: 28918
 */
#include <msp430.h>

#include "stepper_3AX.h"

struct Point currentLoc;
struct Point targetLoc;
struct Point inactiveLoc; //location we go to when we're done, or when we're stopping for some reason
int xMax = 7999;
int yMax = 7999;
int zMax = 2000; //10 full steps
const int zHold = 1000; //z location we go to when stopping/holding
int stepWaitTime = SLOW_STEPS;
int stepWaitTimeDown = FAST_STEPS;


void StepperZeroOut()
{
	currentLoc.x = 0;
	currentLoc.y = 0;
	currentLoc.z = 0;

	StepperGoTo(0,0,zHold); //basically just pull pen up
}


void StepperInit()
{
	P1OUT &= ~(STEP_X|STEP_Y|STEP_Z|ENBL); //set low
	P1DIR |= STEP_X|STEP_Y|STEP_Z|ENBL; //set as output
	P2OUT &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);
	P2DIR |= BIT0|BIT1|BIT2|BIT3|BIT4|BIT5;
	inactiveLoc.z = zHold;
}

void StepperGoToGCode(struct Point location)
{
	int x,y,z;
	//start with current coordinates
	x = currentLoc.x;
	y = currentLoc.y;
	z = currentLoc.z;
	//pull in new values if not -1
	if(location.x >= 0)
		x = location.x;
	if(location.y >= 0)
		y = location.y;
	if(location.z >= 0)
		z = location.z;
	StepperGoTo(x,y,z);
}

void StepperGoTo(int X_Coord, int Y_Coord, int Z_Coord)
{
	//copy to target variable
	if(X_Coord > xMax)
		targetLoc.x = xMax;
	else
		targetLoc.x = X_Coord;
	if(Y_Coord > yMax)
		targetLoc.y = yMax;
	else
		targetLoc.y = Y_Coord;
	if(Z_Coord > zMax)
		targetLoc.z = zMax;
	else
		targetLoc.z = Z_Coord;

	char stepSpeed = 0;
	while(!StepperIsAtTarget())
	{
		stepSpeed = 1;
		//for now - resetting to known state - not sure if it'd be easier to compare values and only change what's needed
		StepperResetDir();
		//check z coordinate first - always go to correct Z, then move too X and Y coords
		if(targetLoc.z != currentLoc.z)
		{
			stepSpeed = 0;
			if(targetLoc.z > currentLoc.z) //forwards
			{
				P1OUT |= STEP_Z;
				currentLoc.z++;
			}
			else //backwards
			{

				P2OUT |= DIR_Z;
				P1OUT |= STEP_Z;
				currentLoc.z--;
			}

		}
		else //check X and Y
		{
			if(targetLoc.x > currentLoc.x) //forward
			{
				P1OUT |= STEP_X;
				currentLoc.x++;
			}
			else if (targetLoc.x < currentLoc.x)
			{
				P2OUT |= DIR_X;
				P1OUT |= STEP_X;
				currentLoc.x--;
			}
			if(targetLoc.y > currentLoc.y) //forward
			{
				P1OUT |= STEP_Y;
				currentLoc.y++;
			}
			else if (targetLoc.y < currentLoc.y)
			{
				P2OUT |= DIR_Y;
				P1OUT |= STEP_Y;
				currentLoc.y--;
			}
		}
		__delay_cycles(32);  // Wait ~2 µs @ 16 MHz
		P1OUT &= ~(STEP_X|STEP_Y|STEP_Z);
		__delay_cycles(32);  // Wait ~2 µs @ 16 MHz
		if(stepSpeed == 0)
			__delay_cycles(SLOW_STEPS);
		else if (stepSpeed == 1)
			__delay_cycles(FAST_STEPS);
	}
}

char StepperIsAtTarget()
{
	char ret = 0;
	if(currentLoc.x == targetLoc.x && currentLoc.y == targetLoc.y && currentLoc.z == targetLoc.z)
		ret = 1;
	return ret;
}

void StepperResetDir()
{
	P2OUT &= ~(DIR_X|DIR_Y|DIR_Z);
}

void StepperStep(int i_stepWaitTime)
{

	int cycles = i_stepWaitTime;
	while (cycles > 0)
	{
		_nop();
		cycles--;
	}
}

void StepperDisable()
{
	P1OUT |= ENBL;
}
void StepperEnable()
{
	P1OUT &= ~ENBL;
}

void StepperCircle(int Mid_X, int Mid_Y, int Radius)
{
	StepperGoTo(currentLoc.x, currentLoc.y, zHold); //lift pen
	StepperGoTo(Mid_X + Radius, Mid_Y, zHold); //go to starting point
	StepperGoTo(Mid_X + Radius, Mid_Y, 0); //drop pen
	int dx = Radius;
	int dy = 0;
	int xChange = 1 - 2 * Radius;
	int yChange = 1;
	int radiusError = 0;
	//first 1/8th - works great!
	while (dx >= dy)
	{
		StepperGoTo(Mid_X + dx, Mid_Y + dy, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}

	dx = Radius;
	dy = 0;
	xChange = 1 - 2 * Radius;
	yChange = 1;
	radiusError = 0;
	while (dx >= dy)
	{
		StepperGoTo(Mid_X + dy, Mid_Y + dx, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}

	dx = Radius;
	dy = 0;
	xChange = 1 - 2 * Radius;
	yChange = 1;
	radiusError = 0;

	while (dx >= dy)
	{
		StepperGoTo(Mid_X - dy, Mid_Y + dx, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}

	dx = Radius;
	dy = 0;
	xChange = 1 - 2 * Radius;
	yChange = 1;
	radiusError = 0;

	while (dx >= dy)
	{
		StepperGoTo(Mid_X - dx, Mid_Y + dy, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}

	dx = Radius;
	dy = 0;
	xChange = 1 - 2 * Radius;
	yChange = 1;
	radiusError = 0;

	while (dx >= dy)
	{
		StepperGoTo(Mid_X - dx, Mid_Y - dy, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}

	dx = Radius;
	dy = 0;
	xChange = 1 - 2 * Radius;
	yChange = 1;
	radiusError = 0;

	while (dx >= dy)
	{
		StepperGoTo(Mid_X - dy, Mid_Y - dx, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}

	dx = Radius;
	dy = 0;
	xChange = 1 - 2 * Radius;
	yChange = 1;
	radiusError = 0;

	while (dx >= dy)
	{
		StepperGoTo(Mid_X + dy, Mid_Y - dx, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}

	dx = Radius;
	dy = 0;
	xChange = 1 - 2 * Radius;
	yChange = 1;
	radiusError = 0;

	while (dx >= dy)
	{
		StepperGoTo(Mid_X + dx, Mid_Y - dy, 0);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 1;
		}
	}



	//works but horribly slow
//	int dyTarget = dx -1;
//
//	while(dyTarget > 0)
//	{
//		dx = Radius;
//		dy = 0;
//		xChange = 1 - 2 * Radius;
//		yChange = 1;
//		radiusError = 0;
//		while (dy < dyTarget)
//		{
//			dy++;
//			radiusError += yChange;
//			yChange += 2;
//			if (2 * radiusError + xChange > 0) {
//				dx--;
//				radiusError += xChange;
//				xChange += 1;
//			}
//		}
//		StepperGoTo(Mid_X + dy, Mid_Y + dx, 0);
//		dyTarget--;
//	}


}
