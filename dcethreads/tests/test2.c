/*
 * Test cancelling in a CATCH handler.
 *
 * We throw <E1>. While processing <E1>, we cancel.
 * This should result in unwinding to the outmost TRY block
 * with a <pthread_cancel_e> exception, NOT an <E1> exception.
 *
 * Finally, the cancel should be absorbed by the outer catch
 * 
 * What happens when I cancel IN an exception handler ?? 
 * 
 */


#include <dce/pthreads_rename.h>
#include <dce/pthread_exc.h>
#include <stdio.h>
#include <string.h>

EXCEPTION e1;

int main()
{
#ifdef PTW32_STATIC_LIB
	        ptw32_processInitialize();
#endif
	printf("test2:    cancelling inside of CATCH handler\n\n");

	EXCEPTION_INIT(e1);

	sys_pthread_setasynccancel(CANCEL_OFF);
	sys_pthread_setcancel(CANCEL_ON);

	TRY
	{
		TRY
		{
			printf (" in inner try block. Raising e1\n");
			RAISE(e1);
		}
		CATCH(e1)
		{
			printf("\t... in inner handler for e1. cancelling myself\n");
			sys_pthread_cancel(sys_pthread_self());
			printf("\t... called pthread_cancel(). calling testcancel\n");
			sys_pthread_testcancel();
		}
		CATCH(pthread_cancel_e)
		{
			printf ("\t... in inner pthread_cancel_e handler\n");
		}
		ENDTRY
	}
	CATCH(e1)
	{
		printf("\t... in outer handler for e1.\n");
	}
	CATCH(pthread_cancel_e)
	{
		printf("\t... in outer handler for pthread_cancel_e\n");
		RERAISE;
	}
	ENDTRY

	printf("normal exiting. if get here, somethings wrong. \n");
	return 1;
}
