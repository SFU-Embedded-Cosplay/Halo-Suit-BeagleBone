/*
    parser.h
    used to deserialize a json message and perform desired action.
*/
#ifndef PARSER
#define PARSER

/**
	takes a json string and parses out the json values.  
	each value is then writes their values to the hardware through halosuit.c.
*/
void parser_parse(char* jsonText);

#endif
