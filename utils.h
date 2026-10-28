#ifndef UTILS_H
#define UTILS_H

#include "types.h"

// Return the total days in a given month, handling leap years
int days_in_month(int year, int month);

// Parse numeric string values from transit CSV file and handle regional formats
double parse_ankara_val(const char* tok);

// Search for a year slot or allocate a new one via pointer modification
int get_or_create_year_slot(CityAnnualData* data, int* n, int year);

#endif 