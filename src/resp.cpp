#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "resp.h"
#include "errors.h"


/**
 Utility to get the length of a stream.
 
 @param stream The stream.
 @return The length.
 */
static stringstream::pos_type stream_length(stringstream & stream)
{
    stream.seekg(0, ios::end);
    return stream.tellg();
}

/**
 Uility to add a comma separator to a stream.
 
 @param stream The stream.
 */
static void stream_add_separator(stringstream & stream)
{
    if (stream_length(stream)) {
        stream << ",";
    }
}

/**
 Builds a JSON message used as a response to the incoming command.
 
 @param status String with failure/success
 @param cmd The original command
 @param cmd_output The output generated by the command execution.
 @param err Error description.
 @param response Will contain the composed JSON.
 @return 0 if success.
 */
int resp_build(const char * status, const char * cmd,  const stringstream * cmd_output, const char * err, stringstream & response)
{
    response << "{";
    
    if (status) {
        response << "\"status\":" << "\"" << status << "\"";
    }
    if (cmd) {
        stream_add_separator(response);
        response << "\"cmd\":" << "\"" << cmd << "\"";
    }

    if (cmd_output) {
        stream_add_separator(response);
        response << "\"response\":" << "\"" << cmd_output->str() << "\"";
    }
    
    if (err) {
        stream_add_separator(response);
        response << "\"error\":" << "\"" << err << "\"";
    }

    response << "}";
    
    return IDCP_SUCCESS;
}






