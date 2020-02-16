#pragma once

#include <windows.h>

class FerTimer
{
public:
	FerTimer();
	void StartWatch();
	void StopWatch();
	float GetTimeMilli() const;
	float GetTimeSec() const;
private:
	float invFreqMilli;
	bool watchStopped;
	unsigned long long currentCount;
	unsigned long long startCount;
};