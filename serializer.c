/*
    serializer.c
*/

#include <stdlib.h>

#include <serializer.h>
#include <json-h>
#include <json-bulider.h>
#include <beagleblue.h>


void Serializer_serialize()
{ 
    while(1) {
	json_value *object = json_object_new(0);

	// push values into object

	char *buf = malloc(json_measure(object));
	json_serializer(buf, object);

	if (beagleblue_glass_send(buf) < strlen(buf)) {
	    printf("MESSAGE TO GLASS INCOMPLETE");
	}
	if (beagleblue_android_send(buf) < strlen(buf)) {
	    printf("MESSAGE TO ANDROID INCOMPLETE");
	}	

	free(buf);
	json_builder_free(object);
	
	sleep(STATUS_FREQUENCY);
    }

    return NULL;
}
