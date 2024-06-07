#pragma once

#ifndef FILE_NAME_NORMALIZED
    #define FILE_NAME_NORMALIZED 0x0
    #define FILE_NAME_OPENED 0x8
#endif
#ifndef VOLUME_NAME_DOS
    #define VOLUME_NAME_DOS 0x0
    #define VOLUME_NAME_GUID 0x1
    #define VOLUME_NAME_NT 0x2
    #define VOLUME_NAME_NONE 0x4
#endif

DWORD WINAPI
WonGetFinalPathNameByHandleA(
    HANDLE hFile,
    LPSTR lpszFilePath,
    DWORD  cchFilePath,
    DWORD  dwFlags);

DWORD WINAPI
WonGetFinalPathNameByHandleW(
    HANDLE hFile,
    LPWSTR lpszFilePath,
    DWORD  cchFilePath,
    DWORD  dwFlags);

#ifdef UNICODE
    #define WonGetFinalPathNameByHandle WonGetFinalPathNameByHandleW
#else
    #define WonGetFinalPathNameByHandle WonGetFinalPathNameByHandleA
#endif
