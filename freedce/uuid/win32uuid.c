#include <windows.h>
#include <rpcdce.h>

unsigned int win32_uuid_create(uuid_t *uuid)
{
	return UuidCreate(uuid);
}

