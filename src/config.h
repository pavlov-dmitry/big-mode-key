#ifndef CONFIG_H
#define CONFIG_H

#define MAX_KEY_COMBO_LENGTH 5

typedef struct {
    int vk_code;
    int key_combo_vk[ MAX_KEY_COMBO_LENGTH ];
    unsigned int key_combo_length;
} KeyBind;

void read_config( const char* filename,
                  KeyBind *out_key_binds,
                  unsigned int max_key_binds,
                  unsigned int *out_key_binds_length );

#endif /* CONFIG_H */
