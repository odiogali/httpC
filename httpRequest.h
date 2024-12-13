#ifndef httpRequest_h
#define httpRequest_h

enum HTTPMethod {
  GET = 0,
  HEAD, 
  POST,
  PUT,
  DELETE,
};

typedef struct HTTPHeader {
  char* name;
  char* value;
  struct HTTPHeader* next;
} HTTPHeader;

typedef struct HTTPRequest {
  int method;
  char* URI;
  float version;
  struct HTTPHeader* headers;
  char* body; // Body can be null - so check
} HTTPRequest;

int request_constructor(char *request_string, HTTPRequest* request);

#endif
