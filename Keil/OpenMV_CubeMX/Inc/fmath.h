#ifndef __FMATH_H__
#define __FMATH_H__
/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"

#define PI 3.1415926535897932384626433832795f
#define M_PI    3.14159265f
#define M_PI_2  1.57079632f
#define M_PI_4  0.78539816f

#define MIN( a, b) ((a<b) ? (a) : (b))
#define MAX( a, b) ((a>b) ? (a) : (b))
#define ABS(x) ( (x)>0.0f ?(x):-(x) )
#define LIMIT( x,min,max ) ( (x) < (min)  ? (min) : ( (x) > (max) ? (max) : (x) ) )

float fast_atanf(float xx);
float fast_atan2f(float y, float x);
float fast_sqrt(float number);
float fast_sin(float rad);
float fast_cos(float rad);
float fast_asin(float i);
float fast_acos(float i);


#endif /* _FMATH_H__ */
/******************* (C) COPYRIGHT 2016 LinCongCong*****END OF FILE************/
