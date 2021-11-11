#include "include/rbpf.h"
#include <bpf.h>
#include <debug_progmem.h>

namespace rBPF
{
bool VirtualMachine::initialised;

String getErrorString(int error)
{
	switch(error) {
	case BPF_OK:
		return F("OK");
	case BPF_ILLEGAL_INSTRUCTION:
		return F("ILLEGAL_INSTRUCTION");
	case BPF_ILLEGAL_MEM:
		return F("ILLEGAL_MEM");
	case BPF_ILLEGAL_JUMP:
		return F("ILLEGAL_JUMP");
	case BPF_ILLEGAL_CALL:
		return F("ILLEGAL_CALL");
	case BPF_ILLEGAL_LEN:
		return F("ILLEGAL_LEN");
	case BPF_ILLEGAL_REGISTER:
		return F("ILLEGAL_REGISTER");
	case BPF_NO_RETURN:
		return F("NO_RETURN");
	case BPF_OUT_OF_BRANCHES:
		return F("OUT_OF_BRANCHES");
	case BPF_ILLEGAL_DIV:
		return F("ILLEGAL_DIV");
	default:
		return F("ERROR ") + String(error);
	}
};

int64_t VirtualMachine::execute(const Container& container, void* ctx, size_t ctxLength)
{
	LOAD_FSTR_ARRAY(appBinary, container);
	return execute(appBinary, container.length(), ctx, ctxLength);
}

int64_t VirtualMachine::execute(const uint8_t* container, size_t containerLength, void* ctx, size_t ctxLength)
{
	if(!initialised) {
		bpf_init();
		initialised = true;
	}

	bpf_t bpf = {
		.application = container,
		.application_len = containerLength,
		.stack = stack,
		.stack_size = sizeof(stack),
	};

	bpf_setup(&bpf);
	int64_t result{-1};
	int err = bpf_execute_ctx(&bpf, ctx, ctxLength, &result);
	if(err != 0) {
		debug_e("Error! VM call failed with %d %s", err, getErrorString(err).c_str());
	}
	lastError = err;
	return result;
}

} // namespace rBPF
