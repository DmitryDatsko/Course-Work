#define ID_CREATE_FILE 1
#define ID_OPEN_FILE 2
#define ID_ABOUT 3
#define ID_SAVE_FILE 4

#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>

using namespace Gdiplus;

#pragma comment (lib, "Gdiplus.lib")

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

LRESULT CALLBACK FileProcessing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static Image* background = nullptr;
    HWND hMainWindow = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message)
    {
    case ID_SAVE_FILE:

        break;
    case WM_CREATE:
    {
        background = new Image(L"C:\\Users\\ddazk\\Downloads\\backimage.png");
        break;
    }
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
        if (backgroundImage) {
            delete backgroundImage;
            backgroundImage = nullptr;
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
            ofn.lpstrFilter = _T("INF Files\0*.inf\0All Files\0*.*\0");
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn))
            {
                MessageBox(hWnd, ofn.lpstrFile, _T("Selected File"), MB_OK);
            }
            else
            {
                MessageBox(hWnd, _T("No file selected."), _T("Open File"), MB_OK);
            }

            break;
        }
        case ID_ABOUT:
            MessageBox(hWnd, _T("Author: Datsko Dmytro\nEmail: dmytro.datsko@nure.ua"), _T("Menu"), MB_OK);
            break;
        }
        break;
    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;

        UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

        for (UINT i = 0; i < fileCount; i++)
        {
            TCHAR filePath[MAX_PATH];

            DragQueryFile(hDrop, i, filePath, MAX_PATH);

            const TCHAR* ext = _tcsrchr(filePath, _T('.'));
            if (ext && _tcsicmp(ext, _T(".inf")) == 0)
            {
                MessageBox(hWnd, filePath, _T("Valid INF File"), MB_OK);
            }
            else
            {
                MessageBox(hWnd, _T("Invalid file type. Only .inf files are allowed."), _T("Error"), MB_OK);
            }
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