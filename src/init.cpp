#include "init.h"
#include <bpf.h>

namespace rBPF
{
namespace
{
bool initialised;
};

void check_init()
{
	if(!initialised) {
		bpf_init();
		initialised = true;
	}
}

} // namespace rBPF
