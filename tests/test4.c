//
//
//  Some more tests of thread cancellation
//
//  Create a service thread loops in a TRY block.
//  Cancel that thread.
//
//  Join on the thread and see what comes back
//


#include <dce/pthread_exc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void 
worker_thd_routine(pthread_addr_t arg)
{
	printf("worker thread starting\n");
	TRY
	{
		while(1)
		{
			printf("........zzzzz\n");
			sleep(1);
		}
	}
	CATCH_ALL
	{
		printf("worker thd caught exception!!\n");
		RERAISE;
	}
	ENDTRY;
	printf("worker thd normal exit. this shouldnt happen!!");
	arg = NULL;
}

int main()
{

	pthread_t worker;
	pthread_addr_t exit_value;

	TRY
	{
		pthd4exc_create(&worker, 
				pthread_attr_default, 
				(pthread_startroutine_t)worker_thd_routine, 
				(pthread_addr_t)NULL);
	}
	CATCH_ALL
	{
		printf("error creating worker thd\n");
		pthread_testcancel();
		exit(1);
	}
	ENDTRY;

	//
	// cancel it
	//

	sleep(5);
	printf("cancelling the worker.... \n");
	pthread_cancel(worker);
	printf("joining canceled thd\n");
	pthread_join(worker, &exit_value);
	printf("exit status of worker thd = %ld\n", exit_value);
	printf("done. exiting\n");  
	return 0;
}
