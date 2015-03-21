#include <string.h>

#include <config/config.h>

static struct _key {
	struct _key* next;
	char* key;
	char* value;
};

static struct _section {
	struct _section* next;
	struct _key* keys;
};

static struct _section* config_file;

void config_init() {

}

int config_get_string(char* section, char* key, char* buf, int bufsize)
{
	return 0;
}

int config_set_string(char* section, char* key, char* buf, int bufsize)
{
	return 0;
}

int config_get_float(char* section, char* key, float* val)
{
	return 0;
}

int config_set_float(char* section, char* key, float* val)
{
	return 0;
}

int config_get_int(char* section, char* key, int* val)
{
	return 0;
}

int config_set_int(char* section, char* key, int* val)
{
	return 0;
}

int config_remove_key(char* section, char* key)
{
	return 0;
}

void config_save()
{

}