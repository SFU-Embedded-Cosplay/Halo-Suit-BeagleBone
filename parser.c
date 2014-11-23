/*
    parser.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <parser.h>
#include <json.h>

void Parser_parse(char* json_text)
{
    json_value* object = NULL;
    int length = 0;

    object = json_parse(json_text, strlen(json_text));
    length = object->u.object.length;
    for (int i = 0; i < length; i++) {
	if (strcmp (object->u.object.values[i].name,"lights")) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on")) {
		// turn on lights
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off")) {
		// turn off lights
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "auto")) {
		// turn on ambient light sensor
	    }
	}	
	else if (strcmp (object->u.object.values[i].name,"head lights red")) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on")) {
		// turn on red head lights
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off")) {
		// turn off red head lights
	    }
	}
	else if (strcmp (object->u.object.values[i].name,"head lights white")) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on")) {
		// turn on white head lights
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off")) {
		// turn off white head lights
	    }
	}
	else if (strcmp (object->u.object.values[i].name,"head fans")) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on")) {
		// turn on head fans
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off")) {
		// turn off head fans
	    }
	}
	else if (strcmp (object->u.object.values[i].name,"water pump")) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on")) {
		// turn on water pump
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off")) {
		// turn off water pump
	    }
	}
	else if (strcmp (object->u.object.values[i].name,"water fan")) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on")) {
		// turn on water cooling fan
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off")) {
		// turn off water cooling fan
	    } 
	}
	else if (strcmp (object->u.object.values[i].name,"peltier")) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on")) {
		// turn on peltier
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off")) {
		// turn off peltier
	    }
	}	
    } 
}
