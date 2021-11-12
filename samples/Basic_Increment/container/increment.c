#include "include/container/increment.h"
// #include <string.h>

static int func1(void* x)
{
	(void)x;
	return 100;
}

int increment(increment_context_t* context)
{
	// 	memset(context, 0, sizeof(increment_context_t) + 513); // Should go bang!!!
	// return context->value + 1;
	return func1(context);
}

int func2(void* x)
{
	(void)x;
	return 200;
}
