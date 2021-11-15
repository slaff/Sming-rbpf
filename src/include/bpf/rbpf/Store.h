#pragma once

#include "../../rbpf/common/Store.h"
#include <bpf/bpfapi/helpers.h>

namespace rBPF
{
class LocalStore : public Store
{
public:
	bool update(Key key, Value value) override
	{
		return bpf_store_local(key, value) == 0;
	}

	bool fetch(Key key, Value& value) override
	{
		return bpf_fetch_local(key, &value) == 0;
	}
};

class GlobalStore : public Store
{
public:
	bool update(Key key, Value value) override
	{
		return bpf_store_global(key, value) == 0;
	}

	bool fetch(Key key, Value& value) override
	{
		return bpf_fetch_global(key, &value) == 0;
	}
};

} // namespace rBPF
