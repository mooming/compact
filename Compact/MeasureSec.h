#pragma once

#include <cstdint>

class MeasureSec
{
   using MilliSec = uint64_t;

private:
   float& deltaTime;
   MilliSec start;

public:
   MeasureSec(float& deltaTime) : deltaTime(deltaTime)
   {
      start = GetTimeMilli();
   }

   ~MeasureSec()
   {
      deltaTime = 0.001f * (GetTimeMilli() - start);
   }

private:
   static MilliSec GetTimeMilli();
};
