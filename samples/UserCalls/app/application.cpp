#include <SmingCore.h>
#include <rbpf.h>

namespace
{
void test_appcall()
{
	Serial.println(F("Calling 'appcall()' in VM"));
	rBPF::VirtualMachine vm(rBPF::Container::appcall);
	vm.execute();
	vm.execute();
}

} // namespace

/*
	46818213 no jumptable
	46816113 jumptable
*/
void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	Serial.println(_F("All up, running the Femto-Container application now"));

	CpuCycleTimer timer;
	test_appcall();
	auto elapsed = timer.elapsedTime();
	Serial.print(_F("Call took "));
	Serial.print(elapsed);
	Serial.println(_F(" cycles"));
}
