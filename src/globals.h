
#ifndef globalh
#define globalh

extern bool BATTERYSAVE;

typedef struct
{
  unsigned int grid_power;
  unsigned int battery_power;
  unsigned int battery_charge;
  unsigned int solar_power;
} TSysStatus;

extern TSysStatus sys_status;

#endif