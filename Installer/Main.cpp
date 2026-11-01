#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <thread>
#include <ShlObj.h>

// Common Controls library
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "resource.h"
#include "GameLocationManager.h"
#include "InstallationManager.h"

// Global Control Identifiers
#define IDC_INSTALL_BUTTON 1001
#define IDC_PROGRESS_BAR   1002
#define IDC_CHECKBOX_USERFILES     1003
#define IDC_CHECKBOX_USERFILES_OVERWRITE     1004
#define IDC_STATUS_TEXT    1005

HWND hStatusText;
HWND hProgressBar;
HWND hInstallBtn;
HWND hCheckboxUserFiles;
HWND hCheckboxUserFilesOverwrite;

// White brush to cleanly paint background underneath text and checkboxes
HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(255, 255, 255));

// Dummy installation task to simulate file extraction
void RunInstallation(HWND hWnd) 
{
    bool bOverwriteUserFiles = IsWindowEnabled(hCheckboxUserFilesOverwrite);

    SetWindowText(hStatusText, L"Status: Finding game location...");
    EnableWindow(hInstallBtn, FALSE); // Disable button during install    
    EnableWindow(hCheckboxUserFiles, FALSE);     // Disable checkboxes so settings can't change
    EnableWindow(hCheckboxUserFilesOverwrite, FALSE);

    // Reset progress bar to zero
    SendMessage(hProgressBar, PBM_SETPOS, 0, 0);

    UpdateWindow(hWnd);

    // Read checkbox states
    bool bWriteUserFiles = (SendMessage(hCheckboxUserFiles, BM_GETCHECK, 0, 0) == BST_CHECKED);
    bOverwriteUserFiles = bOverwriteUserFiles && (SendMessage(hCheckboxUserFilesOverwrite, BM_GETCHECK, 0, 0) == BST_CHECKED);

    std::wstring gameFolder = GameLocationManager::FindSteamGameFolder(L"293760", L"Automation");

    if (gameFolder.empty()) 
    {
        // Prompt the user with a directory selection window fallback
        MessageBox(hWnd, L"Failed to fetch game location from Steam", L"Installation Failed", MB_OK | MB_ICONERROR);
        SendMessage(hWnd, WM_CLOSE, 0, 0);
    }
    SendMessage(hProgressBar, PBM_SETPOS, 25, 0);

    // Install binaries
    SetWindowText(hStatusText, L"Status: Unpacking binaries...");

    std::wstring gameExportPluginsDir = gameFolder + L"\\UE427\\AutomationGame\\Content\\ExportPlugins";
    //MessageBoxW(hWnd, gameExportPluginsDir.c_str(), L"Directory", MB_OK);

    if (!InstallationManager::UnpackResource(IDR_ZIPFILE_BINARIES, gameExportPluginsDir, InstallationManager::UnpackArchive, true))
    {
        MessageBox(hWnd, L"Failed to install exporter binaries", L"Installation Failed", MB_OK | MB_ICONERROR);
        SendMessage(hWnd, WM_CLOSE, 0, 0);
    }
    SendMessage(hProgressBar, PBM_SETPOS, 50, 0);

    // Install readme
    SetWindowText(hStatusText, L"Status: Initializing application data...");
    TCHAR path[MAX_PATH];
    if (!(SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK))
    {
        MessageBox(hWnd, L"Failed to find application data location", L"Installation Failed", MB_OK | MB_ICONERROR);
        SendMessage(hWnd, WM_CLOSE, 0, 0);
    }

    std::wstring appDataDir = std::wstring(path) + L"\\AutomationGame\\CSVExporter 2";
    if (!InstallationManager::UnpackResource(IDR_README, appDataDir, InstallationManager::WriteFile, true, L"ReadMe.txt"))
    {
        MessageBox(hWnd, L"Failed to initialize application data", L"Installation Failed", MB_OK | MB_ICONERROR);
        SendMessage(hWnd, WM_CLOSE, 0, 0);
    }
    SendMessage(hProgressBar, PBM_SETPOS, 75, 0);

    if (bWriteUserFiles) 
    {
        // Install user files
        SetWindowText(hStatusText, L"Status: Extracting user files...");
        std::wstring userFilesFolder = appDataDir + L"\\UserFiles";
        if (!InstallationManager::UnpackResource(IDR_ZIPFILE_USERFILES, userFilesFolder, InstallationManager::UnpackArchive, bOverwriteUserFiles))
        {
            MessageBox(hWnd, L"Failed to unpack user files", L"Installation Failed", MB_OK | MB_ICONERROR);
            SendMessage(hWnd, WM_CLOSE, 0, 0);
        }
    }
    SendMessage(hProgressBar, PBM_SETPOS, 100, 0);

    SetWindowText(hStatusText, L"Status: Installation Complete!");
    MessageBox(hWnd, L"The CSVExporter 2 was successfully installed!", L"Success", MB_OK | MB_ICONINFORMATION);
    SendMessage(hWnd, WM_CLOSE, 0, 0);
}

// Window Procedure to handle GUI events
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) 
    {
        case WM_CREATE: 
        {
            HFONT hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

            HWND hTitle = CreateWindow(L"STATIC", L"Welcome to the CSVExporter installer",
                WS_VISIBLE | WS_CHILD,
                20, 15, 350, 25, hWnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);

            hCheckboxUserFiles = CreateWindow(L"BUTTON", L"Install optional user files",
                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                25, 50, 340, 25, hWnd, (HMENU)IDC_CHECKBOX_USERFILES, NULL, NULL);
            SendMessage(hCheckboxUserFiles, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hCheckboxUserFiles, BM_SETCHECK, BST_CHECKED, 0);

            hCheckboxUserFilesOverwrite = CreateWindow(L"BUTTON", L"Overwrite existing user files",
                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                25, 80, 340, 25, hWnd, (HMENU)IDC_CHECKBOX_USERFILES_OVERWRITE, NULL, NULL);
            SendMessage(hCheckboxUserFilesOverwrite, WM_SETFONT, (WPARAM)hFont, TRUE);

            hStatusText = CreateWindow(L"STATIC", L"Status: Ready to install.",
                WS_VISIBLE | WS_CHILD,
                20, 120, 350, 20, hWnd, (HMENU)IDC_STATUS_TEXT, NULL, NULL);
            SendMessage(hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);
           
            hProgressBar = CreateWindow(PROGRESS_CLASS, L"",
                WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                25, 150, 335, 20, hWnd, (HMENU)IDC_PROGRESS_BAR, NULL, NULL);

            // Define minimum and maximum scale range (0 to 100)
            SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

            hInstallBtn = CreateWindow(L"BUTTON", L"Install Now",
                WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                250, 180, 120, 35, hWnd, (HMENU)IDC_INSTALL_BUTTON, NULL, NULL);
            SendMessage(hInstallBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }

        // Intercept drawing message to make static text and checkbox background transparent
        case WM_CTLCOLORSTATIC: 
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT); // Erases grey backgrounds from text blocks
            return (INT_PTR)hBackgroundBrush;  // Blends control bounding regions cleanly into white
        }

        case WM_COMMAND: 
        {
            int controlId = LOWORD(wParam);
            int notificationCode = HIWORD(wParam);

            if (controlId == IDC_CHECKBOX_USERFILES && notificationCode == BN_CLICKED) 
            {
                LRESULT isChecked = SendMessage(hCheckboxUserFiles, BM_GETCHECK, 0, 0);

                if (isChecked == BST_CHECKED) 
                {
                    EnableWindow(hCheckboxUserFilesOverwrite, TRUE);
                }
                else 
                {
                    EnableWindow(hCheckboxUserFilesOverwrite, FALSE);
                    SendMessage(hCheckboxUserFilesOverwrite, BM_SETCHECK, BST_UNCHECKED, 0);
                }
                // Triggers window to clean-refresh options layout immediately
                InvalidateRect(hWnd, NULL, TRUE);
            }

            if (controlId == IDC_INSTALL_BUTTON) 
            {
                std::thread(RunInstallation, hWnd).detach();
            }
            break;
        }

        case WM_DESTROY:
            DeleteObject(hBackgroundBrush); // Free brush resource allocation
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
    InitCommonControls();

    const wchar_t CLASS_NAME[] = L"InstallerWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // White background
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0, CLASS_NAME, L"CSVExporter Installer",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 270,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL)
    {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
