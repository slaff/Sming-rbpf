#include <bpf.h>
#include <debug_progmem.h>
#include <WString.h>

namespace rBPF
{
namespace VM
{
namespace
{
int magic{1234};
}

int bpf_user_get_magic(bpf_t* bpf)
{
	(void)bpf;
	debug_i("%s() returning %d", __FUNCTION__, magic);
	return magic;
}

void bpf_user_set_magic(bpf_t* bpf, int value)
{
	(void)bpf;
	debug_i("%s(%d)", __FUNCTION__, value);
	magic = value;
}

size_t bpf_user_send_packet(bpf_t* bpf, char* data, size_t len)
{
	// Verify that data points to valid readable memory
	if(bpf_load_allowed(bpf, data, len) < 0) {
		return 0;
	}
	m_printHex("SEND", data, len);
	return len;
}

size_t bpf_user_read_packet(bpf_t* bpf, char* buffer, size_t len)
{
	DEFINE_FSTR_LOCAL(FS_test, "Some test packet content to return");
	LOAD_FSTR(test, FS_test);
	len = std::min(len, FS_test.length());
	// Verify that buffer points to valid writeable memory
	if(bpf_store_allowed(bpf, buffer, len) < 0) {
		return 0;
	}
	memcpy(buffer, test, len);
	return len;
}

} // namespace VM
} // namespace rBPF
