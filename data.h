#ifndef DATA_H
#define DATA_H
#include <windows.h>
#define SERVICE_NAME "test_service"
#define SERVICE_DISPLAY_NAME "Test Service"
#define SERVICE_FILE "test.exe"
//#define SERVICE_BIN_PATH INS_PATH_SYSTEM
#define MAKE_DIR "test"

enum INS_PATH{
    INS_PATH_SYSTEM,
    INS_PATH_WINDOWS,
    INS_PATH_TEMP,
    INS_PATH_CURRENT,

};

/*
#ifndef TEXT
#ifdef UNICODE
#define TEXT(q) L##q
#else
#define TEXT(q)
#endif
#ifndef TCHAR
#ifdef UNICODE
#define TCHAR wchar_t
#else
#define TCHAR char
#endif
#endif
#endif
*/
typedef struct PACKFILE_STRUCT
{
    TCHAR *szFileName;
    DWORD dwFileSize;
    BYTE  *byFileData;
    enum INS_PATH installPath;
    TCHAR *szSubDir;
} PACKFILE, *LPPACKFILE;

//extern PACKFILE fileTable[];
extern DWORD GetPackFileTable(LPPACKFILE *lpPackFilePointer);
//extern DWORD g_dwFileTableLength;
#endif // DATA_H
