
void config_init();
int config_get_string(char* section, char* key, char* buf, int bufsize);
int config_set_string(char* section, char* key, char* buf, int bufsize);

int config_get_float(char* section, char* key, float* val);
int config_set_float(char* section, char* key, float* val);

int config_get_int(char* section, char* key, int* val);
int config_set_int(char* section, char* key, int* val);

int config_remove_key(char* section, char* key);
void config_save();