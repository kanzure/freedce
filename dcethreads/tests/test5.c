/*
 * 
 * Exception Test RERAISE.
 *
 * RERAISE in the inner TRY block should dispatch to the
 * outer TRY block.
 *
 * This test insures that TRY block context is properly popped, 
 * as well as scope rules work on RERAISE.
 *
 *
 *
 */


#include <dce/pthread_exc.h>
#include <stdio.h>
#include <string.h>

static EXCEPTION e1, e2;

int main()
{

	printf ("test5:  unhandled\n");
	EXCEPTION_INIT(e1);
	EXCEPTION_INIT(e2);


	TRY
	{
		TRY
		{
			printf("about to raise e1\n");
			RAISE(e1);
		}
		CATCH(e2)
		{
			printf("\t... caught e2 in inner CATCH block. reraising.\n");
			RERAISE;
		}
		CATCH(e1)
		{
			printf("\t... caught e1 in inner CATCH block. reraising.\n");
			RERAISE;
		}
		CATCH_ALL
		{
			printf("\t... caught e1 in CATCH_ALL block. reraising.\n");
			RERAISE;
		}
		ENDTRY
	}
	ENDTRY;

	printf("done. normal exit. \n");
	return 0;
}
