#pragma once

#include <math.h>

//config settings for fixed point library, using 16.16 fixed point numbers
#define FIXEDPT_BITS	 32
#define FIXEDPT_WBITS 16

#include "fixedptc.h"

inline fixedpt fixedpt_sin_deg(fixedpt angle){
   return fixedpt_sin(fixedpt_div(fixedpt_mul(angle, FIXEDPT_PI), fixedpt_rconst(180.0)));
}

inline fixedpt fixedpt_cos_deg(fixedpt angle){
   return fixedpt_cos(fixedpt_div(fixedpt_mul(angle, FIXEDPT_PI), fixedpt_rconst(180.0)));
}

inline double sin_deg(double angle){
   return sin(angle * M_PI / 180.0);
}

inline double cos_deg(double angle){
   return cos(angle * M_PI / 180.0);
}
