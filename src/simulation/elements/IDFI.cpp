#include "simulation/ElementCommon.h"
#include <atomic>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_IDFI()
{
	Identifier = "DEFAULT_PT_IDFI";
	Name = "IDFI";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	// element properties here

	Update = &update;
	Graphics = &graphics;
}

static int ZERO = 0;

static int update(UPDATE_FUNC_ARGS)
{
	std::atomic<int>* shm = (std::atomic<int>*)sim->iwifi_shared_mem;
	Particle& self = parts[i];
	if (self.life == 0) {
		std::atomic_fetch_add(&shm[1024 + (self.tmp % 1024)], 1);
		self.life = shm[1024 + (self.tmp % 1024)];
	}
	bool sending_charge = false;
	for (int rx = -1; rx < 2; rx++)
		for (int ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int neighbor_data = pmap[y+ry][x+rx];

				if (!neighbor_data)
					continue;

				Particle& neighbor = parts[ID(neighbor_data)];
				if (TYP(neighbor_data) == PT_SPRK && neighbor.ctype == PT_PSCN) {
					sending_charge = true;
					shm[self.tmp % 1024].compare_exchange_strong(ZERO, self.life);
					/*{
						sim->iwifi_shared_mem[self.tmp % 1024] = self.life;
						//printf("chargE! 2\n");
					}*/
					//printf("chargE!\n");
				} else if (TYP(neighbor_data) == PT_NSCN && neighbor.life == 0 && sim->iwifi_shared_mem[self.tmp % 1024] != 0) {
					neighbor.ctype = TYP(neighbor_data );
					sim->part_change_type(ID(neighbor_data ),x+rx,y+ry,PT_SPRK);
					neighbor.life = 4;
				}
			}
	self.ctype = sim->iwifi_shared_mem[self.tmp % 1024];
	if (!sending_charge) {
		shm[self.tmp % 1024].compare_exchange_strong(self.life, 0);
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// graphics code here
	// return 1 if nothing dymanic happens here

	return 0;
}

static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	Particle& self = sim->parts[i];
	if (to == PT_NONE) {
		if (sim->iwifi_shared_mem[self.tmp % 1024] == self.life) {
			sim->iwifi_shared_mem[self.tmp % 1024] = 0;
		}
	}
}
