/*
    serializer.h
    creates a json string that contains status information ans sends it via bluetooth
*/

#ifndef SERIALIZER
#define SERIALIZER

void serializer_serialize(char *buf);

void serializer_save_sound(char *buf);

#endif
