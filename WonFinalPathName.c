#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <shlwapi.h>
#include <fileapi.h>
#include <strsafe.h>
#include "WonFinalPathName.h"

#define IsConsoleHandle(h) ((((ULONG_PTR)h) & 0x10000003) == 0x3)

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef enum OBJECT_INFORMATION_CLASS
{
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectAllInformation,
    ObjectDataInformation
} OBJECT_INFORMATION_CLASS;

typedef NTSTATUS (NTAPI* FN_NtQueryObject)(HANDLE Handle, OBJECT_INFORMATION_CLASS Info, PVOID Buffer, ULONG BufferSize, PULONG ReturnLength);

typedef struct OBJECT_NAME_INFORMATION
{
    UNICODE_STRING Name;
    WCHAR NameBuffer;
} OBJECT_NAME_INFORMATION;

static FN_NtQueryObject NtQueryObject()
{
    static FN_NtQueryObject s_pNtQueryObject = NULL;
    if (!s_pNtQueryObject)
    {
        HMODULE hNTDLL = GetModuleHandleA("ntdll");
        s_pNtQueryObject = (FN_NtQueryObject)GetProcAddress(hNTDLL, "NtQueryObject");
    }
    return s_pNtQueryObject;
}

static DWORD
GetNtPathFromHandle(HANDLE hFile, LPWSTR pszPath, DWORD cchPath)
{
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
        return ERROR_INVALID_HANDLE;

    if (IsConsoleHandle(hFile))
    {
        StringCchPrintfW(pszPath, cchPath, L"\\Device\\Console%04X", (DWORD)(DWORD_PTR)hFile);
        return 0;
    }

    BYTE  Buffer[2000];
    DWORD cbLength;
    UNICODE_STRING* Name = &((OBJECT_NAME_INFORMATION*)Buffer)->Name;
    NtQueryObject()(hFile, ObjectNameInformation, Buffer, sizeof(Buffer), &cbLength);

    if (!Name->Buffer || !Name->Length)
        return ERROR_FILE_NOT_FOUND;

    Name->Buffer[Name->Length / sizeof(WCHAR)] = UNICODE_NULL;

    StringCchCopyW(pszPath, cchPath, Name->Buffer);
    return 0;
}

static DWORD GetDosPathFromNtPath(LPCWSTR pszNTPath, LPWSTR pszDosPath, DWORD cchDosPath)
{
    DWORD error, cbSize;
    WCHAR szComPort[50], szDrives[300];
    LPWSTR pchDrive, pchNext;
    WCHAR szVolume[1000];
    INT cchVolume;
    HKEY hKey;

    if (wcsnicmp(pszNTPath, L"\\Device\\Serial", 14) == 0 ||
        wcsnicmp(pszNTPath, L"\\Device\\UsbSer", 14) == 0)
    {
        error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Hardware\\DeviceMap\\SerialComm", 0, KEY_READ, &hKey);
        if (error)
            return error;

        cbSize = sizeof(szComPort);
        error = RegQueryValueExW(hKey, pszNTPath, 0, NULL, (BYTE*)szComPort, &cbSize);
        if (error)
        {
            RegCloseKey(hKey);
            return ERROR_UNKNOWN_PORT;
        }

        StringCchCopyW(pszDosPath, cchDosPath, szComPort);
        RegCloseKey(hKey);
        return 0;
    }

    if (wcsnicmp(pszNTPath, L"\\Device\\LanmanRedirector\\", 25) == 0)
    {
        StringCchCopyW(pszDosPath, cchDosPath, L"\\\\");
        StringCchCatW(pszDosPath, cchDosPath, &pszNTPath[25]);
        return 0;
    }

    if (wcsnicmp(pszNTPath, L"\\Device\\Mup\\", 12) == 0)
    {
        StringCchCopyW(pszDosPath, cchDosPath, L"\\\\");
        StringCchCatW(pszDosPath, cchDosPath, &pszNTPath[12]);
        return 0;
    }

    if (!GetLogicalDriveStringsW(_countof(szDrives), szDrives))
        return GetLastError();

    pchDrive = szDrives;
    while (pchDrive[0])
    {
        pchNext = pchDrive + wcslen(pchDrive) + 1;
        pchDrive[2] = 0;

        szVolume[0] = 0;
        if (!QueryDosDeviceW(pchDrive, szVolume, _countof(szVolume)))
            return GetLastError();

        cchVolume = lstrlenW(szVolume);
        if (cchVolume > 0 && wcsnicmp(pszNTPath, szVolume, cchVolume) == 0)
        {
            StringCchCopyW(pszDosPath, cchDosPath, pchDrive);
            StringCchCatW(pszDosPath, cchDosPath, &pszNTPath[cchVolume]);
            return 0;
        }

        pchDrive = pchNext;
    }

    return ERROR_BAD_PATHNAME;
}

DWORD WINAPI
WonGetFinalPathNameByHandleW(
    HANDLE hFile,
    LPWSTR lpszFilePath,
    DWORD  cchFilePath,
    DWORD  dwFlags)
{
    DWORD error;
    WCHAR szNTPath[MAX_PATH];

    lpszFilePath[0] = 0;

    error = GetNtPathFromHandle(hFile, szNTPath, _countof(szNTPath));
    if (error)
    {
        SetLastError(error);
        return 0;
    }

    if (dwFlags & VOLUME_NAME_NT)
    {
        StringCchCopyW(lpszFilePath, cchFilePath, szNTPath);
    }
    else if (dwFlags & VOLUME_NAME_GUID)
    {
        WCHAR szDosPath[MAX_PATH], szVolume[MAX_PATH], szRoot[64];
        error = GetDosPathFromNtPath(szNTPath, szDosPath, _countof(szDosPath));
        lstrcpynW(szRoot, szDosPath, _countof(szRoot));
        PathStripToRootW(szRoot);
        if (!GetVolumeNameForVolumeMountPointW(szRoot, szVolume, _countof(szVolume)))
        {
            error = ERROR_CALL_NOT_IMPLEMENTED;
        }
        else
        {
            StringCchCopyW(lpszFilePath, cchFilePath, szVolume);
            StringCchCatW(lpszFilePath, cchFilePath, &szDosPath[lstrlenW(szRoot)]);
        }
    }
    else if (dwFlags & VOLUME_NAME_NONE)
    {
        error = GetDosPathFromNtPath(szNTPath, lpszFilePath, cchFilePath);
        if (cchFilePath > 3 && lpszFilePath[1] == L':' && lpszFilePath[2] == L'\\')
        {
            MoveMemory(&lpszFilePath[0], &lpszFilePath[2], (lstrlenW(lpszFilePath) + 1) * sizeof(WCHAR));
        }
    }
    else
    {
        if (cchFilePath < 4)
        {
            error = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            StringCchCopyW(lpszFilePath, cchFilePath, L"\\\\?\\");
            error = GetDosPathFromNtPath(szNTPath, &lpszFilePath[4], cchFilePath - 4);
        }
    }

    if (error)
    {
        SetLastError(error);
        return 0;
    }

    SetLastError(ERROR_SUCCESS);
    return lstrlenW(lpszFilePath);
}

DWORD WINAPI
WonGetFinalPathNameByHandleA(
    HANDLE hFile,
    LPSTR lpszFilePath,
    DWORD  cchFilePath,
    DWORD  dwFlags)
{
    WCHAR szPath[MAX_PATH];
    DWORD ret;

    lpszFilePath[0] = 0;

    ret = WonGetFinalPathNameByHandleW(hFile, szPath, _countof(szPath), dwFlags);
    if (ret == 0)
        return 0;

    WideCharToMultiByte(CP_ACP, 0, szPath, -1, lpszFilePath, cchFilePath, NULL, NULL);
    return lstrlenA(lpszFilePath);
}
