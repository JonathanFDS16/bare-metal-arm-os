

void reboot() {
	__asm volatile ("svc #1");
}
