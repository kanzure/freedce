/* 
 * API Definitions for DCOM, part of the FreeDCE & DCOM package.
 * This file borrows from objbase.h, part of the Windows Includes,
 * Copyright Microsoft Corporation.
 * */

/* Macros for interface declaration.
  *      Example Interface declaration:
 *
 *          #undef  INTERFACE
 *          #define INTERFACE   IClassFactory
 *
 *          DECLARE_INTERFACE_(IClassFactory, IUnknown)
 *          {
 *              // *** IUnknown methods ***
 *              STDMETHOD(QueryInterface) (THIS_
 *                                        REFIID riid,
 *                                        LPVOID FAR* ppvObj) PURE;
 *              STDMETHOD_(ULONG,AddRef) (THIS) PURE;
 *              STDMETHOD_(ULONG,Release) (THIS) PURE;
 *
 *              // *** IClassFactory methods ***
 *              STDMETHOD(CreateInstance) (THIS_
 *                                        LPUNKNOWN pUnkOuter,
 *                                        REFIID riid,
 *                                        LPVOID FAR* ppvObject) PURE;
 *          };
 *
 *      Example C++ expansion:
 *
 *          struct FAR IClassFactory : public IUnknown
 *          {
 *              virtual HRESULT STDMETHODCALLTYPE QueryInterface(
 *                                                  IID FAR& riid,
 *                                                  LPVOID FAR* ppvObj) = 0;
 *              virtual HRESULT STDMETHODCALLTYPE AddRef(void) = 0;
 *              virtual HRESULT STDMETHODCALLTYPE Release(void) = 0;
 *              virtual HRESULT STDMETHODCALLTYPE CreateInstance(
 *                                              LPUNKNOWN pUnkOuter,
 *                                              IID FAR& riid,
 *                                              LPVOID FAR* ppvObject) = 0;
 *          };
 *
 *      Example C expansion:
 *
 *          typedef struct IClassFactory
 *          {
 *              const struct IClassFactoryVtbl FAR* lpVtbl;
 *          } IClassFactory;
 *
 *          typedef struct IClassFactoryVtbl IClassFactoryVtbl;
 *
 *          struct IClassFactoryVtbl
 *          {
 *              HRESULT (STDMETHODCALLTYPE * QueryInterface) (
 *                                                  IClassFactory FAR* This,
 *                                                  IID FAR* riid,
 *                                                  LPVOID FAR* ppvObj) ;
 *              HRESULT (STDMETHODCALLTYPE * AddRef) (IClassFactory FAR* This) ;
 *              HRESULT (STDMETHODCALLTYPE * Release) (IClassFactory FAR* This) ;
 *              HRESULT (STDMETHODCALLTYPE * CreateInstance) (
 *                                                  IClassFactory FAR* This,
 *                                                  LPUNKNOWN pUnkOuter,
 *                                                  IID FAR* riid,
 *                                                  LPVOID FAR* ppvObject);
 *              HRESULT (STDMETHODCALLTYPE * LockServer) (
 *                                                  IClassFactory FAR* This,
 *                                                  BOOL fLock);
 *          };
 */

#define STDMETHODCALLTYPE	/* nothing special */

#if defined(__cplusplus)
# define EXTERN_C	extern "C"
#else
# define EXTERN_C /* nothing special */
#endif

#define FAR	/* nothing special */
 
#if defined(__cplusplus) && !defined(CINTERFACE)
/* C++ macros */
# define interface 					struct
# define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
# define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
# define PURE                    = 0
# define THIS_
# define THIS                    void
# define DECLARE_INTERFACE(iface)    interface iface
# define DECLARE_INTERFACE_(iface, baseiface)    interface iface : public baseiface

# if !defined(BEGIN_INTERFACE)
#  define BEGIN_INTERFACE
#  define END_INTERFACE
# endif
#else /* ! __cplusplus */
 /* Vanilla C macros */
# define interface               struct

# define STDMETHOD(method)       HRESULT (STDMETHODCALLTYPE * method)
# define STDMETHOD_(type,method) type (STDMETHODCALLTYPE * method)

# if !defined(BEGIN_INTERFACE)
#  define BEGIN_INTERFACE
#  define END_INTERFACE
# endif

#define PURE
#define THIS_                   INTERFACE FAR* This,
#define THIS                    INTERFACE FAR* This

#ifdef CONST_VTABLE
# undef CONST_VTBL
# define CONST_VTBL const
# define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    const struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef const struct iface##Vtbl iface##Vtbl; \
                                const struct iface##Vtbl
#else
# undef CONST_VTBL
# define CONST_VTBL
# define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef struct iface##Vtbl iface##Vtbl; \
                                struct iface##Vtbl
#endif
										  
#define DECLARE_INTERFACE_(iface, baseiface)    DECLARE_INTERFACE(iface)

#endif /* ! __cplusplus */

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID name \
			= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


/* COM initialization flags; passed to CoInitialize. */
typedef enum tagCOINIT
{
  COINIT_APARTMENTTHREADED  = 0x2,      /* Apartment model */
  COINIT_MULTITHREADED      = 0x0,      /* OLE calls objects on any thread. */
  COINIT_DISABLE_OLE1DDE    = 0x4,      /* Don't use DDE for Ole1 support. */
  COINIT_SPEED_OVER_MEMORY  = 0x8,      /* Trade memory for speed. */
} COINIT;

/* pull in IDL generated headers */
#include "wtypes.h"

/****** STD Object API Prototypes *****************************************/

DWORD CoBuildVersion(void);

/* init/uninit */

HRESULT CoInitialize(LPVOID pvReserved);
void  CoUninitialize(void);
HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
HRESULT CoGetCurrentProcess(void);

/*
HRESULT CoCreateStandardMalloc(DWORD memctx, IMalloc ** ppMalloc);
HRESULT CoGetMalloc(DWORD dwMemContext, LPMALLOC * ppMalloc);
*/

/* register/revoke/get class objects */

HRESULT  CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved,
                    REFIID riid, LPVOID FAR* ppv);
/*
HRESULT  CoRegisterClassObject(REFCLSID rclsid, LPUNKNOWN pUnk,
                    DWORD dwClsContext, DWORD flags, LPDWORD lpdwRegister);
HRESULT  CoRevokeClassObject(DWORD dwRegister);
HRESULT  CoResumeClassObjects(void);
HRESULT  CoSuspendClassObjects(void);
*/

