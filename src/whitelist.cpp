#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include "jsmn/jsmn.h"
#include "json_ext.h"
#include "errors.h"
#include "whitelist.h"


static const char * IDCP_KEY_CMDS = "cmds";
static jsmn_parser whitelist_p;
static jsmntok_t t[32];  // We have a simple JSON, do not expect more that 4 tokens.
static int r = 0;
static char *source = NULL;

int whitelist_load_whitelist(char ** source, long * len);
int whitelist_create_parser(char * source, long len);

/**
 Load and parses the withelist.json configuration file. Keeps a parser on memeory for further requests for command validations.
 
 @return 0 if success.
 */
int whitelist_init( )
{
    int err = IDCP_SUCCESS;
    
    /* Load the whitelist.json content */
    long len;
    err = whitelist_load_whitelist(&source, &len);
    if (err) {
        return err;
    }
    
    /* Create parser with whitelist */
    err = whitelist_create_parser(source, len);
    
	return err;
}

/**
 Dispose the whitelist parser from memory.
 
 @return 0 Success.
 */
int whitelist_terminate( )
{
    if (source) {
        free(source);
        source = NULL;
    }
    return IDCP_SUCCESS;
}

/**
 Load the content of whitelist.json file into memory.
 
 @param content Will contain the content of the file.
 @param len Will containthe length of the content.
 @return 0 Success.
 */
int whitelist_load_whitelist(char ** content, long * len)
{
    int err = IDCP_SUCCESS;
    size_t newLen;
    long bufsize;
    
    FILE * fp = fopen("whitelist.json", "r");
    if (!fp) {
        return IDCP_WHITELIST_MISSING;
    }
    
    if (fseek(fp, 0L, SEEK_END) != 0) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
        goto CLEANUP;
    }
    
    /* Get the size of the file. */
    bufsize = ftell(fp);
    if (bufsize == -1) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
        goto CLEANUP;
    }
    
    /* Allocate our buffer to that size. */
    *content = (char*)malloc(sizeof(char) * (bufsize + 1));
    *len = bufsize;
    
    /* Go back to the start of the file. */
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
        goto CLEANUP;
    }
    
    /* Read the entire file into memory. */
    newLen = fread(*content, sizeof(char), bufsize, fp);
    if ( ferror( fp ) != 0 ) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
    } else {
        (*content)[newLen++] = '\0'; /* Just to be safe. */
    }
    
CLEANUP:
    
    if (fclose(fp)) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
    }
    return err;
}

/**
 Parses a withelist JSON to validate and load the JSMIN token for later use.
 
 @param source The whitelist content as a string.
 @param len The length of the string.
 @return 0 Success.
 */
int whitelist_create_parser(char * source, long len)
{
    int err = IDCP_SUCCESS;
    int i = 1;
    
    jsmn_init(&whitelist_p);
    r = jsmn_parse(&whitelist_p, source, len, t, sizeof(t)/sizeof(t[0]));
    if (r < 0) {
        return IDCP_ERROR_PARSING_WHITELIST ;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        return IDCP_ERROR_PARSING_WHITELIST;
    }
    
    /* Validate the format/syntax of whitelist.
     {"cmds" : ["cmd1",...]}
     */
    for (i = 1; i < r; i++) {
        if (jsoneq(source, &t[i], IDCP_KEY_CMDS) == 0) {
            int j;
            if (t[i+1].type != JSMN_ARRAY) {
                err = IDCP_ERROR_PARSING_WHITELIST; // We expect cmds to be an array of strings
                break;
            }
            for (j = 0; j < t[i+1].size; j++) {
                jsmntok_t *g = &t[i+j+2];
                if (g->type != JSMN_STRING) {
                    err = IDCP_ERROR_PARSING_WHITELIST;
                    break;
                }
            }
            i += t[i+1].size + 1;
        } else {
            err = IDCP_ERROR_PARSING_WHITELIST;
            break;
        }
    }
    
    return err;
}

/**
 Validates if the given command is in the whitelist.
 
 @param cmd The command to validate.
 @return 0 is Success.
 */
int whitelist_validate_cmd(const char * cmd)
{
    int i = 1;
    int j;
    int err = IDCP_FAILURE;
    
    if (r == 0 || i >= r) {
        return IDCP_WHITELIST_PARSER_NOT_INITIALISED;
    }
    
    /* Loop over all commands to find match */
    if (jsoneq(source, &t[i], IDCP_KEY_CMDS) != 0) {
        return err;
    }
    if (t[i+1].type != JSMN_ARRAY) {
        return err;
    }
    for (j = 0; j < t[i+1].size; j++) {
        jsmntok_t *g = &t[i+j+2];
        if (jsoneq(source, g, cmd) == 0) {
            err = IDCP_SUCCESS;
            break;
        }
    }
    
    return err;
}




