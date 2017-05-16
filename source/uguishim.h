#pragma once

#include "ugui/ugui.h"

extern UG_GUI screen_context;
extern UG_COLOR Frame_Colors[];

void init_ugui();
void bsod(char* text);

void Fake_Window_Frame( UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye, UG_COLOR* p );
