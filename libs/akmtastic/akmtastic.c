#include "SEMC_APIs.h"

int AKM_Init(int maxFormNumber, const register_map_ak897x *regs,
					const int16_t mag_layout[3][3])
{
	// TODO
	return -1;
}

void AKM_Release(void)
{
	
}

int AKM_Start(const char *path)
{
	// TODO
	return -1;
}

int AKM_Stop(const char *path)
{
	// TODO
	return -1;
}

int AKM_SaveAcc(int acc_x, int acc_y, int acc_z, int acc_sensitivity)
{
	// TODO
	return -1;
}

int AKM_SaveMag(int mag_x, int mag_y, int mag_z,
					int mag_status, const int period)
{
	// TODO
	return -1;
}

int AKM_GetOrientationValues(sensors_event_t *data)
{
	// TODO
	return -1;
}

int AKM_GetMagneticValues(sensors_event_t *data)
{
	// TODO
	return -1;
}

unsigned int AKM_GetCalibrationGoodness(void)
{
	// TODO
	return 0;
}

void AKM_ForceReCalibration(void)
{
	
}

int AKM_ChangeFormFactor(int formFactorNumber)
{
	// TODO
	return -1;
}
