#ifndef NINEKEY_H
#define NINEKEY_H

#include "quantum.h"

#define LAYOUT( \
       k01, k02, \
       k11, k12, \
       k21, k22,  \
       k31, k32, \
       k41, k42, \
       k51, k52  \
) \
{ \
    {  k01, k02 }, \
    {  k11, k12 }, \
    {  k21, k22 }, \
    {  k31, k32 }, \
    {  k41, k42 }, \
    {  k51, k52 } \
}

#endif
