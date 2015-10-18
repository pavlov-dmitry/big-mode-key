#include <windows.h>
#include <stdio.h>

#include "config.h"

HHOOK hook_handle = 0;

LRESULT PREVENT_KEY_PRESS = 1;

BOOL is_space_down = FALSE;
DWORD space_pressed_time = 0;

const int DELAY_TIME_MS = 200;
const int LONG_THINK_TIMEOUT_MS = 500;

#define MAX_DELAYED_COUNT 20
KBDLLHOOKSTRUCT delayed_keys[ MAX_DELAYED_COUNT ];
int delayed_keys_count = 0;

void reset_delayed_keys();
void add_to_delayed_keys( PKBDLLHOOKSTRUCT key_info );
void resolve_delays_keys_as_normal();
void resolve_delays_keys_as_combo();
void resolve_as_combo( PKBDLLHOOKSTRUCT key );

void press_space( BOOL down );

void process_space_down( PKBDLLHOOKSTRUCT key_info );
void process_space_up( PKBDLLHOOKSTRUCT key_info );
BOOL process_not_space_down( PKBDLLHOOKSTRUCT key_info );

LRESULT CALLBACK hook( int nCode, WPARAM wParam, LPARAM lParam );

BOOL is_need_delay( DWORD time );
BOOL is_down( PKBDLLHOOKSTRUCT key_info );

#define MAX_KEY_BINDS 128
KeyBind key_binds[ MAX_KEY_BINDS ];
unsigned int key_binds_count;

#define MAX_KEYS_RING_COUNT 128
typedef struct {
    int vk_code;
    BOOL down;
} Key;
Key ignore_keys_ring[ MAX_KEYS_RING_COUNT ];
int read_ring_idx = 0;
int write_ring_idx = 1;

void push_to_ignore_keys( int vk_code, BOOL down );
BOOL is_ignore_key( PKBDLLHOOKSTRUCT key );
void pop_ignore_key();

int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    MSG msg;
    key_binds_count = 0;
    read_config( "./key_binds", key_binds, MAX_KEY_BINDS, &key_binds_count );

    hook_handle = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        (HOOKPROC)hook,
        hInstance,
        0
    );

    while( 1 )
    {
        PeekMessage( &msg, 0, 0, 0, PM_REMOVE );
        TranslateMessage( &msg );
        DispatchMessage( &msg );

        if ( is_space_down && delayed_keys_count && !is_need_delay( GetTickCount() ) )
        {
            resolve_delays_keys_as_combo();
        }

        Sleep( 10 );
    }

    UnhookWindowsHookEx( hook_handle );

    return 0;
}

LRESULT CALLBACK hook( int nCode, WPARAM wParam, LPARAM lParam )
{
    PKBDLLHOOKSTRUCT key_info = 0;
    char msg[256];

    if ( nCode < 0 )
    {
        return CallNextHookEx( hook_handle, nCode, wParam, lParam );
    }

    key_info = (PKBDLLHOOKSTRUCT)lParam;

    if ( is_ignore_key( key_info ) )
    {
        pop_ignore_key();
        return CallNextHookEx( hook_handle, nCode, wParam, lParam );
    }

    if ( key_info->vkCode == VK_SPACE )
    {
        if ( is_down( key_info ) )
        {
            process_space_down( key_info );
        }
        else
        {
            process_space_up( key_info );
        }
        return PREVENT_KEY_PRESS;
    }
    else if ( is_down( key_info ) && is_space_down )
    {
        if ( process_not_space_down( key_info ) )
        {
            return PREVENT_KEY_PRESS;
        }
    }

    return CallNextHookEx( hook_handle, nCode, wParam, lParam );
}

void process_space_down( PKBDLLHOOKSTRUCT key_info )
{
    if ( !is_space_down )
    {
        is_space_down = TRUE;
        space_pressed_time = key_info->time;
    }
}

void process_space_up( PKBDLLHOOKSTRUCT key_info )
{
    if ( 0 != delayed_keys_count )
    {
        resolve_delays_keys_as_normal();
    }
    else
    {
        if ( ( key_info->time - space_pressed_time ) < LONG_THINK_TIMEOUT_MS)
        {
            press_space( TRUE );
            press_space( FALSE );
        }
    }
    is_space_down = FALSE;
}

BOOL process_not_space_down( PKBDLLHOOKSTRUCT key_info )
{
    BOOL need_prevent = FALSE;
    if ( is_need_delay( key_info->time ) )
    {
        add_to_delayed_keys( key_info );
        need_prevent = TRUE;
    }
    else
    {
        if ( is_down( key_info ) )
        {
            resolve_as_combo( key_info );
        }
        need_prevent = TRUE;
    }
    return need_prevent;
}

BOOL is_down( PKBDLLHOOKSTRUCT key_info )
{
    return ( key_info->flags & LLKHF_UP ) == 0;
}

BOOL is_need_delay( DWORD time )
{
    return ( time - space_pressed_time ) < DELAY_TIME_MS;
}

void init_input( INPUT *input )
{
    input->type = INPUT_KEYBOARD;
    input->ki.time = 0;
    input->ki.dwFlags = 0;
    input->ki.dwExtraInfo = 0;
}

void set_down( INPUT *input, BOOL down )
{
    if ( !down )
    {
        input->ki.dwFlags |= KEYEVENTF_KEYUP;
    }
}

void key2input( KBDLLHOOKSTRUCT *key, INPUT *input)
{
    KEYBDINPUT *kbd = &input->ki;
    kbd->wVk = (WORD)key->vkCode;
    kbd->wScan = (WORD)key->scanCode;
    kbd->dwFlags = 0;
    if ( key->flags & LLKHF_EXTENDED )
    {
        kbd->dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }
    if ( key->flags & LLKHF_UP )
    {
        kbd->dwFlags |= KEYEVENTF_KEYUP;
    }
    kbd->dwExtraInfo = key->dwExtraInfo;
}

void press( INPUT* input )
{
    SendInput( 1, input, sizeof( INPUT ) );
}

void press_key( int vk_code, BOOL down )
{
    INPUT input;
    init_input( &input );
    input.ki.wVk = vk_code;
    input.ki.wScan = MapVirtualKey( vk_code, MAPVK_VK_TO_VSC );
    set_down( &input, down );
    push_to_ignore_keys( vk_code, down );
    press( &input );

}

void press_space( BOOL down )
{
    press_key( VK_SPACE, down );
}

void reset_delayed_keys()
{
    delayed_keys_count = 0;
}

void add_to_delayed_keys( PKBDLLHOOKSTRUCT key_info )
{
    if ( delayed_keys_count < MAX_DELAYED_COUNT )
    {
        delayed_keys[ delayed_keys_count ] = *key_info;
        delayed_keys_count += 1;
    }
}

void resolve_delays_keys()
{
    int i = 0;
    for ( i = 0; i < delayed_keys_count; i += 1 )
    {
        INPUT input;
        init_input( &input );
        key2input( &delayed_keys[ i ], &input );
        press( &input );
    }
    reset_delayed_keys();
}

void resolve_as_combo( PKBDLLHOOKSTRUCT key )
{
    int i, j;
    for ( i = 0; i < key_binds_count; ++i )
    {
        KeyBind *current = &key_binds[ i ];
        if ( key->vkCode == current->vk_code )
        {
            for ( j = 0; j < current->key_combo_length; ++j )
            {
                press_key( current->key_combo_vk[ j ], TRUE );
            }
            for ( j = current->key_combo_length - 1; 0 <= j; --j )
            {
                press_key( current->key_combo_vk[ j ], FALSE );
            }
            break;
        }
    }
}

void resolve_delays_keys_as_combo()
{
    int i = 0;
    for ( i = 0; i < delayed_keys_count; i += 1 )
    {
        if ( is_down( &delayed_keys[ i ] ) )
        {
            resolve_as_combo( &delayed_keys[ i ] );
        }
    }
    reset_delayed_keys();
}

void resolve_delays_keys_as_normal()
{
    press_space( TRUE );
    press_space( FALSE );
    resolve_delays_keys();
}

int inc_ring_idx( int idx )
{
    if ( idx + 1 == MAX_KEYS_RING_COUNT )
    {
        return 0;
    }
    return idx + 1;
}

void push_to_ignore_keys( int vk_code, BOOL down )
{
    if ( write_ring_idx != read_ring_idx )
    {
        ignore_keys_ring[ write_ring_idx ].vk_code = vk_code;
        ignore_keys_ring[ write_ring_idx ].down = down;
        write_ring_idx = inc_ring_idx( write_ring_idx );
    }
}

BOOL is_ignore_key( PKBDLLHOOKSTRUCT key )
{
    BOOL result = FALSE;
    int idx_to_read = inc_ring_idx( read_ring_idx );
    if ( idx_to_read != write_ring_idx )
    {
        Key *current = &ignore_keys_ring[ idx_to_read ];
        if ( current->vk_code == key->vkCode && current->down == is_down( key ) )
        {
            result = TRUE;
        }
    }
    return result;
}

void pop_ignore_key()
{
    int idx_to_read = inc_ring_idx( read_ring_idx );
    if ( idx_to_read != write_ring_idx )
    {
        read_ring_idx = idx_to_read;
    }
}
