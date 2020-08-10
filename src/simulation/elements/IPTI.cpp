#include "simulation/ElementCommon.h"
#include <atomic>
#include <csignal>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_IPTI()
{
	Identifier = "DEFAULT_PT_IPTI";
	Name = "IPTI";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;


	Weight = 100;
	// element properties here
	Properties = TYPE_SOLID;

	Update = &update;
	Graphics = &graphics;
}

bool IPRTL_add_part(Simulation* sim, Particle* part, int uchannel) {
	int channel = uchannel % 64;
	std::atomic<int>* ctrlshm = (std::atomic<int>*)sim->iportal_shared_mem;
	Particle* partshm = sim->iportal_part_buf;
	//printf("%ld\n", sizeof(std::atomic<int>));
	//printf("%p %p\n", &ctrlshm[channel * 4 + 1], &ctrlshm[channel * 4]);
	//printf("I2 %d %d\n", ctrlshm[channel * 4 + 1].load(), ctrlshm[channel * 4].load());
	int bufpos = ctrlshm[channel * 4]++; // reserved
	//printf("I2 %d %d\n", ctrlshm[channel * 4 + 1].load(), ctrlshm[channel * 4].load());
	if (bufpos > 255) {
		atomic_fetch_sub(&ctrlshm[channel * 4], 1);
		return false;
	}
	//printf("I2 %d %d\n", ctrlshm[channel * 4 + 1].load(), ctrlshm[channel * 4].load());
	if (bufpos < 0) {
		atomic_fetch_sub(&ctrlshm[channel * 4], 1);
		return false;
	}
	//printf("I2 %d %d\n", ctrlshm[channel * 4 + 1].load(), ctrlshm[channel * 4].load());
	int foo = 64 * channel + bufpos;
	//printf("%d\n", foo);
	//std::raise(SIGINT);
	partshm[foo] = *part;
	//printf("%p, %p\n", &partshm[foo], &ctrlshm[channel * 4]);
	
	ctrlshm[channel * 4 + 1]++; // available.
	//printf("I %d %d\n", ctrlshm[channel * 4 + 1].load(), ctrlshm[channel * 4].load());
	return true;
}

Particle* IPRTL_remove_part(Simulation* sim, int uchannel) {
	int channel = uchannel % 64;
	std::atomic<int>* ctrlshm = (std::atomic<int>*)sim->iportal_shared_mem;
	Particle* partshm = sim->iportal_part_buf;
	while(ctrlshm[channel * 4].load(std::memory_order_acquire) != ctrlshm[channel * 4 + 1].load(std::memory_order_acquire)) {}; // spinloop
	if (ctrlshm[channel * 4 + 1] <= 0) {
		return nullptr;
	}
	if (ctrlshm[channel * 4] <= 0) {
		return nullptr;
	}
	auto a = atomic_fetch_sub_explicit(&ctrlshm[channel * 4 + 1], 1, std::memory_order_release) - 1;
	atomic_fetch_sub_explicit(&ctrlshm[channel * 4], 1, std::memory_order_release);
	//printf("o\n");
	return &partshm[channel * 64 + a];
}

static int update(UPDATE_FUNC_ARGS)
{
	Particle& self = parts[i];
	for (int rx = -1; rx < 2; rx++)
		for (int ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int neighbor_data = pmap[y+ry][x+rx];

				if (!neighbor_data)
					continue;

				Particle& neighbor = parts[ID(neighbor_data)];
				if (((sim->elements[TYP(neighbor_data)].Properties & TYPE_SOLID) == 0)) {
					if (IPRTL_add_part(sim, &parts[ID(neighbor_data)], self.tmp)) {
						sim->kill_part(ID(neighbor_data));
					}
				}
			}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// graphics code here
	// return 1 if nothing dymanic happens here

	return 0;
}

