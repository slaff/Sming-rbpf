#include "include/rbpf/VirtualMachine.h"
#include "init.h"
#include <bpf.h>
#include <debug_progmem.h>

namespace rBPF
{
enum {
	RBPF_NO_MEMORY = -100,
};

GlobalStore VirtualMachine::globals;

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
	case RBPF_NO_MEMORY:
		return F("NO_MEMORY");
	default:
		return F("ERROR ") + String(error);
	}
};

bool VirtualMachine::init(const Container& container)
{
	check_init();

	appBinary.reset(new uint8_t[container.size()]);
	if(!appBinary) {
		return false;
	}
	container.readFlash(0, appBinary.get(), container.size());

	inst.reset(new uint8_t[sizeof(bpf_t)]);
	if(!inst) {
		return false;
	}

	// auto bpf = reinterpret_cast<bpf_t*>(inst.get());
	auto bpf = new(inst.get()) bpf_t({
		.application = appBinary.get(),
		.application_len = container.length(),
		.stack = stack,
		.stack_size = sizeof(stack),
	});

	bpf_setup(bpf);
	return true;
}

int64_t VirtualMachine::execute(void* ctx, size_t ctxLength)
{
	if(!appBinary || !inst) {
		return RBPF_NO_MEMORY;
	}

	int64_t result{-1};
	int err = bpf_execute_ctx(reinterpret_cast<bpf_t*>(inst.get()), ctx, ctxLength, &result);
	if(err != 0) {
		debug_e("Error! VM call failed with %d %s", err, getErrorString(err).c_str());
	}
	lastError = err;
	return result;
}

} // namespace rBPF
