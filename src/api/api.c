#include "api.h"
#include "constants.h"
#include "race_capture/printk.h"
#include "mod_string.h"

static jsmn_parser p;
static jsmntok_t json_tok[10];

const api_t apis[] = SYSTEM_APIS;

static void dispatch_api(Serial *serial, const char * apiMsgName, const jsmntok_t *apiPayload){

	const api_t * api = apis;

	while (api->cmd != NULL){
		if (strcmp(api->cmd, apiMsgName) == 0){
			api->func(serial, apiPayload );
			break;
		}
		api++;
	}
	if (NULL == api->cmd){
		//TODO: send error maybe?
	}
}

static void execute_api(Serial * serial, char * buffer, jsmntok_t *json){
	const jsmntok_t *root = &json[0];
	if (root->type == JSMN_OBJECT && root->size == 2){
		const jsmntok_t *apiMsgName = &json[1];
		const jsmntok_t *payload = &json[2];
		if (apiMsgName->type == JSMN_STRING){
			buffer[apiMsgName->end] = '\0';
			const char * name = (buffer + apiMsgName->start);
			dispatch_api(serial, name, payload);
		}
	}
}

void process_api(Serial *serial, char * buffer, size_t bufferSize){
	jsmn_init(&p);

	interactive_read_line(serial, buffer, bufferSize);
	int r = jsmn_parse(&p, buffer, json_tok, 10);
	if (r == JSMN_SUCCESS){
		execute_api(serial, buffer, json_tok);
	}
	else{
		pr_warning("API Error \r\n");
	}
}
