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

VirtualMachine::VirtualMachine() : locals(*this)
{
}

VirtualMachine::~VirtualMachine()
{
	unload();
}

bool VirtualMachine::load(const Container& container, size_t stackSize)
{
	check_init();

	unload();

	this->container = &container;

	if(stackSize != this->stackSize) {
		stack.reset(new uint8_t[stackSize]);
		if(!stack) {
			debug_e("[VM] No memory for stack");
			return false;
		}
		this->stackSize = stackSize;
	}

	inst.reset(new struct bpf_s({
		.application = container.data(),
		.application_len = container.length(),
		.stack = stack.get(),
		.stack_size = stackSize,
	}));
	if(bpf_setup(inst.get()) < 0) {
		debug_e("[VM] Init failed");
		inst.reset();
		return false;
	}

	return true;
}

void VirtualMachine::unload()
{
	if(inst) {
		bpf_destroy(inst.get());
		inst.reset();
	}
	lastError = 0;
}

int64_t VirtualMachine::execute(void* ctx, size_t ctxLength)
{
	if(!inst) {
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
