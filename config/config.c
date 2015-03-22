#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <config/config.h>

static int changes_made;

struct _key {
	struct _key* next;
	char* key;
	char* value;
};

struct _section {
	struct _section* next;
	char* section;
	struct _key* keys;
};

static FILE* config_file;
static struct _section* config_tree;

static void parse_file() {
	int c;
	c = fgetc(config_file);
	while (!feof(config_file)) {
		if (c == ' ' || c == '\n') {
			//next non white space
		} else if (c == '#') {
			//parse out comments
			while (c != '\n') {
				c = fgetc(config_file);
				if (c == EOF) {
					return;
				}
			}
		} else if (c == '[') {
			//parse section
			char* section = (char*) malloc(MAX_BUF_SIZE);
			memset(section, 0, MAX_BUF_SIZE);
			c = fgetc(config_file);
			int i = 0;
			while (c != ']') {
				section[i++] = (char) c;
				c = fgetc(config_file);
			}
			//if config_tree is NULL set it
			if (config_tree == NULL) {
				config_tree = (struct _section*) malloc(sizeof(struct _section));
				config_tree->next = NULL;
				config_tree->keys = NULL;
				config_tree->section = section;
			} else {
				struct _section* new_section = (struct _section*) malloc(sizeof(struct _section));
				new_section->next = config_tree;
				new_section->keys = NULL;
				new_section->section = section;
				//set config_tree to the new section
				config_tree = new_section;
			}
		} else {
			if (config_tree != NULL) {
				char* key = (char*) malloc(MAX_BUF_SIZE);
				memset(key, 0, MAX_BUF_SIZE);
				int i = 0;
				while (c != '=' && c != ' ') {
					key[i++] = (char) c;
					c = fgetc(config_file);
				}

				while (c == '=' || c == ' ') {
					c = fgetc(config_file);
				}

				char* value = (char*) malloc(MAX_BUF_SIZE);
				memset(value, 0, MAX_BUF_SIZE);
				i = 0;
				while (c != ' ' && c != '\n' && c != '#') {
					value[i++] = (char) c;
					c = fgetc(config_file);
				}
				//allocate new key value pair
				struct _key* new_key = (struct _key*) malloc(sizeof(struct _key));
				new_key->next = NULL;
				new_key->key = key;
				new_key->value = value;
				
				if (config_tree->keys == NULL) {
					config_tree->keys = new_key;
				} else {
					new_key->next = config_tree->keys;
					config_tree->keys = new_key;
				}

				if (c == '#') {
					fputc(c, config_file);
				}
			}
		}
		c = fgetc(config_file);
	}
}

void config_init() {
	changes_made = 0;
	config_tree = NULL;
	config_file = fopen("beaglebone.conf", "r");
	if (config_file != NULL) {
		parse_file();
	}
}

void free_config_tree() {
	struct _section* curr_section = config_tree;
	struct _key* curr_key = curr_section->keys;

}

void config_exit() {
	if (changes_made > 0) {
		config_save();
	}
	fclose(config_file);
	free_config_tree();
}

static struct _section* config_get_section(struct _section* sections, char* section) {
	struct _section* curr_section = sections;
	while (curr_section != NULL) {
		if (strcmp(section, curr_section->section) == 0) {
			return curr_section;
		}
		curr_section = curr_section->next;
	}
	return NULL; 
}

static struct _key* config_get_key(struct _key* keys, char* key) {
	struct _key* curr_key = keys;
	while (curr_key != NULL) {
		if (strcmp(key, curr_key->key) == 0) {
			return curr_key;
		}
		curr_key = curr_key->next;
	}
	return NULL;
}

int config_get_string(char* section, char* key, char* buf, int bufsize)
{
	struct _section* s = config_get_section(config_tree, section);
	struct _key* k = config_get_key(s->keys, key);
	if (k->value != NULL) {
		strncpy(buf, k->value, bufsize);
		return 0;
	}
	return -1;
}

void config_set_string(char* section, char* key, char* val)
{
	//first check if the section
	struct _section* s = config_get_section(config_tree, section);
	if (s == NULL) {
		//make new section
		struct _section* new_section = (struct _section*) malloc(sizeof(struct _section));
		new_section->next = config_tree;
		config_tree = new_section;
		new_section->section = (char*) malloc(MAX_BUF_SIZE);
		strncpy(new_section->section, section, MAX_BUF_SIZE);
		s = new_section;
	}

	struct _key* k = config_get_key(s->keys, key);
	if (k == NULL) {
		//make new key
		struct _key* new_key = (struct _key*) malloc(sizeof(struct _key));
		new_key->next = s->keys;
		s->keys = new_key;
		new_key->key = (char*) malloc(MAX_BUF_SIZE);
		strncpy(new_key->key, key, MAX_BUF_SIZE);
		new_key->value = (char*) malloc(MAX_BUF_SIZE);
		strncpy(new_key->value, val, MAX_BUF_SIZE);
	}
}

int config_get_float(char* section, char* key, float* val)
{
	struct _section* s = config_get_section(config_tree, section);
	struct _key* k = config_get_key(s->keys, key);
	if (k->value != NULL) {
		if (sscanf(k->value, "%f", val) == 0) {
			return -1;
		}
		return 0;
	}
	return -1;
}

void config_set_float(char* section, char* key, float val)
{
	char* value = malloc(MAX_BUF_SIZE);
	snprintf(value, MAX_BUF_SIZE, "%f", val);
	config_set_string(section, key, value);
	free(value);
}

int config_get_int(char* section, char* key, int* val)
{
	struct _section* s = config_get_section(config_tree, section);
	struct _key* k = config_get_key(s->keys, key);
	if (k->value != NULL) {
		if (sscanf(k->value, "%d", val) == 0) {
			return -1;
		}
		return 0;
	}
	return -1;
	return 0;
}

void config_set_int(char* section, char* key, int val)
{
	char* value = malloc(MAX_BUF_SIZE);
	snprintf(value, MAX_BUF_SIZE, "%d", val);
	config_set_string(section, key, value);
	free(value);
}

int config_remove_key(char* section, char* key)
{
	return 0;
}

void config_save()
{

}

void config_print() {
	struct _section* print_section = config_tree;
	while (print_section != NULL) {
		printf("[%s]\n", print_section->section);
		struct _key* key = print_section->keys;
		while (key != NULL) {
			printf("%s=%s\n", key->key, key->value);
			key = key->next;
		}
		print_section = print_section->next;
	}
}