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
    bool keydown[InputAction_NumValues];
    bool prev_keydown[InputAction_NumValues];

    Vec2 mouse_pos;
    Vec2 mouse_delta;
    R32 mouse_scroll;
    Vec2 mouse_clicked_at;

    bool mousedown[2];
    bool prev_mousedown[2];
};

bool IsKeyDown(InputHandler* input, InputAction action);

bool IsMouseClicked(InputHandler* input, int mouse_idx);