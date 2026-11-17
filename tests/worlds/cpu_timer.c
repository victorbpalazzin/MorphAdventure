/* Process CPU time, separate from wall time and observed presentation rate. */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
double WorldsTestCpuSeconds(void)
{
    FILETIME creation,exit_time,kernel,user;
    if(!GetProcessTimes(GetCurrentProcess(),&creation,&exit_time,&kernel,&user))return -1;
    ULARGE_INTEGER k,u;
    k.LowPart=kernel.dwLowDateTime;k.HighPart=kernel.dwHighDateTime;
    u.LowPart=user.dwLowDateTime;u.HighPart=user.dwHighDateTime;
    return (double)(k.QuadPart+u.QuadPart)*1e-7;
}
#else
#include <time.h>
double WorldsTestCpuSeconds(void) { return (double)clock()/CLOCKS_PER_SEC; }
#endif
