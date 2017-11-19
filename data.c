#include "data.h"
/*
unsigned char file1[]={'a','b'};
unsigned char file2[]={'1','2'};
unsigned char file3[]={'z','y'};
*/


unsigned char data_netcat[3] = {0x20,0x21,0x22};




PACKFILE fileTable[]={
    {TEXT("a.txt"),sizeof(data_netcat),data_netcat,INS_PATH_CURRENT,NULL}
};

DWORD GetPackFileTable(LPPACKFILE *lpPackFilePointer) {
    *lpPackFilePointer = fileTable;
    return sizeof(fileTable) / sizeof(*fileTable);
}
