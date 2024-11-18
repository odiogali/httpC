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

void printHeadersList(struct HTTPHeader* header){
  struct HTTPHeader *current, *head;
  head = header;

  current = head;
  while (current != NULL){
    printf("Header name: '%s', header value: '%s'\n", current->name, current->value);
    current = current->next;
  }
}

void parseHeaders(HTTPRequest* request, char* header_string){
  int count = 0;
  for (int i = 0; i < strlen(header_string) - 1; i++){
    if (header_string[i] == '\r' && header_string[i + 1] == '\n'){
      count++;
    }
  }
  ++count;

  char* line = strtok(header_string, "\r\n");
  char* headers_split[count];

  int i = 0;
  while (line != NULL){
    headers_split[i++] = line;
    line = strtok(NULL, "\r\n");
  }

  struct HTTPHeader *head, *current;

  current = malloc(sizeof(struct HTTPHeader));

  i = 0;
  while (i < count){
    if (i == 0){
      head = current;
    }
    current->name = strtok(headers_split[i], ": ");    
    current->value = strtok(NULL, ": ");
    //printf("Current name: '%s', Current value: '%s'\n", current->name, current->value);

    if (++i != count){
      struct HTTPHeader* new_header = malloc(sizeof(struct HTTPHeader));
      current->next = new_header;
      current = current->next;
    }
  }

  request->headers = head;
  printHeadersList(head);
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
  
  // HTTP Request line is first line - ends at \r\n
  // HTTP headers is everything right after the request line but before ||||
  // HTTP request body is everything after the headers

  char* request_line = strtok(request_string, "\n");
  char* header_fields = strtok(NULL, "||||");
  char* request_body = strtok(NULL, "||||");

  char* request_method = strtok(request_line, " ");
  r.method = method_constructor(request_method);
  r.URI = strtok(NULL, " ");

  char* httpVersion = strtok(NULL, "\0");
  strtok(httpVersion, "/");
  httpVersion = strtok(NULL, "/");
  r.version = atof(httpVersion);

  parseHeaders(&r, header_fields);

  r.body = request_body;

  return r;
}
