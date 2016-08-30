#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <config/config.h>

static int changes_made;
static char* file;

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

/*
	configuration files are in the format
	[section name1]
	key1=value1
	key2=value2
	[section name2]
	key1=value1
	key2=value2

	Ex. 
	[Bluetooth]
	android=11:22:33:44:55:6A
	[automation]
	temp_warn=28
	temp_max=32
*/

static void parse_file() {
	int c;
	c = fgetc(config_file);
	while (!feof(config_file)) {
		if (c == ' ' || c == '\n' || c == '\r') {
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

void config_init(char* f) {
	file = f;
	changes_made = 0;
	config_tree = NULL;
	config_file = fopen(file, "r");
	if (config_file != NULL) {
		parse_file();
		fclose(config_file);
	}
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
	if (s == NULL) {
		return -1;
	}
	struct _key* k = config_get_key(s->keys, key);
	if (k == NULL) {
		return -1;
	}
	
	if (k->value != NULL) {
		strncpy(buf, k->value, bufsize);
		return 0;
	}
	return -1;
}

void config_set_string(char* section, char* key, char* val)
{
	changes_made++;
	//first check if the section exists
	struct _section* s = config_get_section(config_tree, section);
	if (s == NULL) {
		//make new section
		struct _section* new_section = (struct _section*) malloc(sizeof(struct _section));
		new_section->next = config_tree;
		config_tree = new_section;
		new_section->section = (char*) malloc(MAX_BUF_SIZE);
		strncpy(new_section->section, section, MAX_BUF_SIZE);
		new_section->keys = NULL;
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
		k = new_key;
	}

	strncpy(k->value, val, MAX_BUF_SIZE);

	if (config_tree == NULL) {
		config_tree = s;	
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

static int section_remove_key(struct _section* section, char* key) {
	struct _key* k = section->keys;

	if (strcmp(k->key, key) == 0) {
		free(k->key);
		free(k->value);
		if (k->next == NULL) {
			free(k);
			section->keys = NULL;
		} else {
			section->keys = k->next;
			free(k);
		}
		return 0;
	}

	while (k->next != NULL) {
		struct _key* next_key = k->next;
		if (strcmp(next_key->key, key) == 0) {
			free(next_key->key);
			free(next_key->value);
			k->next = next_key->next;
			free(next_key);
			return 0;
		}
		k = k->next;
	}
	return -1;
}

int config_remove_key(char* section, char* key)
{
	struct _section* s = config_get_section(config_tree, section);
	if (s != NULL) {
		return section_remove_key(s, key);
	}
	return -1;
}

static void free_keys(struct _key* keys) {
	if (keys != NULL) {
		if (keys->next == NULL) {
			free(keys->key);
			free(keys->value);
			free(keys);
		} else {
			free_keys(keys->next);
			free(keys->key);
			free(keys->value);
			free(keys);
		}
	}
}

int config_remove_section(char* section) {
	if (strcmp(config_tree->section, section) == 0) {
		if (config_tree->next == NULL) {
			free(config_tree->section);
			//then free config_tree and set to NULL
			free_keys(config_tree->keys);
			config_tree = NULL;
			return 0;
		}
	}

	struct _section* s = config_tree;
	while (s->next != NULL) {
		struct _section* next = s->next;
		s->next = next->next;
		if (strcmp(next->section, section) == 0) {
			free(next->section);

			free_keys(next->keys);
			return 0;
		}
	}
	return -1;
}

static void free_sections(struct _section* sections) {
	if (sections != NULL) {
		if (sections->next == NULL) {
			free(sections->section);
			free_keys(sections->keys);
			free(sections);
		} else {
			free_sections(sections->next);
			free(sections->section);
			free_keys(sections->keys);
			free(sections);
		}
	}
}

void config_exit() {
	if (changes_made > 0) {
		config_save();
	}
	free_sections(config_tree);
	config_tree = NULL;
}

void config_save()
{
	config_file = fopen(file, "w+");

	struct _section* curr_section = config_tree;
	while (curr_section != NULL) {
		fprintf(config_file, "[%s]\n", curr_section->section);
		struct _key* key = curr_section->keys;
		while (key != NULL) {
			fprintf(config_file, "%s=%s\n", key->key, key->value);
			key = key->next;
		}
		fprintf(config_file, "\n");
		curr_section = curr_section->next;
	}

	fclose(config_file);
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