
#include "launcher.hpp"
#include "resource.h"

#include <windows.h>

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

static HWND window;
static HANDLE image;
static HFONT font;
static String launcherStatus;

static unsigned lastRefresh = 0;
static Mutex refreshMutex;

LRESULT CALLBACK wndProcedure( HWND window, UINT msg, WPARAM wParam, LPARAM lParam );

void accessDenied()
{
    ShowWindow( window, SW_HIDE );
    MessageBox( 0, "The updater failed to write to your home directory.\n\nPlease ensure there is enough space on your HDD.", window_title, MB_ICONERROR );
    exit( 0 );
}

void changeStatus( const char* status )
{
    launcherStatus = status;

    refreshMutex.enter();
    InvalidateRect( window, 0, FALSE );
    refreshMutex.leave();
}

void launcherOutdated()
{
    ShowWindow( window, SW_HIDE );
    MessageBox( 0, "Your game launcher is outdated!\n\nPlease grab a new one from\n" launcher_url, window_title, MB_ICONERROR );
    exit( 0 );
}

void refresh()
{
    if ( refreshMutex.enter( 0 ) )
    {
        if ( GetTickCount() > lastRefresh + 50 )
        {
            lastRefresh = GetTickCount();

            InvalidateRect( window, 0, FALSE );
        }

        refreshMutex.leave();
    }
}

void runGame()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

#ifdef install_openal_redist
{
    String detectFileName = base + "/oalinst_ok";

    Reference<File> isInstall = File::open( detectFileName );

    if ( !isInstall )
    {
        changeStatus( "Updating OpenAL" );

        String fileName = base + "/oalinst.exe";

        SHELLEXECUTEINFO sei;
        sei.cbSize = sizeof( sei );
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd = 0;
        sei.lpVerb = "open";
        sei.lpFile = fileName;
        sei.lpParameters = "-s";
        sei.lpDirectory = 0;
        sei.nShow = SW_SHOWNORMAL;
        sei.hProcess = 0;

        if ( ShellExecuteEx( &sei ) )
        {
            WaitForSingleObject( sei.hProcess, INFINITE );
            CloseHandle( sei.hProcess );
        }
        else
        {
            ShowWindow( window, SW_HIDE );
            MessageBox( 0, "Failed to update the OpenAL runtime.", window_title, MB_ICONERROR );
            exit( 0 );
        }

        isInstall = File::open( detectFileName, true );
    }
}
#endif

    GetStartupInfo( &si );

    if ( CreateProcess( base + "/" + exe_name, 0, 0, 0, FALSE, 0, 0, base, &si, &pi ) )
    {
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }
    else
    {
        ShowWindow( window, SW_HIDE );
        MessageBox( 0, "Failed to start the game.", window_title, MB_ICONERROR );
        exit( 0 );
    }

    if ( wait_timeout >= 0 )
    {
        if ( wait_timeout > 0 )
            Sleep( wait_timeout );

        SendMessage( window, WM_DESTROY, 0, 0 );
    }
}

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    WNDCLASSEX wndClass;
    MSG msg;

    wndClass.cbSize        = sizeof( wndClass );
    wndClass.style         = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wndClass.lpfnWndProc   = wndProcedure;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hInstance     = hInstance;
    wndClass.hIcon         = LoadIcon( hInstance, "A" );
    wndClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wndClass.hbrBackground = ( HBRUSH )GetStockObject( WHITE_BRUSH );
    wndClass.lpszMenuName  = 0;
    wndClass.lpszClassName = app_name;
    wndClass.hIconSm       = LoadIcon( hInstance, IDI_APPLICATION );
    RegisterClassEx( &wndClass );

    window = CreateWindowEx( 0,
                          app_name,
                          window_title,
                          WS_POPUP,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          image_width,
                          image_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL );

    changeStatus( "initializing" );

    image = LoadImage( hInstance, MAKEINTRESOURCE( IDB_SPLASH ), IMAGE_BITMAP, 0, 0, 0 );

    font = CreateFont( 16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FF_DONTCARE, font_name );

    SetWindowPos( window, HWND_TOPMOST, ( GetSystemMetrics( SM_CXSCREEN ) - image_width ) / 2, ( GetSystemMetrics( SM_CYSCREEN ) - image_height ) / 2,
            0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );

#ifdef __li_MSW
    String baseDir = ( String ) getenv( "APPDATA" ) + "/." package_name;
#else
    String baseDir = ( String ) getenv( "HOME" ) + "/." package_name;
#endif

    checkForUpdates( baseDir );

    while ( GetMessage( &msg, 0, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    DeleteObject( font );
    DeleteObject( image );

    return msg.wParam;
}

LRESULT CALLBACK wndProcedure( HWND window, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg )
    {
    	case WM_DESTROY:
    	    PostQuitMessage( WM_QUIT );
    	    break;

    	case WM_PAINT:
        {
            HDC hDC, memDC;
            PAINTSTRUCT ps;

            RECT textRect = { 0, image_height * 6 / 10, image_width - 1, image_height * 7 / 10 - 1 };

    	    hDC = BeginPaint( window, &ps );

            memDC = CreateCompatibleDC( hDC );
            SelectObject( memDC, image );

    	    BitBlt( hDC, 0, 0, image_width, image_height, memDC, 0, 0, SRCCOPY );

            SelectObject( hDC, font );

            SetBkMode( hDC, TRANSPARENT );
            SetTextColor( hDC, font_colour );

            DrawText( hDC, launcherStatus, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER );

            unsigned y = image_height * 14 / 20 + 5;

            transfersMutex.enter();

            iterate ( transfers )
            {
                RECT labelRect = { image_width / 20, y, image_width * 9 / 20, y + image_height / 20 - 1 };
                RECT progressFieldRect = { image_width * 11 / 20, y, image_width * 19 / 20, y + image_height / 20 - 1 };

                RECT progressBarRect = { progressFieldRect.left + 2, progressFieldRect.top + 2,
                        progressFieldRect.left + 2 + ( progressFieldRect.right - progressFieldRect.left - 4 ) * transfers.current()->progress / 100, progressFieldRect.bottom - 2 };

                DrawText( hDC, transfers.current()->fileName, -1, &labelRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER );
                Rectangle( hDC, progressFieldRect.left, progressFieldRect.top, progressFieldRect.right, progressFieldRect.bottom );
                FillRect( hDC, &progressBarRect, ( HBRUSH ) GetStockObject( BLACK_BRUSH ) );

                y += image_height / 20;
            }

            transfersMutex.leave();

    	    DeleteDC( memDC );
    	    EndPaint( window, &ps );
    	    break;
        }

    	default:
    	    return DefWindowProc( window, msg, wParam, lParam );
    }

    return 0;
}
