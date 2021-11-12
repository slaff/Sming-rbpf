#include <bpf/bpfapi/helpers.h>

int64_t store()
{
	uint32_t value1 = 0;
	bpf_fetch_global(1, &value1);
	uint32_t value2 = 0;
	bpf_fetch_local(2, &value2);

	bpf_store_global(1, value1 + 1000000);
	bpf_store_local(2, value2 + 2000000);

	return ((uint64_t)value1 << 32) | value2;
}
