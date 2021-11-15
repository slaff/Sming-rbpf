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

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	Serial.println(_F("All up, running the Femto-Container application now"));

	test_appcall();
}
