#pragma once

#include <FlashString/Array.hpp>
#include <rbpf/containers.h>

namespace rBPF
{
String getErrorString(int error);

class VirtualMachine
{
public:
	using Container = FSTR::Array<uint8_t>;

	/**
     * @brief Execute a container
     * @param container Container code to execute
     * @param ctx IN/OUT Passed to container
     * @retval int64_t Result returned from container
     */
	template <typename Context> int64_t execute(const Container& container, Context& ctx)
	{
		return execute(container, &ctx, sizeof(ctx));
	}

	int64_t execute(const Container& container, void* ctx, size_t ctxLength);
	int64_t execute(const uint8_t* container, size_t containerLength, void* ctx, size_t ctxLength);

	int getLastError() const
	{
		return lastError;
	}

private:
	uint8_t stack[512]{};
	int lastError{0};
	static bool initialised;
};

} // namespace rBPF
