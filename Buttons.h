/*
 * Buttons.h
 *
 * Created April 1, 2017
 * author:    Andras Kun (kun.andras@yahoo.de), Jason Brinkerhoff (jb@jbkuma.com)
 * source:    https://github.com/Protonerd/FX-SaberOS
 * 
 * based on LightSaber OS originally created by Sebastien CAPOU (neskweek@gmail.com)
 */
#include <Arduino.h>
#if not defined BUTTONS_H_
#define BUTTONS_H_


/*
 * BUTTONS PARAMETERS
 ************************************/
#define CLICK				200 //5    // ms you need to press a button to be a click
#define PRESS_ACTION		600 //200  // ms you need to press a button to be a long press, in action mode
#define PRESS_CONFIG		600 //400  // ms you need to press a button to be a long press, in config mode
/************************************/




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

#endif /* BUTTONS_H_ */


