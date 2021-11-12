#pragma once

#include <cstdint>

namespace rBPF
{
class VirtualMachine;

class Store
{
public:
	using Key = uint32_t;
	using Value = uint32_t;

	class Entry
	{
	public:
		explicit operator bool() const
		{
			return valid;
		}

		Entry& operator=(Value value)
		{
			valid = store.update(key, value);
			return *this;
		}

		operator Value() const
		{
			return store.get(key);
		}

	private:
		friend Store;

		Entry(Store& store, Key key) : store(store), key(key)
		{
		}

		Store& store;
		Key key;
		bool valid;
	};

	virtual bool update(Key key, Value value) = 0;
	virtual bool fetch(Key key, Value& value) = 0;

	Value get(Key key, Value defaultValue = 0)
	{
		Value res;
		return fetch(key, res) ? res : defaultValue;
	}

	Entry operator[](Key key)
	{
		return Entry(*this, key);
	}
};

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
