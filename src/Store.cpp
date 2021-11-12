#include "init.h"
#include "include/rbpf/Store.h"
#include "include/rbpf/VirtualMachine.h"
#include <bpf.h>
#include <bpf/store.h>

namespace rBPF
{
bool LocalStore::update(Key key, Value value)
{
	return bpf_store_update_local(reinterpret_cast<bpf_t*>(vm.inst.get()), key, value) == 0;
}

bool LocalStore::fetch(Key key, Value& value)
{
	return bpf_store_fetch_local(reinterpret_cast<bpf_t*>(vm.inst.get()), key, &value) == 0;
}

bool GlobalStore::update(Key key, Value value)
{
	return bpf_store_update_global(key, value) == 0;
}

bool GlobalStore::fetch(Key key, Value& value)
{
	return bpf_store_fetch_global(key, &value) == 0;
}

// void bpf_store_iter_global(btree_cb_t cb, void* ctx);

} // namespace rBPF
