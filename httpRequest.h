#ifndef httpRequest_h
#define httpRequest_h

enum HTTPMethod {
  GET = 0,
  HEAD, 
  POST,
  PUT,
  PATCH,
  DELETE,
  CONNECT, 
  OPTIONS,
  TRACE
};

typedef struct HTTPRequest {
  int method;
  char* URI;
  float version;
} HTTPRequest;

HTTPRequest request_constructor(char *request_string);

#endif
