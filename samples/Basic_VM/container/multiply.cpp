#include "include/container/multiply.h"

namespace
{
int func1(void* x)
{
	(void)x;
	return 100;
}

int func2(void* x)
{
	(void)x;
	return 200;
}
} // namespace

int multiply(multiply_context_t& context)
{
	context.output = int64_t(context.input1) * context.input2;
	return 0;
}
