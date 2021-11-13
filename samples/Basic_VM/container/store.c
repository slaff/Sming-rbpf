#include <bpf/bpfapi/helpers.h>

static uint32_t lastValue1;		// Allocated in BSS
static uint32_t lastValue2 = 3; // Allocate in DATA

int64_t store()
{
	static const char str1[] = ">> Now in VM running store() <<\r\n";
	bpf_printf(str1);
	static const char str1a[] = "lastValue (%u, %u)\r\n";
	bpf_printf(str1a, lastValue1, lastValue2);

	uint32_t value1 = 0;
	bpf_fetch_global(1, &value1);
	uint32_t value2 = 0;
	bpf_fetch_local(2, &value2);

	lastValue1 = value1;
	lastValue2 = value2;

	bpf_store_global(1, value1 + 1000000);
	bpf_store_local(2, value2 + 2000000);

	uint64_t res = ((uint64_t)value1 << 32) | value2;

	static const char str2[] = ">> Now leaving store() <<\r\n";
	bpf_printf(str2);

	return res;
}
