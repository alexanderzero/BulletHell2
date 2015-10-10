#include "Time.hpp"

#include "windows.h"

bool g_timeStarted = false;
uint64_t g_clockFreq;
microseconds g_startTime;


void timeStart()
{
   if (g_timeStarted) return;
   g_timeStarted = true;
   QueryPerformanceFrequency((LARGE_INTEGER*)&g_clockFreq);
   QueryPerformanceCounter((LARGE_INTEGER*)&g_startTime);
}
microseconds timeCurrentMicroseconds()
{
   timeStart(); //if not already initialized...

   microseconds out;
   QueryPerformanceCounter((LARGE_INTEGER*)&out);
   out -= g_startTime;
   return (out * 1000000) / g_clockFreq;
}


void timeSleep(milliseconds ms)
{
   Sleep((DWORD)ms);
}