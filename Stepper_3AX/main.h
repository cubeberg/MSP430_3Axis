/*
 * main.h
 *
 *  Created on: Jun 21, 2013
 *      Author: 28918
 */

#ifndef MAIN_H_
#define MAIN_H_

volatile char RXBuffer[50];
volatile char bufIndex = 0;
volatile char TXBuffer[20];
volatile char txIndex = 0;
volatile char txLen = 0;
volatile int LastZ = 5;

void initUart();

int ConvertString(char startIndex, char stopIndex);
struct Point ProcessBuffer();
void SendSerial(char *text, char len);
void SendSerialNext();

#endif /* MAIN_H_ */
