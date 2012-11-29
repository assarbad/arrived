///////////////////////////////////////////////////////////////////////////////
///
/// Written 2008-2009, Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
/// (with the exception of parts that are (C) FRISK Software International)
///
/// Original filename: arrived.cpp
/// Project          : arrived
/// Date of creation : 2008-11-25
/// Author(s)        : Oliver Schneider
///
/// Purpose          : Allows to execute a script if devices, such as USB
///                    drives arrive in the system.
///
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// #define UNICODE
// #define _UNICODE
// These two defines are given implicitly through the settings of C_DEFINES in
// the SOURCES file of the project. Hence change them there and there only.
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <tchar.h>
#include "execommon.h"
#include "exeversion.h"
#include <Windows.h>
#include <ShellAPI.h>
#include <Dbt.h>
#include <common/SimpleBuffer.h>

#define LIST_SEPARATOR ','
namespace
{
    const DWORD PATIENT_THREAD_SLEEPTIME = 15; // in seconds
    const _TCHAR DriveTypeSwitch[] = TEXT("-only=");
    static CSimpleBuf<_TCHAR> g_command;
    static HWND g_hwnd = NULL;
    static HANDLE g_hArrivalThread = NULL;

    struct
    {
        _TCHAR* name;
        BOOL    included;
        UINT    type;
    } DriveTypeList[] =
    {
        { TEXT("unknown"),      FALSE, DRIVE_UNKNOWN },
        { TEXT("noroot"),       FALSE, DRIVE_NO_ROOT_DIR },
        { TEXT("removable"),    FALSE, DRIVE_REMOVABLE },
        { TEXT("fixed"),        FALSE, DRIVE_FIXED },
        { TEXT("remote"),       FALSE, DRIVE_REMOTE },
        { TEXT("cdrom"),        FALSE, DRIVE_CDROM },
        { TEXT("ramdisk"),      FALSE, DRIVE_RAMDISK }
    };

    DWORD ExecuteCommand(bool bArriving, _TCHAR driveletter)
    {
        CSimpleBuf<_TCHAR> parameters(bArriving ? TEXT("arrival") : TEXT("removal"));
        if(driveletter)
        {
            parameters += CSimpleBuf<_TCHAR>::format(TEXT(" %c:"), driveletter);
        }
        STARTUPINFO si = { sizeof(si), 0 ,0 ,0 ,0 ,0 ,0 ,0 , 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 };
        PROCESS_INFORMATION pi = { 0, 0, 0, 0 };
        ::GetStartupInfo(&si);
        CSimpleBuf<_TCHAR> command(g_command + TEXT(" ") + parameters);
        _tprintf(TEXT("\t-> Executing '%s'\n"), command.Buffer());

        BOOL created = ::CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

        if(created)
        {
            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);
            _tprintf(TEXT("\tDone.\n"));
        }
        else
        {
            _tprintf(TEXT("\tFailed (%d).\n"), ::GetLastError());
        }
        return 0;
    }
}

DWORD WINAPI PatientArrivalThread(__in  LPVOID lpParameter)
{
    _tprintf(TEXT("\t-> Patiently waiting %d seconds for drive %c: to settle.\n"), PATIENT_THREAD_SLEEPTIME, reinterpret_cast<_TCHAR>(lpParameter));
    ::Sleep(PATIENT_THREAD_SLEEPTIME * 1000);
    return ExecuteCommand(true, reinterpret_cast<_TCHAR>(lpParameter));
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_CREATE:
        _tprintf(TEXT("Waiting for devices to arrive or depart ...\n"));
        _tprintf(TEXT("Command: \"%s\"\n"), g_command.Buffer());
        _tprintf(TEXT("(Press Ctrl+C to quit!)\n\n"));
        break;
    case WM_DEVICECHANGE:
        switch(wParam)
        {
        case DBT_CONFIGCHANGECANCELED: // A request to change the current configuration (dock or undock) has been canceled.
            _tprintf(TEXT("* DBT_CONFIGCHANGECANCELED\n"));
            break;
        case DBT_CONFIGCHANGED: // The current configuration has changed, due to a dock or undock.
            _tprintf(TEXT("* DBT_CONFIGCHANGED\n"));
            break;
        case DBT_CUSTOMEVENT: // A custom event has occurred.
            _tprintf(TEXT("* DBT_CUSTOMEVENT\n"));
            break;
        case DBT_DEVICEARRIVAL: // A device or piece of media has been inserted and is now available.
            {
                _tprintf(TEXT("* DBT_DEVICEARRIVAL\n"));
                DEV_BROADCAST_HDR* msg = reinterpret_cast<DEV_BROADCAST_HDR*>(lParam);
                switch(msg->dbch_devicetype)
                {
                case DBT_DEVTYP_VOLUME:
                    {
                        _tprintf(TEXT("\t* DBT_DEVTYP_VOLUME\n"));
                        DEV_BROADCAST_VOLUME* vol = reinterpret_cast<DEV_BROADCAST_VOLUME*>(msg);
                        for(size_t i = 0; i < 8*sizeof(vol->dbcv_unitmask); i++)
                        {
                            if(vol->dbcv_unitmask & (1 << i))
                            {
                                DWORD dwThreadId = 0;
                                UINT const currtype = ::GetDriveType(CSimpleBuf<_TCHAR>::format(TEXT("%c:"), 'A' + i));
                                for(size_t j = 0; j < sizeof(DriveTypeList)/sizeof(DriveTypeList[0]); j++)
                                {
                                    if(currtype == DriveTypeList[j].type)
                                    {
                                        _tprintf(TEXT("\t\tDrive %c: (%s) arrived\n"), 'A' + i, DriveTypeList[j].name);
                                        if(DriveTypeList[j].included)
                                        {
                                            ::CreateThread(NULL, 0, PatientArrivalThread, reinterpret_cast<LPVOID>('A' + i), 0, &dwThreadId);
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                default:
                    _tprintf(TEXT("\t%08X\n"), msg->dbch_devicetype);
                    break;
                }
            }
            break;
        case DBT_DEVICEQUERYREMOVE: // Permission is requested to remove a device or piece of media. Any application can deny this request and cancel the removal.
            _tprintf(TEXT("* DBT_DEVICEQUERYREMOVE\n"));
            break;
        case DBT_DEVICEQUERYREMOVEFAILED: // A request to remove a device or piece of media has been canceled.
            _tprintf(TEXT("* DBT_DEVICEQUERYREMOVEFAILED\n"));
            break;
        case DBT_DEVICEREMOVECOMPLETE: // A device or piece of media has been removed.
            {
                _tprintf(TEXT("* DBT_DEVICEREMOVECOMPLETE\n"));
                DEV_BROADCAST_HDR* msg = reinterpret_cast<DEV_BROADCAST_HDR*>(lParam);
                switch(msg->dbch_devicetype)
                {
                case DBT_DEVTYP_VOLUME:
                    {
                        _tprintf(TEXT("\t* DBT_DEVTYP_VOLUME\n"));
                        DEV_BROADCAST_VOLUME* vol = reinterpret_cast<DEV_BROADCAST_VOLUME*>(msg);
                        for(size_t i = 0; i < 8*sizeof(vol->dbcv_unitmask); i++)
                        {
                            if(vol->dbcv_unitmask & (1 << i))
                            {
                                DWORD dwThreadId = 0;
                                _tprintf(TEXT("\t\tDrive %c: removed\n"), 'A' + i);
                            }
                        }
                    }
                    break;
                default:
                    _tprintf(TEXT("\t%08X\n"), msg->dbch_devicetype);
                    break;
                }
            }
            break;
        case DBT_DEVICEREMOVEPENDING: // A device or piece of media is about to be removed. Cannot be denied.
            _tprintf(TEXT("* DBT_DEVICEREMOVEPENDING\n"));
            break;
        case DBT_DEVICETYPESPECIFIC: // A device-specific event has occurred.
            _tprintf(TEXT("* DBT_DEVICETYPESPECIFIC\n"));
            break;
        case DBT_DEVNODES_CHANGED: // A device has been added to or removed from the system.
            _tprintf(TEXT("* DBT_DEVNODES_CHANGED\n"));
            break;
        case DBT_QUERYCHANGECONFIG: // Permission is requested to change the current configuration (dock or undock).
            _tprintf(TEXT("* DBT_QUERYCHANGECONFIG\n"));
            break;
        default:
            _tprintf(TEXT("* WM_DEVICECHANGE.wParam(%d)\n"), wParam);
        }
        break;
    }
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam); 
}

BOOL WINAPI ConsoleCtrlHandlerRoutine(__in  DWORD dwCtrlType)
{
    _tprintf(TEXT("\nCaught signal (%d)\n"), dwCtrlType);
    ::PostMessage(g_hwnd, WM_QUIT, 0, 0);
    ::SuspendThread(g_hArrivalThread);
    return TRUE;
}

int __cdecl _tmain(int argc, _TCHAR *argv[])
{
    // React to Ctrl+C and friends
    ::SetConsoleCtrlHandler(ConsoleCtrlHandlerRoutine, TRUE);

    _tprintf(TEXT("arrived %1d.%1d/%03d - written 2008-2009, Oliver Schneider (assarbad.net)\n")
             TEXT("   Portions Copyright (c) 2007-2008, FRISK Software International\n\n"), PRD_MAJVER, PRD_MINVER, FILE_BUILD);
    switch(argc)
    {
    case 2:
        // Set the default
        for(size_t i = 0; i < sizeof(DriveTypeList)/sizeof(DriveTypeList[0]); i++)
        {
            switch(DriveTypeList[i].type)
            {
            case DRIVE_REMOVABLE:
                DriveTypeList[i].included = TRUE;
                break;
            default:
                DriveTypeList[i].included = FALSE;
                break;
            }
        }
        break;
    case 3:
        {
            // Get out if no explicit switch was given
            if(argc < 3)
            {
                break;
            }
            CSimpleBuf<_TCHAR> argument(argv[argc-1]);
            // Check the syntax
            if(0 == _tcsnicmp(DriveTypeSwitch, argument, _tcslen(DriveTypeSwitch)))
            {
                // We're interested in prepending a comma
                argument[_tcslen(DriveTypeSwitch)-1] = TEXT(LIST_SEPARATOR);
                // Skip over the switch to get to the list
                argument = CSimpleBuf<_TCHAR>(argument.Buffer() + (_tcslen(DriveTypeSwitch) - 1));

                while(_TCHAR* pos = _tcsrchr(argument, TEXT(LIST_SEPARATOR)))
                {
                    *pos++ = 0; // zero-terminate here and step over what was the comma
                    for(size_t i = 0; i < sizeof(DriveTypeList)/sizeof(DriveTypeList[0]); i++)
                    {
                        if(0 == _tcsicmp(pos, DriveTypeList[i].name))
                        {
                            DriveTypeList[i].included = TRUE;
                            _tprintf(TEXT("\tIncluding '%s' drives\n"), DriveTypeList[i].name);
                            break;
                        }
                    }
                }

                break;
            } // fall through if the syntax was wrong
        }
    default:
        {
            _tprintf(TEXT("Syntax:\n"));
            _tprintf(TEXT("\tarrived <command> [-only=drive_type,...]\n\n"));
            _tprintf(TEXT("\tThe drive types are:\n"));
            for(size_t i = 0; i < sizeof(DriveTypeList)/sizeof(DriveTypeList[0]); i++)
            {
                _tprintf(TEXT("\t- %s\n"), DriveTypeList[i].name);
            }
            _tprintf(TEXT("NOTE: The default, if the '-only' switch is not explicitly given is 'removable'\n"));
            return 1;
        }
    }
    g_command = argv[1];

    WNDCLASSEX wcx = {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW,
        MainWndProc,
        0,
        0,
        ::GetModuleHandle(NULL),
        NULL,
        NULL,
        NULL,
        MAKEINTRESOURCE(0),
        TEXT("{4A51A206-7BCA-4afd-B394-43E4337AFD0A}"),
        NULL
    };
    RegisterClassEx(&wcx);

    g_hwnd =
        ::CreateWindow( 
            wcx.lpszClassName,
            TEXT("{955ED093-CA16-4c42-95DE-1CE176AA46F4}"),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            ::GetModuleHandle(NULL),
            NULL
        );

    MSG msg; 
    BOOL fGotMessage = FALSE;
    while (((fGotMessage = ::GetMessage(&msg, (HWND) NULL, 0, 0)) != 0) && (fGotMessage != -1)) 
    { 
        ::TranslateMessage(&msg); 
        ::DispatchMessage(&msg); 
    } 
    return (int)msg.wParam; 
}
