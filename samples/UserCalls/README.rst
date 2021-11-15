UserCalls
=========

.. highlight:: c++

This sample demonstrates how to provide application functions for use by VM containers.

1. Declare calls
	This sample provides three application calls::

		int bpf_user_get_magic();
		void bpf_user_set_magic(int value);
		size_t bpf_user_send_packet(char* data, size_t len)                                                   \
		size_t bpf_user_read_packet(char* buffer, size_t len)

	These are declared using the :c:macro:`BPF_SYSCALL_APP` macro.

	The ``include/bpf_appcalls.h`` must be provided in the container directory.

2. Implement calls
	The application must provide implementations for the declared functions	in the ``rBPF::VM`` namespace.
	Implementations are identical to the VM calls but with an additional pointer to the VM :cpp:typedef:`bpf_t` instance::

		int bpf_user_get_magic(bpf_t* bpf);
		void bpf_user_set_magic(bpf_t* bpf, int value);
		size_t bpf_user_send_packet(bpf_t* bpf, char* data, size_t len)                                                   \
		size_t bpf_user_read_packet(bpf_t* bpf, char* buffer, size_t len)

	Note that when passing pointers these **must** be checked using either
	:c:func:`bpf_store_allowed` or :c:func:`bpf_load_allowed`.

	See ``appcalls.cpp`` for the implementations.
