
int rpc__namedpipe_open_cli( const char *srv_name, const char *pipe_name);
int rpc__namedpipe_create(const char *pipe_name);
int rpc__namedpipe_listen(int sock, int backlog);
int np_socket_error(void);

int rpc__namedpipe_close(int sock);

