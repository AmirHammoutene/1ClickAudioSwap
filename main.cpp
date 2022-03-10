/**
 * Conceived and developed by Amir Hammoutene (amir.hammoutene@gmail.com) in March 2022.
 * Under GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 *
 * This program is a Windows system tray tool that permits to swap between 2 default audio devices.
 *
 * The code uses a lot of Qt framework version 6. You'll find a bunch of documentation about Qt classes on internet and principally on https://doc.qt.io/
**/

#include "mainwidget.h"
#include <QApplication>
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <string.h>

// Kill a Windows process with his name
// Used to kill any 1ClickAudioSwap.exe to be sure there will be only one instance
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
            if (hProcess != NULL && pEntry.th32ProcessID != GetCurrentProcessId()) // Don't kill self
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
    w.hide(); // since it's a system tray app, we don't show any window
    return a.exec();
}
