#ifndef TYPES_H
#define TYPES_H

#define LINE_LEN 4096
#define MONTHS 12
#define MAX_YEARS 64

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Data structure for city transit annual energy metrics
typedef struct {
    int year;                   // Tracked calendar year
    double energy_kwh;          // Total traction energy consumption
    double co2_baseline_ton;    // Displaced grid carbon footprint
    int has_data;               // Boolean flag (1 or 0) indicating valid data entry
} CityAnnualData;

// Data structure for NASA meteorological datasets
typedef struct {
    double allsky[MONTHS];      // Monthly average solar irradiance
    double ws10m[MONTHS];       // Monthly average wind speed
    int has_allsky;             // Boolean flag for valid solar data parsing
    int has_ws10m;              // Boolean flag for valid wind data parsing
} NasaClimatology;

#endif