#include "FerTimer.h"
#pragma comment(lib, "winmm.lib" )

FerTimer::FerTimer()
{
	unsigned long long frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	invFreqMilli = 1.0f / (float)((double)frequency / 1000.0);
	StartWatch();
}

void FerTimer::StopWatch()
{
	if (!watchStopped)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCount);
		watchStopped = true;
	}
}

void FerTimer::StartWatch()
{
	watchStopped = false;
	QueryPerformanceCounter((LARGE_INTEGER*)&startCount);
}

float FerTimer::GetTimeSec() const
{
	return GetTimeMilli() / 1000.0f;
}

float FerTimer::GetTimeMilli() const
{
	if (!watchStopped)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCount);
		return (float)(currentCount - startCount) * invFreqMilli;
	}
	else
	{
		return (float)(currentCount - startCount) * invFreqMilli;
	}
}