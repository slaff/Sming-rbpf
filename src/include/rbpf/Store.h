#pragma once

#include "common/Store.h"

namespace rBPF
{
class VirtualMachine;

class LocalStore : public Store
{
public:
	LocalStore(VirtualMachine& vm) : vm(vm)
	{
	}

	bool update(Key key, Value value) override;
	bool fetch(Key key, Value& value) override;

private:
	VirtualMachine& vm;
};

class GlobalStore : public Store
{
public:
	bool update(Key key, Value value) override;
	bool fetch(Key key, Value& value) override;
};

} // namespace rBPF
