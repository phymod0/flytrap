#ifndef REST_SERVER_CTX
#define REST_SERVER_CTX


typedef struct {
	char* docroot;
} ServerConfig;


typedef struct {
	ServerConfig config;
} ServerCtx;


#endif /* REST_SERVER_CTX */
