#include "httpRequest.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int method_constructor(char* method_string){
  if (strcmp(method_string, "GET") == 0){
    return GET;
  } else if(strcmp(method_string, "POST") == 0){
    return POST;
  } else if(strcmp(method_string, "PUT") == 0){
    return PUT;
  } else if(strcmp(method_string, "HEAD") == 0){
    return HEAD;
  } else if(strcmp(method_string, "PATCH") == 0){
    return PATCH;
  } else if(strcmp(method_string, "DELETE") == 0){
    return DELETE;
  } else if(strcmp(method_string, "CONNECT") == 0){
    return CONNECT;
  } else if(strcmp(method_string, "OPTIONS") == 0){
    return OPTIONS;
  } else { // TRACE
    return TRACE;
  }
}

HTTPRequest request_constructor(char *request_string){
  HTTPRequest r;

  // Seperate request body and headers with |
  for (int i = 0; i < strlen(request_string) - 1; i++){
    if (request_string[i] == '\n' && request_string[i + 1] == '\n'){
      request_string[i + 1] = '|';
    }
  }
  // Now we know where to find the request line of the http request (first line)
  // We know where to find the header (Everything right after the request line but before '|')
  // And the request body is everything after the header

  char* request_line = strtok(request_string, "\n");
  char* header_fields = strtok(NULL, "|");
  char* request_body = strtok(NULL, "|");

  char* request_method = strtok(request_line, " ");
  r.method = method_constructor(request_method);
  r.URI = strtok(NULL, " ");

  char* httpVersion = strtok(NULL, "\0");
  strtok(httpVersion, "/");
  httpVersion = strtok(NULL, "/");
  r.version = atof(httpVersion);

  printf("The parsed request looks like this: HTTP Request Method: %s, URI: %s, HTTP/%.1f\n", request_method, r.URI, r.version);

  return r;
}
