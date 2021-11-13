Basic_VM 
========

.. highlight:: text

This sample is a starting point for working with the rBPF system on Sming. 

See :library:`rbpf` for toolchain setup.

The sample contains three functions:

- ``increment()`` simply adds 1 to the paramter value and returns it.
- ``multiply()`` instead stores the output value in the context parameters.
- ``store()`` demonstrates parameter passing using the stores.

The source code for these can be found in the ``container`` subdirectory.
Each file contains a single function which is compiled into eRBF code for execution by the virtual machine.

Build the sample like any regular Sming application by running `make`.
You can try it out without any hardware using the Host Emulator:

.. code-block:: bash

	make SMING_ARCH=Host
	make run

You should see this::

	All up, running the Femto-Container application now
	Calling 'increment()' in VM
	input 0, result 1, expected 1
	Calling 'multiply()' in VM
	input (120000005, 120000023), output 14400003360000115, expected 14400003360000115, result 0
	Calling 'store()' in VM
	output (1001234, 2005678), result (1234, 5678)

Note that if a runtime error occurs then an appropriate error message is displayed.

