#pragma once

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
};