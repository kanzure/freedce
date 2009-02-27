#include <windows.h>
#include <stdio.h>

/****************************************************************************
  open and listen to a pipe
****************************************************************************/
int rpc__namedpipe_create(const char *pipe_name)
{
	HANDLE hnd;
	char path[80];

	sprintf (path, "\\\\.\\pipe\\%s", pipe_name);

	hnd = CreateNamedPipe ( path,
	                          PIPE_ACCESS_DUPLEX, // read/write access
	                          PIPE_TYPE_MESSAGE | // message type pipe
	                          PIPE_READMODE_MESSAGE | // message-read mode
	                          PIPE_WAIT, // blocking mode
	                          PIPE_UNLIMITED_INSTANCES, // max. instances
	                          65535, // output buffer size
	                          65535, // input buffer size
	                          NMPWAIT_USE_DEFAULT_WAIT, // client time-out (ms)
	                          NULL); // no security attribute
	if (hnd == INVALID_HANDLE_VALUE)
		return -1;
	return (int)hnd;
}

int np_socket_error(void)
{
	return GetLastError();
}

int rpc__namedpipe_listen(int sock, int backlog __attribute__((__unused__)))
{
    HANDLE hnd = (HANDLE)sock;
    return ConnectNamedPipe(hnd, NULL) ? 0 : -1;
}

int rpc__namedpipe_open_cli( const char *srv_name, const char *pipe_name)
{
	HANDLE hnd;
	DWORD dwMode;
	char path[80];

	/* create pipe name \\.\pipe\PIPENAME */

	sprintf(path, "\\\\%s\\pipe\\%s", srv_name, pipe_name);

	while (1)
	{
		hnd = CreateFile(path, GENERIC_READ|GENERIC_WRITE,
				 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hnd != INVALID_HANDLE_VALUE)
			break;

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			/*DEBUG(0, ("Could not open pipe"));*/
			return -1;
		}


		if (!WaitNamedPipe(path, 20000))
		{
			/*DEBUG(0, ("Could not open pipe"));*/
			return -1;
		}
	}

	/* set message-mode */
	dwMode = PIPE_READMODE_MESSAGE; 
	if (!SetNamedPipeHandleState( 
		hnd,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL))    // don't set maximum time 
	{
		/*DEBUG(0,("SetNamedPipeHandleState failed"));*/
		CloseHandle(hnd);
		return -1;
	}

	return (int)hnd;
}

int rpc__namedpipe_close(int sock)
{
	CloseHandle((HANDLE)sock);
	return 0;
}

