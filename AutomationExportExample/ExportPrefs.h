#pragma once
#include <cstdint>
#include "EXPVERSION.h"


struct ExportPrefs
{
	uint8_t PrefsVersion = EXPPREFSVERSION;
	uint8_t PrefsFlags = 0b00000000;
	//uint8_t CostPreset = 0;
};