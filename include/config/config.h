#define MAX_BUF_SIZE 20

#ifndef CONFIG_H
#define CONFIG_H


void config_init(char* file);
int config_get_string(char* section, char* key, char* buf, int bufsize);
void config_set_string(char* section, char* key, char* val);

int config_get_float(char* section, char* key, float* val);
void config_set_float(char* section, char* key, float val);

int config_get_int(char* section, char* key, int* val);
void config_set_int(char* section, char* key, int val);

int config_remove_key(char* section, char* key);
int config_remove_section(char* section);
//frees memory that config uses, saves current configuration 
//and closes file
void config_exit();

//premature saving without freeing everything
void config_save();

void config_print();

#endif