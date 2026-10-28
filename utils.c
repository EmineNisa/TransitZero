#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Return the total days in a given month, handling leap years
int days_in_month(int year, int month) {
    static const int d[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int leap = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
    if (month == 2) return d[1] + leap;
    return d[month-1];
}

// Search for a year slot or allocate a new one via pointer modification
int get_or_create_year_slot(CityAnnualData* data, int* n, int year) {
    for (int i = 0; i < *n; i++) {
        if (data[i].year == year) return i;
    }
    if (*n >= MAX_YEARS) return -1;
    int slot = (*n)++;
    data[slot].year = year;
    data[slot].energy_kwh = 0.0;
    data[slot].co2_baseline_ton = 0.0;
    data[slot].has_data = 1;
    return slot;
}

// Parse numeric string values from transit CSV file and handle regional formats
double parse_ankara_val(const char* tok) {
    if (!tok) return 0.0;
    
    char tmp[256];
    size_t len = 0;
    // Strip whitespaces, carriage returns, and newlines
    for (size_t i = 0; tok[i] != '\0' && len < sizeof(tmp) - 1; i++) {
        if (tok[i] != ' ' && tok[i] != '\r' && tok[i] != '\n') {
            tmp[len++] = tok[i];
        }
    }
    tmp[len] = '\0';

    // Return zero for empty or placeholder entries
    if (len == 0 || strcmp(tmp, "-") == 0) {
        return 0.0;
    }

    char clean[256];
    size_t c_len = 0;
    int comma_count = 0;
    int comma_pos = -1;

    // Evaluate comma format characteristics
    for (int i = 0; tmp[i] != '\0'; i++) {
        if (tmp[i] == ',') {
            comma_count++;
            comma_pos = i;
        }
    }

    // Determine if comma represents a decimal point
    int is_decimal_comma = (comma_count == 1 && ((int)len - 1 - comma_pos == 2));

    // Convert regional commas/dots to standard double format
    for (size_t i = 0; tmp[i] != '\0'; i++) {
        if (tmp[i] == '.') continue;
        if (tmp[i] == ',') {
            if (is_decimal_comma) {
                clean[c_len++] = '.';
            } else {
                continue;
            }
            continue;
        }
        clean[c_len++] = tmp[i];
    }
    clean[c_len] = '\0';

    // Attempt to parse the numeric value
    char* endptr;
    double val = strtod(clean, &endptr);

    // Return zero if no valid numeric data is found
    if (endptr == clean) {
        return 0.0;
    }

    return val;
}