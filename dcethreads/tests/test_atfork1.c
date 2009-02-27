#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


const char* libtool_error =
"call order does not match the specification\n"
"  this is probably due to not active pthread_atfork wrapper inside dcethreads\n"
"  look at the dcethreads library please:\n"
"  objdump -T .libs/libdcethreads.so | less\n"
"  you should find there line like this:\n"
"  ........ g    DF .text  ........  GLIBC_2.0   pthread_atfork\n"
"  if not i.e.\n"
"  ........      DF *UND*  ........  GLIBC_2.0   pthread_atfork\n"
"  you have not linked this library against the version script\n"
"  look at library link command line if you can see the following:\n"
"  -Wl,--version-script,./Versions\n"
"  if you can see it in at libtool call but not at gcc/ld call\n"
"  you are not using the right ltmain.sh script\n";

#define NO_FORK_HANDLER 7

int fork_stack_pre   [NO_FORK_HANDLER] = {0};
int fork_stack_parent[NO_FORK_HANDLER] = {0};
int fork_stack_child [NO_FORK_HANDLER] = {0};

int fork_pos_pre    = 0;
int fork_pos_parent = 0;
int fork_pos_child  = 0;


#define FORK_HANDLER(standard, order, kind, param) \
void standard ## _handler_ ## order ## _ ## kind (param) \
{ \
	fork_stack_ ## kind [ fork_pos_ ## kind ] = order; \
	fork_pos_ ## kind ++; \
}

#define FORK_HANDLER_SET(standard, order, param) \
FORK_HANDLER(standard, order, pre,    param) \
FORK_HANDLER(standard, order, parent, param) \
FORK_HANDLER(standard, order, child,  param)

#define DRAFT4_HANDLER_SET(order) \
	FORK_HANDLER_SET(draft4, order, void* __data)

#define DRAFT7_HANDLER_SET(order) \
	FORK_HANDLER_SET(draft7, order, void)

#define INSTALL_DRAFT4_HANDLER_SET(order) \
	install_atfork_handler(NULL, \
			&draft4_handler_ ## order ## _pre, \
			&draft4_handler_ ## order ## _parent, \
			&draft4_handler_ ## order ## _child)
#define INSTALL_DRAFT7_HANDLER_SET(order) \
	install_pthread_atfork_handler(\
			&draft7_handler_ ## order ## _pre, \
			&draft7_handler_ ## order ## _parent, \
			&draft7_handler_ ## order ## _child)

DRAFT4_HANDLER_SET(1)
DRAFT7_HANDLER_SET(2)
DRAFT7_HANDLER_SET(3)
DRAFT4_HANDLER_SET(4)
DRAFT7_HANDLER_SET(5)
DRAFT4_HANDLER_SET(6)
DRAFT4_HANDLER_SET(7)

void install_pthread_atfork_handler(
		void (*pre)(void),
		void (*parent)(void),
		void (*child)(void))
{
	pthread_atfork(pre, parent, child);
}

#include <dce/pthread.h>

void install_atfork_handler(void* data,
		void (*pre)(void*),
		void (*parent)(void*),
		void (*child)(void*))
{
	atfork(data, pre, parent, child);
}


void fork_and_wait(int order, const char* desc)
{
	pid_t child;
	int retval;
	int status = 0;

	child = fork();
	if (-1 == child) {
		perror(desc);
		exit(EXIT_FAILURE);
	}
	if (0 == child) {
		size_t idx;
		int failed = !!0;
		printf("child  sequence:");
		for (idx = 0; idx < order; idx++)
			printf(" %d", fork_stack_child[idx]);
		printf("\n");
		for (idx = 0; idx < order; idx++) {
			if (fork_stack_child[idx] != idx + 1) {
				printf("child sequence malformed\n");
				failed = !0;
			}
		}
		if (failed) {
			printf("%s\n", libtool_error);
			exit(EXIT_FAILURE);
		}
		else
			exit(EXIT_SUCCESS);
	}
	retval = waitpid(child, &status, 0);
	if (-1 == retval) {
		perror(desc);
		exit(EXIT_FAILURE);
	}
	if (EXIT_SUCCESS != status) {
		exit(EXIT_FAILURE);
	}
}


void print_sequences(int order)
{
	size_t idx;
	printf("pre    sequence:");
	for (idx = 0; idx < order; idx++)
		printf(" %d", fork_stack_pre[idx]);
	printf("\n");
	printf("parent sequence:");
	for (idx = 0; idx < order; idx++)
		printf(" %d", fork_stack_parent[idx]);
	printf("\n");
}


void check_sequences(int order)
{
	size_t idx;
	int failed = !!0;
	for (idx = 0; idx < order; idx++) {
		if (fork_stack_pre[idx] != order - idx) {
			printf("pre sequence malformed\n");
			failed = !0;
		}
		if (fork_stack_parent[idx] != idx + 1) {
			printf("parent sequence malformed\n");
			failed = !0;
		}
	}
	if (failed) {
		printf("%s\n", libtool_error);
		exit(EXIT_FAILURE);
	}
}


#define BZERO_VAR(x) memset((x), 0, sizeof((x)))
void run_fork_test(int order, const char* desc)
{
	BZERO_VAR(fork_stack_pre);
	BZERO_VAR(fork_stack_parent);
	BZERO_VAR(fork_stack_pre);
	fork_pos_pre     = 0;
	fork_pos_parent  = 0;
	fork_pos_child = 0;

	fork_and_wait(order, desc);
	print_sequences(order);
	check_sequences(order);
}

int main(int argc, char* argv[])
{
	fork_and_wait(0, "no handler");

	
	INSTALL_DRAFT4_HANDLER_SET(1);
	run_fork_test(1, "add dce handler1");

	INSTALL_DRAFT7_HANDLER_SET(2);
	run_fork_test(2, "add posix handler2");

	INSTALL_DRAFT7_HANDLER_SET(3);
	run_fork_test(3, "add posix handler3");

	INSTALL_DRAFT4_HANDLER_SET(4);
	run_fork_test(4, "add dce handler4");

	INSTALL_DRAFT7_HANDLER_SET(5);
	run_fork_test(5, "add posix handler5");

	INSTALL_DRAFT4_HANDLER_SET(6);
	run_fork_test(6, "add dce handler6");

	INSTALL_DRAFT4_HANDLER_SET(7);
	run_fork_test(7, "add dce handler7");

	return 0;
}


