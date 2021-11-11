#include <SmingCore.h>

#include <rbpf.h>
#include <increment.h>
#include <int64/multiply.h>

namespace
{
void test_increment()
{
	Serial.println(F("Calling 'increment()' in VM"));
	increment_context_t ctx{
		.value = 0,
	};
	rBPF::VirtualMachine vm;
	auto res = vm.execute(rBPF::Container::increment, ctx);
	if(vm.getLastError() == 0) {
		Serial.print(_F("input "));
		Serial.print(ctx.value);
		Serial.print(_F(", result "));
		Serial.print(res);
		Serial.print(_F(", expected "));
		Serial.println(ctx.value + 1);
	}
}

void test_multiply()
{
	Serial.println(F("Calling 'multiply()' in VM"));
	multiply_context_t ctx{
		.input1 = 120000005,
		.input2 = 120000023,
	};
	rBPF::VirtualMachine vm;
	auto res = vm.execute(rBPF::Container::int64_multiply, ctx);
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

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	Serial.println("All up, running the Femto-Container application now");

	test_increment();
	test_multiply();
}
