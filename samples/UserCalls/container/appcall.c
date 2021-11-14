#include <bpf/bpfapi/helpers.h>

int64_t appcall()
{
	bpf_printf("bpf_user_get_magic() returned %d\r\n", bpf_user_get_magic());
	bpf_user_set_magic(5000);

	char packet[] = "This is my packet";
	bpf_user_send_packet(packet, sizeof(packet));

	char buffer[128];
	size_t len = bpf_user_read_packet(buffer, sizeof(buffer) - 1);
	buffer[len] = '\0';
	bpf_printf("Read %u bytes: %s\r\n", len, buffer);

	return 0;
}
