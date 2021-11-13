#include <SmingCore.h>

// Single include required to use rBPF
#include <rbpf.h>

// Each function declares a context structure for parameter passing
#include <container/increment.h>
#include <container/multiply.h>

namespace
{
/**
 * Adds 1 to the input value and returns it from the function.
 */
void test_increment()
{
	Serial.println(F("Calling 'increment()' in VM"));
	increment_context_t ctx{
		.value = 0,
	};
	rBPF::VirtualMachine vm(rBPF::Container::increment);
	auto res = vm.execute(ctx);
	if(vm.getLastError() == 0) {
		Serial.print(_F("input "));
		Serial.print(ctx.value);
		Serial.print(_F(", result "));
		Serial.print(res);
		Serial.print(_F(", expected "));
		Serial.println(ctx.value + 1);
	}
}

/*
 * Demonstrates using the context block to return output parameters.
 * The function always returns 0.
 */
void test_multiply()
{
	Serial.println(F("Calling 'multiply()' in VM"));
	multiply_context_t ctx{
		.input1 = 120000005,
		.input2 = 120000023,
	};
	rBPF::VirtualMachine vm(rBPF::Container::multiply);
	auto res = vm.execute(ctx);
	if(vm.getLastError() == 0) {
		Serial.print(_F("input ("));
		Serial.print(ctx.input1);
		Serial.print(", ");
		Serial.print(ctx.input2);
		Serial.print(_F("), output "));
		Serial.print(ctx.output);
		Serial.print(_F(", expected "));
		Serial.print(int64_t(ctx.input1) * ctx.input2);
		Serial.print(_F(", result "));
		Serial.println(res);
	}
}

/*
 * Test data exchange using key stores
 */
void test_store()
{
	Serial.println(F("Calling 'store()' in VM"));
	rBPF::VirtualMachine vm(rBPF::Container::store);
	vm.globals[1] = 1234;
	vm.locals[2] = 5678;
	for(unsigned i = 0; i < 3; ++i) {
		auto res = vm.execute(nullptr, 0);
		if(vm.getLastError() == 0) {
			Serial.print(_F("output ("));
			Serial.print(vm.globals[1]);
			Serial.print(", ");
			Serial.print(vm.locals[2]);
			uint32_t value1 = res >> 32;
			uint32_t value2 = res;
			Serial.print(_F("), result ("));
			Serial.print(value1);
			Serial.print(", ");
			Serial.print(value2);
			Serial.println(")");
		}
	}
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	Serial.println("All up, running the Femto-Container application now");

	test_increment();
	test_multiply();
	test_store();
}
