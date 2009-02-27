#include <dcom.h>

/* Initialize DCOM */
HRESULT CoInitialize(LPVOID pvReserved)
{
	return CoInitializeEx(pvReserved, 0);
}

HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit)
{
	
	return S_OK;
}

/* Shutdown DCOM things */
void  CoUninitialize(void)
{

}

DWORD CoBuildVersion(void)
{
	return 0x00010001;
}

HRESULT CoGetCurrentProcess(void)
{
	return getpid();
}

HRESULT  CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved,
                    REFIID riid, LPVOID FAR* ppv)
{
	return S_FALSE;
}

