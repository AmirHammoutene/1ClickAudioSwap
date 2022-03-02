#include "mainwidget.h"
#include <QApplication>
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <string.h>

void killProcessByName(const wchar_t *filename)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
        if (std::wcscmp(pEntry.szExeFile, filename) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD) pEntry.th32ProcessID);
            if (hProcess != NULL && pEntry.th32ProcessID != GetCurrentProcessId())
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

int main(int argc, char *argv[])
{
    killProcessByName(L"1ClickAudioSwap.exe");

    QApplication a(argc, argv);
    QApplication::setOrganizationName("Amir Hammoutene");
    a.setWindowIcon(QIcon(":/img/icon.png"));

    MainWidget w;
    w.hide();
    return a.exec();
}
