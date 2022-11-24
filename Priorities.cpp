#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>

void SetProcessPriority(LPWSTR ProcessName, int Priority)
{
    PROCESSENTRY32 proc32;
    HANDLE hSnap;
    if (hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (hSnap == INVALID_HANDLE_VALUE)
    {

    }
    else
    {
        proc32.dwSize = sizeof(PROCESSENTRY32);
        while ((Process32Next(hSnap, &proc32)) == TRUE)
        {
            if (_wcsicmp(proc32.szExeFile, ProcessName) == 0)
            {
                HANDLE h = OpenProcess(PROCESS_SET_INFORMATION, TRUE, proc32.th32ProcessID);
                SetPriorityClass(h, Priority);
                CloseHandle(h);
            }
        }
        CloseHandle(hSnap);
    }
}