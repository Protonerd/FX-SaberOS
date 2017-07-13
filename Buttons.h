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

void ConfigMenuButtonEventHandler(bool SaturateColor);

#endif /* BUTTONS_H_ */


