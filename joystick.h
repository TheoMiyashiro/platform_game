#ifndef __JOYSTICK__ 																										
#define __JOYSTICK__																											

#include <stdbool.h>

typedef struct {																												
	bool right;																										
	bool left;																											
	bool up;																												
	bool down;																												
} joystick;																														

joystick* joystick_create();																										
void joystick_destroy(joystick *element);																							
void joystick_right(joystick *element);																								
void joystick_left(joystick *element);
void joystick_left_release(joystick *element);		
void joystick_right_release(joystick *element);
void joystick_down(joystick *element);
void joystick_down_release(joystick *element);																		


#endif																														