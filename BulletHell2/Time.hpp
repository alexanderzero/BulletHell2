#pragma once

#include "stdint.h"

typedef uint64_t microseconds;
typedef uint64_t milliseconds;

void timeStart();
microseconds timeCurrentMicroseconds();

void timeSleep(milliseconds ms=1);