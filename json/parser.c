/*
    parser.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <json/parser.h>
#include <json/json.h>
//#include <halosuit/halosuit.h>
#include <testcode/automationtestdata.h>

void parser_parse(char* json_text)
{
    json_value* object = NULL;
    int length = 0;
    
    printf("printing this string: %s\n", json_text); 

    object = json_parse(json_text, strlen(json_text));
    
    if (object == NULL) {
		printf("ERROR: MESSAGE NOT JSONi\n");
		return;
    }

    length = object->u.object.length;
    
    // in case of incorrect json

    int i = 0; // here until we get c99 working
    for (i = 0; i < length; i++) {
	if (strcmp (object->u.object.values[i].name,"lights") == 0) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
		// turn on lights
		if (halosuit_relay_switch(LIGHTS, HIGH) != 0) {
		    printf("ERROR: LIGHTS RELAY FAILURE\n");
		}
		if (halosuit_relay_switch(LIGHTS_AUTO, LOW) != 0) {
		    printf("ERROR: LIGHTS_AUTO RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
		// turn off lights
		if (halosuit_relay_switch(LIGHTS, LOW) != 0) {
		    printf("ERROR: LIGHTS RELAY FAILURE\n");
		}
		if (halosuit_relay_switch(LIGHTS_AUTO, LOW) != 0) {
		    printf("ERROR: LIGHTS_AUTO RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "auto") == 0) {
		// turn on ambient light sensor
		if (halosuit_relay_switch(LIGHTS_AUTO, HIGH) != 0) {
		    printf("ERROR: LIGHTS_AUTO RELAY FAILURE\n");
		}
		if (halosuit_relay_switch(LIGHTS, LOW) != 0) {
		    printf("ERROR: LIGHTS RELAY FAILURE\n");
		}
	    }
	}	
	else if (strcmp (object->u.object.values[i].name,"head lights red") == 0) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
		// turn on red head lights
		if (halosuit_relay_switch(HEADLIGHTS_RED, HIGH) != 0) {
		    printf("ERROR: HEADLIGHTS_RED RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
		// turn off red head lights
		if (halosuit_relay_switch(HEADLIGHTS_RED, LOW) != 0) {
		    printf("ERROR: HEADLIGHTS_RED RELAY FAILURE\n");
		}
	    }
	}
	else if (strcmp (object->u.object.values[i].name,"head lights white") == 0) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
		// turn on white head lights
		if (halosuit_relay_switch(HEADLIGHTS_WHITE, HIGH) != 0) {
		    printf("ERROR: HEADLIGHTS_WHITE RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
		// turn off white head lights
		if (halosuit_relay_switch(HEADLIGHTS_WHITE, LOW) != 0) {
		    printf("ERROR: HEADLIGHTS_WHITE RELAY FAILURE\n");
		}
	    }
	}
	else if (strcmp (object->u.object.values[i].name,"head fans") == 0) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
		// turn on head fans
		if (halosuit_relay_switch(HEAD_FANS, HIGH) != 0) {
		    printf("ERROR: HEAD_FANS RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
		// turn off head fans
		if (halosuit_relay_switch(HEAD_FANS, LOW) != 0) {
		    printf("ERROR: HEAD_FANS RELAY FAILURE\n");
		}
	    }
	}
	else if (strcmp (object->u.object.values[i].name,"water pump") == 0) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
		// turn on water pump
		if (halosuit_relay_switch(WATER_PUMP, HIGH) != 0) {
		    printf("ERROR: WATER_PUMP RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
		// turn off water pump
		if (halosuit_relay_switch(WATER_PUMP, LOW) != 0) {
		    printf("ERROR: WATER_PUMP RELAY FAILURE\n");
		}
	    }
	}
    /*
	else if (strcmp (object->u.object.values[i].name,"water fan") == 0) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
		// turn on water cooling fan
		if (halosuit_relay_switch(WATER_FAN, HIGH) != 0) {
		    printf("ERROR: WATER_FAN RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
		// turn off water cooling fan
		if (halosuit_relay_switch(WATER_FAN, LOW) != 0) {
		    printf("ERROR: WATER_FAN RELAY FAILURE\n");
		}
	    } 
	}
    */
	else if (strcmp (object->u.object.values[i].name,"peltier") == 0) {
	    if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
		// turn on peltier
		if (halosuit_relay_switch(PELTIER, HIGH) != 0) {
		    printf("ERROR: PELTIER RELAY FAILURE\n");
		}
	    }
	    else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
		// turn off peltier
		if (halosuit_relay_switch(PELTIER, LOW) != 0) {
		    printf("ERROR: PELTIER RELAY FAILURE\n");
		}
	    }
	}	
	else {
	    printf("Cannot parse key %d.\n",i);
	}
    } 
    
   json_value_free(object); 
}
