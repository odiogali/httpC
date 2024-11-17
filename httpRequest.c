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
  } else if(strcmp(method_string, "DELETE") == 0){
    return DELETE;
  } else {
    return HEAD;
  }
}

HTTPRequest request_constructor(char *request_string){
  HTTPRequest r;

  // printf("BEFORE: Request string is:\n%s\n\n", request_string);

  // Seperate request body and headers with |
  for (int i = 0; i < strlen(request_string) - 3; i++){
    if (request_string[i] == '\r' && request_string[i + 1] == '\n' && 
      request_string[i + 2] == '\r' && request_string[i + 3] == '\n'){
      request_string[i] = '|';
      request_string[i+1] = '|';
      request_string[i+2] = '|';
      request_string[i+3] = '|';
    }
  }
  
  // printf("AFTER: Request string is:\n%s\n\n", request_string);
  
  // Now we know where to find the request line of the http request (first line - ends at \r\n)
  // We know where to find the header (Everything right after the request line but before '||||')
  // And the request body is everything after the headers

  char* request_line = strtok(request_string, "\n");
  char* header_fields = strtok(NULL, "||||");
  printf("Header fields: %s\n", header_fields);
  char* request_body = strtok(NULL, "||||");
  printf("Request body: %s\n", request_body);

  char* request_method = strtok(request_line, " ");
  r.method = method_constructor(request_method);
  r.URI = strtok(NULL, " ");

  char* httpVersion = strtok(NULL, "\0");
  strtok(httpVersion, "/");
  httpVersion = strtok(NULL, "/");
  r.version = atof(httpVersion);

  return r;
}
