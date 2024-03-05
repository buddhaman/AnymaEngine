#include "InputHandler.h"

bool IsKeyDown(InputHandler* input, InputAction action)
{
    return input->keydown[action];
}