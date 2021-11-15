#pragma once

#include <stdint.h>

namespace rBPF
{
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

	/**
	 * @brief Update value in store
	 * @param key
	 * @param value
	 * @retval bool true on success, false if store is full
	 */
	virtual bool update(Key key, Value value) = 0;

	/**
	 * @brief Fetch value from store
	 * @param key
	 * @param value
	 * @retval bool true on success, false if store is full
	 * 
	 * If key is not found in the store then its added and set to 0.
	 */
	virtual bool fetch(Key key, Value& value) = 0;

	/**
	 * @brief Fetch value from store
	 * @param key
	 */
	Value get(Key key)
	{
		Value res;
		fetch(key, res);
		return res;
	}

	Entry operator[](Key key)
	{
		return Entry(*this, key);
	}
};

} // namespace rBPF
