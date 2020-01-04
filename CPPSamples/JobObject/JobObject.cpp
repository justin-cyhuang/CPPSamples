// JobObject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <ntstatus.h>

class MyJobObj
{
public:
    static HRESULT MainHR(void)
    {
        HANDLE hProcess, hJobObject;
        HANDLE hJobProcess, hJobMainThread;
        NTSTATUS status = STATUS_SUCCESS;
        BOOL bResult = false;
        DWORD dwExitCode;
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION exLimitInfo = { 0 };
        std::string sCommandLine = "notepad.exe";
        LPWSTR pwszCommand;

        //Create the JobObject
        hJobObject = CreateJobObjectW(NULL, NULL);

        if (hJobObject == NULL)
        {
            //todo
        }

        //Assign the JobObjectInforamtionFlag
        exLimitInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

        bResult = SetInformationJobObject(hJobObject, JobObjectExtendedLimitInformation, &exLimitInfo, sizeof(exLimitInfo));
        if (!bResult)
        {
            //todo
        }

        //Attach the current process to the jobobject
        bResult = AssignProcessToJobObject(hJobObject, ::GetCurrentProcess());
        if (!bResult)
        {
            //todo
        }

        //Then let's put some child process into the job object.
        std::cout << "Let's start a notepad as child process!\n";
        size_t OutSize;
        pwszCommand = (LPWSTR)malloc(sizeof(wchar_t));
        mbstowcs_s(&OutSize, pwszCommand, sCommandLine.length() + 1, sCommandLine.c_str(), sCommandLine.length());
        //mbstowcs(pwszCommand, sCommandLine.c_str(), sCommandLine.length());

        LaunchProcess(
            pwszCommand,
            0,
            &hJobProcess,
            &hJobMainThread
        );

        bResult = AssignProcessToJobObject(hJobObject, hJobProcess);
        //
        // Wait for the worker process to exit
        //
        switch (WaitForSingleObject(hJobProcess, INFINITE))
        {
        case WAIT_OBJECT_0:
            bResult = ::GetExitCodeProcess(hJobProcess, &dwExitCode);
            wprintf(L"Child process exited with exit code %d.\n", dwExitCode);
            break;

        default:
            return STATUS_ABANDONED;
        }
    }


private:
    static HRESULT LaunchProcess(
        __in LPWSTR pwszCommandLine,
        __in DWORD dwCreationFlags,
        __out HANDLE* phProcess,
        __out HANDLE* phPrimaryThread
    )
    {
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };

        si.cb = sizeof(si);
        if (!CreateProcess(
            NULL,
            pwszCommandLine,
            NULL,
            NULL,
            FALSE,
            dwCreationFlags,
            NULL,
            NULL,
            &si,
            &pi))
        {
            //todo
        }

        //*phPrimaryThread = pi.hThread;
        *phProcess = pi.hProcess;
        return STATUS_SUCCESS;
    }
};

int main()
{
    HRESULT hr;
    std::cout << "Hello World!\n";
    MyJobObj* cMainObj = new MyJobObj;

    hr = cMainObj->MainHR();

    if (FAILED(hr))
    {
        wprintf(L"HRESULT: 0x%08X\n", hr);
    }
    else
    {
        wprintf(L"Success.\n");
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

