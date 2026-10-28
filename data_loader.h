#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "types.h"

// Load annual city transit energy consumption report
void load_ankara_energy_report(const char* filename, CityAnnualData* data, int* n);

// Load localized NASA climatology datasets
int load_nasa_climatology(const char* filename, NasaClimatology* nasa);

#endif