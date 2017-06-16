#pragma once

#include <math.h>

inline double sin_deg(double angle){
   return sin(angle * M_PI / 180.0);
}

inline double cos_deg(double angle){
   return cos(angle * M_PI / 180.0);
}
