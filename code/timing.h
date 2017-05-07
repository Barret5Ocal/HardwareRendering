#define TIMING_DEBUG 0

// NOTE(barret): debugging needs <stdio.h>

struct time_info 
{
    LARGE_INTEGER LastCounter;
    LARGE_INTEGER EndCounter;
    //NOTE(barret): The Frequency is the number of system counts per second
    LARGE_INTEGER SystemPerformanceFrequency;
    bool SleepIsGranular;
    float TargetSecondsPerFrame; 
    double SecondsElapsed; 
    bool IsInitialized; 
};

inline LARGE_INTEGER 
GetWallClock()
{
    LARGE_INTEGER Result; 
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline float
GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End, LARGE_INTEGER Frequency)
{
    float Result = ((float)(End.QuadPart - Start.QuadPart) /
                     (float)Frequency.QuadPart);
    return(Result);
}

bool
RunLoop(time_info *TimeInfo, bool Running, int FrameRate)
{
    if(!TimeInfo->IsInitialized)
    {    
        QueryPerformanceCounter(&TimeInfo->SystemPerformanceFrequency);

        UINT DesiredSchedulerMS = 1;
        TimeInfo->SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

        TimeInfo->TargetSecondsPerFrame = 1.0f / (float)FrameRate;

        TimeInfo->LastCounter = GetWallClock(); 

        TimeInfo->IsInitialized = 1;
        return 1; 
    }
    else
    {
        DWORD SleepMS;
        if (Running)
        {        
            TimeInfo->EndCounter = GetWallClock();
            TimeInfo->SecondsElapsed = GetSecondsElapsed(TimeInfo->LastCounter, TimeInfo->EndCounter, TimeInfo->SystemPerformanceFrequency); 
            if (TimeInfo->SecondsElapsed < TimeInfo->TargetSecondsPerFrame)
            {
                if (TimeInfo->SleepIsGranular)
                {
                    SleepMS = (DWORD)(1000.0f * (TimeInfo->TargetSecondsPerFrame - TimeInfo->SecondsElapsed));

                    if (SleepMS > 0)
                    {
                        Sleep(SleepMS);
                    }
                }
            }

#if TIMING_DEBUG 
            double FPS = 0.0f;
            double SecondsPerFrame = (double)TimeInfo->SecondsElapsed; 
            double MSPerFrame = 1000.0f * SecondsPerFrame;
        
            char FPSBuffer[256];
            _snprintf_s(FPSBuffer, sizeof(FPSBuffer),
                        "SleepMS: %.02f MSPerFrame: %.02f \n", (double)SleepMS, MSPerFrame);
            OutputDebugStringA(FPSBuffer);
#endif

            TimeInfo->LastCounter = TimeInfo->EndCounter;
            return (1);
        }
        else
        {
            return (0);
        }
    }
}
