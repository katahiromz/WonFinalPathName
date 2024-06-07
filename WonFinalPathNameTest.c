#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <shlwapi.h>
#include <fileapi.h>
#include <strsafe.h>
#include "WonFinalPathName.h"

int main(int argc, char **argv)
{
    CHAR Path[1024];
    HANDLE hFile;
    DWORD ret;

    if (argc != 2)
    {
        printf("Usage: %s <filename>\n", PathFindFileNameA(argv[0]));
        return 0;
    }

    hFile = CreateFileA(argv[1],
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Could not open file (error %d\n)", GetLastError());
        return 0;
    }

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NT);
    printf("NT: %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NT);
    printf("NT: %s (%ld)\n", Path, ret);

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_DOS);
    printf("DOS: %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_DOS);
    printf("DOS: %s (%ld)\n", Path, ret);

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NONE);
    printf("NONE: %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NONE);
    printf("NONE: %s (%ld)\n", Path, ret);

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_GUID);
    printf("GUID: %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_GUID);
    printf("GUID: %s (%ld)\n", Path, ret);

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NT | FILE_NAME_OPENED);
    printf("NT (Opened): %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NT | FILE_NAME_OPENED);
    printf("NT (Opened): %s (%ld)\n", Path, ret);

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_DOS | FILE_NAME_OPENED);
    printf("DOS (Opened): %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_DOS | FILE_NAME_OPENED);
    printf("DOS (Opened): %s (%ld)\n", Path, ret);

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NONE | FILE_NAME_OPENED);
    printf("NONE (Opened): %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_NONE | FILE_NAME_OPENED);
    printf("NONE (Opened): %s (%ld)\n", Path, ret);

    ret = GetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_GUID | FILE_NAME_OPENED);
    printf("GUID (Opened): %s (%ld)\n", Path, ret);
    ret = WonGetFinalPathNameByHandleA(hFile, Path, _countof(Path), VOLUME_NAME_GUID | FILE_NAME_OPENED);
    printf("GUID (Opened): %s (%ld)\n", Path, ret);

    CloseHandle(hFile);
    return 0;
}
