#include <Windows.h>
#include <iostream>
#pragma comment( lib, "Winmm.lib" )

class c_time_tracker {
public:
    c_time_tracker( ) { refresh( ); }
    void refresh( ) {
        QueryPerformanceFrequency( &m_frequency );
        QueryPerformanceCounter( &m_starting_time );
    }

    float elapsed( ) {
        QueryPerformanceCounter( &m_ending_time );
        m_elapsed_ms.QuadPart = m_ending_time.QuadPart - m_starting_time.QuadPart;
        return m_elapsed_ms.QuadPart * 1000.f / m_frequency.QuadPart;
    }

private:
    LARGE_INTEGER m_starting_time{ 0 };
    LARGE_INTEGER m_ending_time{ 0 };
    LARGE_INTEGER m_frequency{ 0 };
    LARGE_INTEGER m_elapsed_ms{ 0 };
};

void send_left_control( bool key_down ) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.dwFlags = key_down ? KEYEVENTF_SCANCODE : KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
    input.ki.wScan = MapVirtualKey( VK_LCONTROL, 0 );
    SendInput( 1, &input, sizeof( INPUT ) );
}

void custom_sleep( float ms ) {
    c_time_tracker excess;

    // Sleep usually takes 1.5ms with "timeBeginPeriod" ( We play it safe )
    constexpr float expected_time{ 2.f };

    // Sleep while we are safe that we wont "oversleep"
    while ( excess.elapsed( ) < ms - expected_time )
        Sleep( 1 );

    // Hotloop the last bit to be accurate
    while ( excess.elapsed( ) < ms )
        Sleep( 0 );
}

int main()
{
    timeBeginPeriod( 1 );
    SetPriorityClass( GetCurrentProcess( ), REALTIME_PRIORITY_CLASS );
    SetConsoleTitleA( "DjumpBugger" );

    float TICK_64_MS{ 15.6f };
    int exit_key{ VK_END };
    int activation_key{ VK_XBUTTON1 };

    printf( "bind mwheeldown +jump;\n" );
    while ( !( GetAsyncKeyState( exit_key ) & 0x8000 ) ) {
        if ( GetAsyncKeyState( activation_key ) & 0x8000 ) {
            mouse_event( MOUSEEVENTF_WHEEL, 0, 0, DWORD( -WHEEL_DELTA ), 0 );
            // For flat surfaces 39 ticks is perfect
            // 38 ticks for mirage mid jump for example...
            custom_sleep( TICK_64_MS * 39.f ); 
            send_left_control( true );
            custom_sleep( TICK_64_MS * 8.f );
            send_left_control( false );
            mouse_event( MOUSEEVENTF_WHEEL, 0, 0, DWORD( -WHEEL_DELTA ), 0 );

            // Normal bhop after jumpbug
            while ( GetAsyncKeyState( activation_key ) & 0x8000 ) {
                mouse_event( MOUSEEVENTF_WHEEL, 0, 0, DWORD( -WHEEL_DELTA ), 0 );
                custom_sleep( TICK_64_MS * 2.f );
            }
        }
        else {
            Sleep( 1 );
        }
    }
    timeEndPeriod( 1 );
    return EXIT_SUCCESS;
}