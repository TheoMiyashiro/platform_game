#include <stdlib.h>
#include "joystick.h"

joystick* joystick_create()
{
    joystick *element = (joystick*) malloc(sizeof(joystick));
    element->right = false;
    element->left  = false;
    element->up    = false;
    element->down  = false;
    return element;
}

void joystick_destroy(joystick *element)
{
    free(element);
}

void joystick_left(joystick *element)
{
    element->left = true;
}

void joystick_right(joystick *element)
{
    element->right = true;
}

void joystick_left_release(joystick *element)
{
    element->left = false;
}

void joystick_right_release(joystick *element)
{
    element->right = false;
}

void joystick_down(joystick *element)
{
    element->down = true;
}

void joystick_down_release(joystick *element)
{
    element->down = false;
}
