#include "MeasureSec.h"

#include <chrono>

MeasureSec::MilliSec MeasureSec::GetTimeMilli()
{
   using namespace std::chrono;
   const milliseconds ms = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
   return ms.count();
}
