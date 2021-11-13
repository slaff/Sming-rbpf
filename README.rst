rBPF Femto-Container support
============================

.. highlight:: bash

**rBPF**: RIOT-style Berkeley Packet Filters

The rBPF subsystem provides a minimal virtual machine for microcontrollers.
This allows code to be run in isolated environments to increase platform security.

The particular implementation here is referred to as a ``Femto-Container``
as only a very limited set of functionality is supported compared to a regular Virtual Machine.

Container code is compiled into BPF object code, which is linked into the Sming
application as a BLOB (Binary Large OBject).
The generated code has a very simple 64-bit instruction set which is then executed
using a runtime interpreter.

See :doc:`about` for further details.


Toolchain setup
---------------

The following applications are required:

- Clang/LLVM compiler suite
- `pyelftools <https://github.com/eliben/pyelftools>`_

From a sample or project directory, install as follows.

Ubuntu::

   sudo apt-get install clang
   make python-requirements

Windows::

   winget install llvm
   set CLANG="c:\Program Files\LLVM\bin\clang"
   make python-requirements


Sming interface
---------------

All .c or .cpp files are compiled into eBPF object code.
The resulting binary data is linked into the project using :cpp:class:`FSTR::Array` objects.
These can be executed using a :cpp:class:`rBPF::VirtualMachine` instance.

Compiled containers are located in the :cpp:namespace:`rBPF::Container` namespace.

Note that parameters are passed to container functions as a pointer.
You should always use a structured type for this as shown in the samples.

The compiler will use the first public function in each source file as the entry point.
It is recommended that all other functions be declared ``static`` or placed within an anonymous namespace.


Low-level details
-----------------

Clang is used to compile container source code to an eBPF object file.
This is then converted to a Femto-Container-specific format using python.

For example, the ``increment.c`` container is compiled to ``increment.o`` then converted to ``increment.bin``.
The ``.o`` file can be inspected using standard binutils tools such as objdump.

The Femto-Container application binary can be inspected as follows:

.. code-block:: bash

	make rbpf-dump

Output from the sample ``increment`` container looks like this::

	Magic:		0x46504272
	Version:	0
	flags:	0x0
	Data length:	0 B
	RoData length:	16 B
	Text length:	24 B
	No. functions:	1
	
	functions:
		"increment": 0x0
	
	data:
	
	rodata:
	    0: 0x69 0x6e 0x63 0x72 0x65 0x6d 0x65 0x6e
	    8: 0x74 0x00 0x00 0x00 0x00 0x00 0x00 0x00
	
	text:
	<increment>
	    0x0:	79 10 00 00 00 00 00 00 r0 = *(uint64_t*)(r1 + 0)
	    0x8:	07 00 00 00 01 00 00 00 r0 += 1
	   0x10:	95 00 00 00 00 00 00 00 Return r0

This shows the:

- application header
- list of functions
- read-only data containing the function name and some padding
- the application code

The application code fetches the value from the pointer in ``r1`` (the context
argument) and increments the value in the second instruction.
The return parameter is stored in register ``r0``.


Build variables
---------------

.. envvar:: RBPF_CONTAINER_PATH

	default: ``container``

	Location of Femto-Container applications.
	Place all .c and .cpp source modules here.


.. envvar:: BPF_STORE_NUM_VALUES

	default: 16

	Maximum number of stored values.

	Space is shared between all stores (global and local).


API Documentation
-----------------

.. doxygennamespace:: rBPF
   :members:
