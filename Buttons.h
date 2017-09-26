/*
 * Buttons.h
 *
 *  Created on: 21 Octber 2016
 *  author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 *  Source : 	https://github.com/neskweek/LightSaberOS
 */
#include <Arduino.h>
#if not defined BUTTONS_H_
#define BUTTONS_H_

// ====================================================================================
// ===               			BUTTONS CALLBACK FUNCTIONS                 			===
// ====================================================================================

// button action enum
enum ButtonActionEnum{SINGLE_CLICK, DOUBLE_CLICK, LONGPRESS, LONGPRESS_START, LONGPRESS_STOP};
//ButtonActionEnum ButtonActionTye;

void mainClick();
void mainDoubleClick();
void mainLongPressStart();
void mainLongPress();
void mainLongPressStop();

#ifndef SINGLEBUTTON
void lockupClick();
void lockupDoubleClick();
void lockupLongPressStart();
void lockupLongPress();
void lockupLongPressStop();
#endif

void ConfigMenuButtonEventHandler(bool SaturateColor,ButtonActionEnum ButtonActionType, int8_t incrementSign=1);

#endif /* BUTTONS_H_ */


