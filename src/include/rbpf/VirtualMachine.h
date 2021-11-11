#pragma once

#include <FlashString/Array.hpp>
#include <rbpf/containers.h>
#include "Store.h"
#include <memory>

namespace rBPF
{
/**
 * @brief Get text for an error code
 */
String getErrorString(int error);

class VirtualMachine
{
public:
	using Container = FSTR::Array<uint8_t>;

	/**
     * @name Create a container
     * @param container Container code blob
     */
	VirtualMachine(const Container& container) : locals(*this)
	{
		init(container);
	}

	/**
     * @name Run the container
     * @param ctx IN/OUT Passed to container. Must be persistent.
     * @retval int64_t Result returned from container
	 * @{
     */
	template <typename Context> int64_t execute(Context& ctx)
	{
		return execute(&ctx, sizeof(ctx));
	}

	int64_t execute(void* ctx, size_t ctxLength);

	int64_t execute()
	{
		return execute(nullptr, 0);
	}
	/** @} */

	/**
	 * @brief Get error code from last call to execute()
	 * @retval int 0 on success. Retrieve text for error code using `getErrorString()`
	 */
	int getLastError() const
	{
		return lastError;
	}

	static GlobalStore globals;
	LocalStore locals;

private:
	friend class LocalStore;

	bool init(const Container& container);

	std::unique_ptr<uint8_t> appBinary;
	std::unique_ptr<uint8_t> inst;
	uint8_t stack[512]{};
	int lastError{0};
};

} // namespace rBPF
