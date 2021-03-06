#pragma once

#include <FlashString/Array.hpp>
#include <rbpf/containers.h>
#include "Store.h"
#include <memory>

struct bpf_s;

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
	static constexpr size_t defaultStackSize{512};

	/**
	 * @brief Create an uninitialised VM
	 */
	VirtualMachine();

	/**
     * @name Create a VM and load a container
     * @param container Container code blob
     */
	VirtualMachine(const Container& container, size_t stackSize = defaultStackSize) : VirtualMachine()
	{
		load(container, stackSize);
	}

	~VirtualMachine();

	/**
	 * @brief Load container and initialise it
	 * @retval bool true on success
	 */
	bool load(const Container& container, size_t stackSize = defaultStackSize);

	/**
	 * @brief Unload container and free any allocated resources
	 */
	void unload();

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

	const Container* container{nullptr};
	std::unique_ptr<struct bpf_s> inst;
	std::unique_ptr<uint8_t> stack;
	size_t stackSize{0};
	int lastError{0};
};

} // namespace rBPF
