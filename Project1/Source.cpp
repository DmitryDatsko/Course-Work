#define ID_CREATE_FILE 1
#define ID_OPEN_FILE 2
#define ID_ABOUT 3
#define ID_SAVE_FILE 4

#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <fstream>
#include <string>
#include <vector>

using namespace Gdiplus;
using namespace std;

#pragma comment (lib, "Gdiplus.lib")

wstring fileContent;
HINSTANCE hInst;
ULONG_PTR gdiplusToken;
Image* backgroundImage = nullptr;

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[] = _T("Coursework");

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

            // Добавляем BOM для UTF-16
            const wchar_t bom = 0xFEFF;
            WriteFile(hFile, &bom, sizeof(bom), &bytesWritten, NULL);

            // Записываем основной текст
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

LRESULT CALLBACK FileProcessing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEdit;
    static Image* background = nullptr;

    switch (message)
    {
    case WM_CREATE:
    {
        background = new Image(L"C:\\Users\\ddazk\\Downloads\\backimage.png");

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
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_SAVE_FILE:
        {
            int textLength = GetWindowTextLength(hEdit);
            if (textLength > 0)
            {
                TCHAR* buffer = new TCHAR[textLength + 1];
                GetWindowText(hEdit, buffer, textLength + 1);

#ifdef UNICODE
                wstring str(buffer);
#else
                string str(buffer);
#endif
                delete[] buffer;

                ShowSaveFileDialog(hWnd, str);
                fileContent = str;

                if (background)
                {
                    delete background;
                    background = nullptr;
                }
                DestroyWindow(hWnd);
            }
            else
            {
                MessageBox(hWnd, _T("Text box is empty."), _T("Info"), MB_OK);
            }
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics graphics(hdc);

        if (background)
        {
            graphics.DrawImage(background, 0, 0, 500, 500);
        }

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_CLOSE:
    case WM_DESTROY:
    {
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
        _T("C:\\Users\\ddazk\\Downloads\\mainIcon.ico"),
        IMAGE_ICON,
        0, 0,
        LR_LOADFROMFILE
    );

    HMENU hMenu = CreateMenu();

    AppendMenu(hMenu, MF_STRING, ID_SAVE_FILE, _T("Save File"));

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

    backgroundImage = new Image(L"C:\\Users\\ddazk\\Downloads\\backimage.png");

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
        _T("C:\\Users\\ddazk\\Downloads\\mainIcon.ico"),
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
            MessageBox(hWnd, _T("Coursework Application\nDeveloped by Dmitry"), _T("About"), MB_OK | MB_ICONINFORMATION);
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
