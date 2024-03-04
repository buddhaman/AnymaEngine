#pragma once

#include "TimVectorMath.h"
#include "AnymUtil.h"

enum InputAction
{
    InputAction_W = 0,
    InputAction_A,
    InputAction_S,
    InputAction_D,
    InputAction_NumValues
};

struct InputHandler
{
    bool keyDown[InputAction_NumValues];
    bool prevKeyDown[InputAction_NumValues];

    Vec2 mousePos;
    Vec2 mouseDelta;
    R32 mouseScroll;

    bool mouseDown[2];
    bool prevMouseDown[2];
};