// Declare our application calls
#define BPF_SYSCALL_APP(XX)                                                                                            \
	XX(0x0100, bpf_user_get_magic, int)                                                                                \
	XX(0x0101, bpf_user_set_magic, void, int value)                                                                    \
	XX(0x0102, bpf_user_send_packet, size_t, char* data, size_t len)                                                   \
	XX(0x0103, bpf_user_read_packet, size_t, char* data, size_t len)
