#include "api.h"

static jsmn_parser p;
static jsmntok_t json_tok[10];

void process_api(Serial *serial, char * buffer, size_t bufferSize){
	jsmn_init(&p);

	interactive_read_line(serial, buffer, bufferSize);
	int r = jsmn_parse(&p, buffer, json_tok, 10);
	execute_api(serial, json_tok);
}

void execute_api(Serial * serial, jsmntok_t *json){

}
