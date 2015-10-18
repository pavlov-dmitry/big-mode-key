#include "config.h"
#include <stdio.h>

#define MAX_STR_LENTGH 256

int read_combo( const char* str, int *out_key_combo, unsigned int *out_key_combo_length_ptr )
{
    int result = 1;
    int vk = 0;
    char tail[ MAX_STR_LENTGH ];

    if ( MAX_KEY_COMBO_LENGTH <= *out_key_combo_length_ptr )
    {
        return 0;
    }

    if ( sscanf( str, "%d+%s", &vk, tail ) == 2 )
    {
        out_key_combo[ *out_key_combo_length_ptr ] = vk;
        *out_key_combo_length_ptr += 1;
        read_combo( tail, out_key_combo, out_key_combo_length_ptr );
    }
    else if ( sscanf( str, "%d", &vk ) == 1 )
    {
        out_key_combo[ *out_key_combo_length_ptr ] = vk;
        *out_key_combo_length_ptr += 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

int read_key_bind( const char* line, KeyBind *out_key_bind_ptr )
{
    int result = 0;
    char key_combo_str[ MAX_STR_LENTGH ];

    if ( sscanf( line, "%d=%s", &out_key_bind_ptr->vk_code, key_combo_str ) == 2 &&
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
