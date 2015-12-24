/*
    parser.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <json/parser.h>
#include <json/serializer.h>
#include <json.h>
#include <halosuit/halosuit.h>
#include <halosuit/stateofcharge.h>
//#include <testcode/automationtestdata.h>
#include <halosuit/automation.h>
#include <halosuit/logger.h>
#include <config/config.h>

void parser_parse(char* json_text)
{
    json_value* object = NULL;
    int length = 0;
    
    logger_log(json_text);

    object = json_parse(json_text, strlen(json_text));
    
    if (object == NULL) {
		logger_log("ERROR: MESSAGE NOT JSON\n");
		return;
    }

    length = object->u.object.length;
    
    // in case of incorrect json
    for (int i = 0; i < length; i++) {
        if (strcmp (object->u.object.values[i].name,"lights") == 0) {
            if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
            // turn on lights
                if (halosuit_relay_switch(LIGHTS, HIGH) != 0) {
                    logger_log("ERROR: LIGHTS RELAY FAILURE\n");
                }
                if (halosuit_relay_switch(LIGHTS_AUTO, LOW) != 0) {
                    logger_log("ERROR: LIGHTS_AUTO RELAY FAILURE\n");
                }
            }
            else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
                // turn off lights
                if (halosuit_relay_switch(LIGHTS, LOW) != 0) {
                    logger_log("ERROR: LIGHTS RELAY FAILURE\n");
                }
                if (halosuit_relay_switch(LIGHTS_AUTO, LOW) != 0) {
                    logger_log("ERROR: LIGHTS_AUTO RELAY FAILURE\n");
                }
            }
            else if (strcmp(object->u.object.values[i].value->u.string.ptr, "auto") == 0) {
                // turn on ambient light sensor
                if (halosuit_relay_switch(LIGHTS_AUTO, HIGH) != 0) {
                    logger_log("ERROR: LIGHTS_AUTO RELAY FAILURE\n");
                }
                if (halosuit_relay_switch(LIGHTS, LOW) != 0) {
                    logger_log("ERROR: LIGHTS RELAY FAILURE\n");
                }
            }
        }	
        else if (strcmp (object->u.object.values[i].name,"head lights red") == 0) {
            if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
                // turn on red head lights
                if (halosuit_relay_switch(HEADLIGHTS_RED, HIGH) != 0) {
                    logger_log("ERROR: HEADLIGHTS_RED RELAY FAILURE\n");
                }
            }
            else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
                // turn off red head lights
                if (halosuit_relay_switch(HEADLIGHTS_RED, LOW) != 0) {
                    logger_log("ERROR: HEADLIGHTS_RED RELAY FAILURE\n");
                }
            }
        }
        else if (strcmp (object->u.object.values[i].name,"head lights white") == 0) {
            if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
                // turn on white head lights
                if (halosuit_relay_switch(HEADLIGHTS_WHITE, HIGH) != 0) {
                    logger_log("ERROR: HEADLIGHTS_WHITE RELAY FAILURE\n"); }
                }
                else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
                // turn off white head lights
                if (halosuit_relay_switch(HEADLIGHTS_WHITE, LOW) != 0) {
                    logger_log("ERROR: HEADLIGHTS_WHITE RELAY FAILURE\n");
                }
            }
        }
        else if (strcmp (object->u.object.values[i].name,"head fans") == 0) {
            if (strcmp(object->u.object.values[i].value->u.string.ptr, "on") == 0) {
                // turn on head fans
                if (halosuit_relay_switch(HEAD_FANS, HIGH) != 0) {
                    logger_log("ERROR: HEAD_FANS RELAY FAILURE\n");
                }
            }
            else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
                // turn off head fans
                if (halosuit_relay_switch(HEAD_FANS, LOW) != 0) {
                    logger_log("ERROR: HEAD_FANS RELAY FAILURE\n");
                }
            }
        }
        else if (strcmp (object->u.object.values[i].name,"water pump") == 0) {
            if (strcmp(object->u.object.values[i].value->u.string.ptr, "auto") == 0) {
                automation_pump_auto();
            }
            else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
                automation_pump_off();
            }
        }
        else if (strcmp(object->u.object.values[i].name, "peltier") == 0) {
            if (strcmp(object->u.object.values[i].value->u.string.ptr, "auto") == 0) {
                automation_peltier_auto();
            }
            else if (strcmp(object->u.object.values[i].value->u.string.ptr, "off") == 0) {
                automation_peltier_off();
            }
        }

        else if (strcmp (object->u.object.values[i].name, "hud battery") == 0) {
            soc_setcharge(GLASS_BATTERY, object->u.object.values[i].value->u.integer);
        }

        else if (strcmp (object->u.object.values[i].name, "phone battery") == 0) {
            soc_setcharge(PHONE_BATTERY, object->u.object.values[i].value->u.integer);
        }

        else if (strcmp(object->u.object.values[i].name, "play sound") == 0) {
            serializer_save_sound(object->u.object.values[i].value->u.string.ptr);
        }

        else if (strcmp(object->u.object.values[i].name, "configuration") == 0) {
            json_value* config = object->u.object.values[i].value;
            for (int j = 0; j < config->u.object.length; j++) {
                char* address = config->u.object.values[j].value->u.string.ptr;
                if (strcmp(config->u.object.values[j].name, "android") == 0) {
                    if (strcmp(address, "delete") == 0) {
                        config_remove_key("Bluetooth", "android");
                    } else {
                        config_set_string("Bluetooth", "android", address);
                    }
                } else if (strcmp(config->u.object.values[j].name, "glass") == 0) {
                    if (strcmp(address, "delete") == 0) {
                        config_remove_key("Bluetooth", "glass");
                    } else {
                        config_set_string("Bluetooth", "glass", address);
                    }
                }
            }
            config_save();
        }
    } 
    
   json_value_free(object); 
}
