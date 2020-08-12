#include "simulation/ElementCommon.h"
#include <atomic>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_IDFI()
{
	Identifier = "DEFAULT_PT_IDFI";
	Name = "IDFI";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = -0.005f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 0;
	Description = "Interdimensional Wifi. Similar to normal wifi, but can send signals across multiple instances of the game!";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;


	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	std::atomic<int>* shm = (std::atomic<int>*)sim->iwifi_shared_mem;
	sim->parts[i].life = std::atomic_fetch_add(&shm[1024], 1);
}

static int update(UPDATE_FUNC_ARGS)
{
	std::atomic<int>* shm = (std::atomic<int>*)sim->iwifi_shared_mem;
	Particle& self = parts[i];
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
					int z = 0;
					shm[self.tmp % 1024].compare_exchange_strong(z, self.life);
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
		int foo = self.life;
		shm[self.tmp % 1024].compare_exchange_strong(foo, 0);
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
