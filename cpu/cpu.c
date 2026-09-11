#include "lapic.h"
#include "cpu.h"

struct cpu cpus[MAX_CPUS];
void initcpus(void) {
	for (int i = 0; i < MAX_CPUS; i++) {
		cpus[i].cpu_id = i;
		cpus[i].lapic_id = 0xFF;
		cpus[i].started = 0;
		cpus[i].ncli = 0;
		cpus[i].intena=0;
	}
}


uint8_t mycpuid(void) {
	uint8_t lapic_id = (uint8_t)(lapic_read(LAPIC_REG_ID) >> 24);
	for (int i = 0; i < MAX_CPUS; i++) {
		if (cpus[i].lapic_id == lapic_id) {
			return cpus[i].cpu_id;
		}
	}
	return 0;
}

struct cpu *mycpu(void) {
	uint8_t id = mycpuid();
	return &cpus[id];
}
