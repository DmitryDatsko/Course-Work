#define ID_CREATE_FILE 1
#define ID_OPEN_FILE 2
#define ID_ABOUT 3
#define ID_SAVE_FILE 4
#define ID_FONT_ARIAL 5
#define ID_FONT_TIMES 6
#define ID_FONT_VERDANA 7
#define ID_FONT_CALIBRI 8

#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

using namespace Gdiplus;
using namespace std;

#pragma comment (lib, "Gdiplus.lib")

wstring fileContent;
HINSTANCE hInst;
ULONG_PTR gdiplusToken;
wchar_t globalImagePath[MAX_PATH] = L"";
wchar_t globalIconPath[MAX_PATH] = L"";
Image* backgroundImage = nullptr;

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[] = _T("Coursework");
static int currentFontId = ID_FONT_ARIAL;
static int fontSize = 16;
static HWND hFontSizeInfo;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void CreateAppMenu(HWND hWnd)
{
    HMENU hMenu = CreateMenu();

    AppendMenu(hMenu, MF_STRING, ID_CREATE_FILE, _T("Create File"));
    AppendMenu(hMenu, MF_STRING, ID_OPEN_FILE, _T("Open File"));
    AppendMenu(hMenu, MF_STRING, ID_ABOUT, _T("About"));

    SetMenu(hWnd, hMenu);
}

void ReadFileContent(const wstring& filePath) {
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) {
        MessageBox(nullptr, L"Cannot open file", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    file.seekg(0, ios::end);
    streamsize fileSize = file.tellg();
    file.seekg(0, ios::beg);

    if (fileSize <= 0) {
        MessageBox(nullptr, L"File is empty", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    vector<char> buffer(static_cast<size_t>(fileSize));
    file.read(buffer.data(), fileSize);
    if (!file) {
        MessageBox(nullptr, L"Failed to read file", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    wstring content(reinterpret_cast<wchar_t*>(buffer.data()), buffer.size() / sizeof(wchar_t));

    fileContent = content.c_str();
}

void ShowSaveFileDialog(HWND hWnd, const wstring& dataToWrite)
{
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH] = _T("simple.inf");

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(TCHAR);
    ofn.lpstrFilter = _T("INF Files\0*.inf\0All Files\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = _T("inf");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        HANDLE hFile = CreateFile(
            ofn.lpstrFile,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD bytesWritten;

            const wchar_t bom = 0xFEFF;
            WriteFile(hFile, &bom, sizeof(bom), &bytesWritten, NULL);
            WriteFile(hFile, dataToWrite.c_str(), dataToWrite.size() * sizeof(wchar_t), &bytesWritten, NULL);

            CloseHandle(hFile);

            MessageBox(hWnd, _T("File saved successfully!"), _T("Success"), MB_OK);
            DestroyWindow(hWnd);
        }
        else
        {
            MessageBox(hWnd, _T("Failed to create the file."), _T("Error"), MB_OK);
        }
    }
    else
    {
        MessageBox(hWnd, _T("Save cancelled."), _T("Info"), MB_OK);
    }
}

LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HFONT hFont = nullptr;

    switch (message)
    {
    case WM_KEYDOWN:
        if (GetKeyState(VK_CONTROL) & 0x8000)
        {
            if (wParam == VK_OEM_PLUS || wParam == VK_ADD)
            {
                fontSize++;
            }
            else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT)
            {
                fontSize = max(1, fontSize - 1);
            }

            if (hFont)
            {
                DeleteObject(hFont);
            }

            TCHAR fontSizeText[50];
            wsprintf(fontSizeText, _T("Font Size: %d"), fontSize);
            SetWindowText(hFontSizeInfo, fontSizeText);

            hFont = CreateFont(
                fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));

            SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    }

    return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA), hWnd, message, wParam, lParam);
}

LRESULT CALLBACK FileProcessing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEdit;
    static HFONT hFont = nullptr;
    static Image* background = nullptr;

    switch (message)
    {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_FONT_ARIAL:
            if (hFont) DeleteObject(hFont);
            hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            currentFontId = ID_FONT_ARIAL;
            break;

        case ID_FONT_TIMES:
            if (hFont) DeleteObject(hFont);
            hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, _T("Times New Roman"));
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            currentFontId = ID_FONT_TIMES;
            break;

        case ID_FONT_VERDANA:
            if (hFont) DeleteObject(hFont);
            hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Verdana"));
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            currentFontId = ID_FONT_VERDANA;
            break;

        case ID_FONT_CALIBRI:
            if (hFont) DeleteObject(hFont);
            hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Calibri"));
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            currentFontId = ID_FONT_CALIBRI;
            break;
        }

        HMENU hMenu = GetMenu(hWnd);
        CheckMenuItem(hMenu, ID_FONT_ARIAL, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_FONT_TIMES, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_FONT_VERDANA, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_FONT_CALIBRI, MF_UNCHECKED);

        CheckMenuItem(hMenu, currentFontId, MF_CHECKED);

        break;
    }
    case WM_CREATE:
    {
        hEdit = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            _T("EDIT"),
            fileContent.c_str(),
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 10, 460, 400,
            hWnd,
            (HMENU)101,
            hInst,
            NULL);

        if (!hEdit)
        {
            MessageBox(hWnd, _T("Failed to create edit box."), _T("Error"), MB_OK);
        }
        else {
            SetWindowLongPtr(hEdit, GWLP_USERDATA, GetWindowLongPtr(hEdit, GWLP_WNDPROC));
            SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            hFont = CreateFont(
                fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            backgroundImage = new Image(globalImagePath);
        }

        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics graphics(hdc);

        graphics.DrawImage(backgroundImage, 0, 0, 500, 500);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_CLOSE:
    case WM_DESTROY:
    {
        if (hFont)
        {
            DeleteObject(hFont);
        }
        if (background)
        {
            delete background;
            background = nullptr;
        }
        DestroyWindow(hWnd);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void CreateFileProcessingForm(HWND hWnd) {
    static bool classRegistered = false;

    if (!classRegistered) {
        WNDCLASSEX wcex;
        ZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = FileProcessing;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInst;
        wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = _T("FileProcessing");
        wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        if (!RegisterClassEx(&wcex)) {
            MessageBox(NULL, _T("Failed to register new form class!"), _T("Error"), MB_OK);
            return;
        }
        classRegistered = true;
    }

    HWND hNewForm = CreateWindowEx(
        0,
        _T("FileProcessing"),
        _T("File Processing Form"),
        WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) - 500) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 500) / 2,
        500, 500,
        NULL,
        NULL,
        hInst,
        NULL);

    if (!hNewForm) {
        MessageBox(hWnd, _T("Failed to create new form!"), _T("Error"), MB_OK);
        return;
    }

    HICON hIcon = (HICON)LoadImage(
        NULL,
        globalIconPath,
        IMAGE_ICON,
        0, 0,
        LR_LOADFROMFILE
    );

    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();

    AppendMenu(hMenu, MF_STRING, ID_SAVE_FILE, _T("Save File"));
    AppendMenu(hSubMenu, MF_STRING, ID_FONT_ARIAL, _T("Arial"));
    AppendMenu(hSubMenu, MF_STRING, ID_FONT_TIMES, _T("Times New Roman"));
    AppendMenu(hSubMenu, MF_STRING, ID_FONT_VERDANA, _T("Verdana"));
    AppendMenu(hSubMenu, MF_STRING, ID_FONT_CALIBRI, _T("Calibri"));

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, _T("Fonts"));
    CheckMenuItem(hSubMenu, ID_FONT_ARIAL, MF_CHECKED);

    SetMenu(hNewForm, hMenu);

    if (hIcon)
    {
        SendMessage(hNewForm, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hNewForm, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }
    else
    {
        MessageBox(hWnd, _T("Failed to load icon."), _T("Error"), MB_OK);
    }

    RECT clientRect;
    GetClientRect(hNewForm, &clientRect);

    hFontSizeInfo = CreateWindowEx(
        0,
        _T("STATIC"),
        _T("Font Size: 16"),
        WS_CHILD | WS_VISIBLE,
        10, clientRect.bottom - 30,
        90, 20,
        hNewForm,
        NULL,
        hInst,
        NULL);

    if (!hFontSizeInfo) {
        MessageBox(hWnd, _T("Failed to create label."), _T("Error"), MB_OK);
    }

    ShowWindow(hNewForm, SW_SHOW);
    UpdateWindow(hNewForm);
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    GetCurrentDirectory(MAX_PATH, globalImagePath);
    GetCurrentDirectory(MAX_PATH, globalIconPath);

    wcscat_s(globalImagePath, MAX_PATH, L"\\backimage.png");
    wcscat_s(globalIconPath, MAX_PATH, L"\\mainIcon.ico");

    backgroundImage = new Image(globalImagePath);

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, _T("RegisterClassEx failed!"), _T("Error"), MB_OK);
        return 1;
    }

    HWND hWnd = CreateWindowEx(
        0, szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) - 500) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 500) / 2,
        500, 500, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        MessageBox(NULL, _T("CreateWindow failed!"), _T("Error"), MB_OK);
        return 1;
    }

    HICON hIcon = (HICON)LoadImage(
        NULL,
        globalIconPath,
        IMAGE_ICON,
        0, 0,
        LR_LOADFROMFILE
    );

    if (hIcon)
    {
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }
    else
    {
        MessageBox(hWnd, _T("Failed to load icon."), _T("Error"), MB_OK);
    }

    CreateAppMenu(hWnd);

    DragAcceptFiles(hWnd, TRUE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete backgroundImage;
    GdiplusShutdown(gdiplusToken);

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT rect;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);
        if (backgroundImage) {
            graphics.DrawImage(backgroundImage, 0, 0, 500, 500);
        }

        SetBkMode(hdc, TRANSPARENT);

        const TCHAR* text = _T("Drop your files");
        GetClientRect(hWnd, &rect);

        DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_CREATE_FILE:
        {
            CreateFileProcessingForm(hWnd);
            break;
        }
        case ID_OPEN_FILE:
        {
            OPENFILENAME ofn;
            TCHAR szFile[MAX_PATH] = _T("");

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(TCHAR);
            ofn.lpstrFilter = _T("All Files\0*.*\0");
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn))
            {
                ReadFileContent(szFile);
                CreateFileProcessingForm(hWnd);
            }
            else
            {
                MessageBox(hWnd, _T("File open cancelled."), _T("Info"), MB_OK);
            }
            break;
        }
        case ID_ABOUT:
        {
            MessageBox(hWnd, _T("Coursework Application\nDeveloped by Dmitry\nEmail: dmytro.datsko@nure.ua"), _T("About"), MB_OK | MB_ICONINFORMATION);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_DROPFILES:
    {
        TCHAR fileName[MAX_PATH];
        HDROP hDrop = (HDROP)wParam;

        if (DragQueryFile(hDrop, 0, fileName, MAX_PATH))
        {
            ReadFileContent(fileName);
            CreateFileProcessingForm(hWnd);
        }

        DragFinish(hDrop);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
