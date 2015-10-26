#include "config.h"
#include <stdio.h>
#include <windows.h>

#define MAX_STR_LENTGH 256

typedef struct {
    const char* str;
    int vk_code;
} KeyAsString;

KeyAsString keys_map[] = {
    {"0", '0'},
    {"1", '1'},
    {"2", '2'},
    {"3", '3'},
    {"4", '4'},
    {"5", '5'},
    {"6", '6'},
    {"7", '7'},
    {"8", '8'},
    {"9", '9'},
    {"Q", 'Q'},
    {"q", 'Q'},
    {"W", 'W'},
    {"w", 'W'},
    {"E", 'E'},
    {"e", 'E'},
    {"R", 'R'},
    {"r", 'R'},
    {"T", 'T'},
    {"t", 'T'},
    {"Y", 'Y'},
    {"y", 'Y'},
    {"U", 'U'},
    {"u", 'U'},
    {"I", 'I'},
    {"i", 'I'},
    {"O", 'O'},
    {"o", 'O'},
    {"P", 'P'},
    {"p", 'P'},
    {"A", 'A'},
    {"a", 'A'},
    {"S", 'S'},
    {"s", 'S'},
    {"D", 'D'},
    {"d", 'D'},
    {"F", 'F'},
    {"f", 'F'},
    {"G", 'G'},
    {"g", 'G'},
    {"H", 'H'},
    {"h", 'H'},
    {"J", 'J'},
    {"j", 'J'},
    {"K", 'K'},
    {"k", 'K'},
    {"L", 'L'},
    {"l", 'L'},
    {"Z", 'Z'},
    {"z", 'Z'},
    {"X", 'X'},
    {"x", 'X'},
    {"C", 'C'},
    {"c", 'C'},
    {"V", 'V'},
    {"v", 'V'},
    {"B", 'B'},
    {"b", 'B'},
    {"N", 'N'},
    {"n", 'N'},
    {"M", 'M'},
    {"m", 'M'},
    {"[", 219},
    {"]", 221},
    {"`", 192},
    {";", 186},
    {"'", 222},
    {"\\", 220},
    {",", 188},
    {".", 190},
    {"/", 90},
    {"-", 189},
    {"Return", VK_RETURN},
    {"Enter", VK_RETURN},
    {"Tab", VK_TAB},
    {"Back", VK_BACK},
    {"Backspace", VK_BACK},
    {"Shift", VK_LSHIFT},
    {"Ctrl", VK_LCONTROL},
    {"Alt", VK_MENU},
    {"Capslock", VK_CAPITAL},
    {"Esc", VK_ESCAPE},
    {"Page Up", VK_PRIOR},
    {"Page Down", VK_NEXT},
    {"End", VK_END},
    {"Home", VK_HOME},
    {"Left", VK_LEFT},
    {"Right", VK_RIGHT},
    {"Up", VK_UP},
    {"Down", VK_DOWN},
    {"Insert", VK_INSERT},
    {"Ins", VK_INSERT},
    {"Delete", VK_DELETE},
    {"Del", VK_DELETE},
    {"F1", VK_F1},
    {"F2", VK_F2},
    {"F3", VK_F3},
    {"F4", VK_F4},
    {"F5", VK_F5},
    {"F6", VK_F6},
    {"F7", VK_F7},
    {"F8", VK_F8},
    {"F9", VK_F9},
    {"F10", VK_F10},
    {"F11", VK_F11},
    {"F12", VK_F12},
};

int to_vk( const char* str, int *out_code )
{
    int result = 0;
    int count = sizeof( keys_map ) / sizeof( KeyAsString );
    int i = 0;
    for ( i = 0; i < count; ++i )
    {
        if ( strcmp( keys_map[ i ].str, str ) == 0 )
        {
            *out_code = keys_map[ i ].vk_code;
            result = 1;
            break;
        }
    }
    if ( result == 0 && sscanf( str, "%d", out_code ) == 1 )
    {
        result = 1;
    }
    return result;
}

int split_str( const char* str, char *out_before, char* out_after, char delimeter )
{
    int result = 0;
    int length = strlen( str );
    int i = 0;
    for ( i = 0; i < length; ++i )
    {
        if ( str[ i ] == delimeter )
        {
            int after_length = length - i - 1;
            memcpy( out_before, str, i );
            out_before[ i ] = 0;
            memcpy( out_after, str + i + 1, after_length );
            out_after[ after_length ] = 0;
            result = 1;
            break;
        }
    }
    return result;
}

int read_combo( const char* str, int *out_key_combo, unsigned int *out_key_combo_length_ptr )
{
    int result = 0;
    int vk = 0;
    char tail[ MAX_STR_LENTGH ];
    char vk_str[ MAX_STR_LENTGH ];

    if ( MAX_KEY_COMBO_LENGTH <= *out_key_combo_length_ptr )
    {
        return 0;
    }

    if ( split_str( str, vk_str, tail, '+' ) )
    {
        if ( to_vk( vk_str, &vk ) )
        {
            out_key_combo[ *out_key_combo_length_ptr ] = vk;
            *out_key_combo_length_ptr += 1;
            result = read_combo( tail, out_key_combo, out_key_combo_length_ptr );
        }
    }
    else if ( to_vk( str, &vk ) )
    {
        out_key_combo[ *out_key_combo_length_ptr ] = vk;
        *out_key_combo_length_ptr += 1;
        result = 1;
    }
    return result;
}

int read_key_bind( const char* line, KeyBind *out_key_bind_ptr )
{
    int result = 0;
    char key_combo_str[ MAX_STR_LENTGH ];
    char vk_str[ MAX_STR_LENTGH ];

    if ( split_str( line, vk_str, key_combo_str, '=' ) &&
         to_vk( vk_str, &out_key_bind_ptr->vk_code ) &&
         read_combo( key_combo_str, out_key_bind_ptr->key_combo_vk, &out_key_bind_ptr->key_combo_length ) )
    {
        result = 1;
    }
    return result;
}

void read_config( const char* filename,
                  KeyBind *out_key_binds,
                  unsigned int max_key_binds,
                  unsigned int *out_key_binds_length_ptr )
{
    FILE *file = fopen( filename, "rt" );
    *out_key_binds_length_ptr = 0;
    if ( file )
    {
        while( !feof( file ) )
        {
            char line[ MAX_STR_LENTGH ];
            if ( fscanf( file, "%s\n", line ) == 1 )
            {
                if ( line[0] != '#' )
                {
                    KeyBind key_bind = {0};
                    if ( *out_key_binds_length_ptr < max_key_binds &&
                         read_key_bind( line, &key_bind ) )
                    {
                        out_key_binds[ *out_key_binds_length_ptr ] = key_bind;
                        *out_key_binds_length_ptr += 1;
                    }
                }
            }
        }
        fclose( file );
    }
}
