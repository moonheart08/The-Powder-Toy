#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_IPTO()
{
	Identifier = "DEFAULT_PT_IPTO";
	Name = "IPTO";
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

Particle* IPRTL_remove_part(Simulation* sim, int uchannel);

static int update(UPDATE_FUNC_ARGS)
{
	Particle& self = parts[i];
	for (int rx = -1; rx < 2; rx++)
		for (int ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int neighbor_data = pmap[y+ry][x+rx];

				if (!neighbor_data) {
					
					Particle* trans = IPRTL_remove_part(sim, self.tmp);
					if (trans != nullptr) {
						int nid = sim->create_part(-1, x + rx, y + ry, PT_DUST);
						parts[nid] = *trans;
						parts[nid].x = x + rx;
						parts[nid].y = y + ry;
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

