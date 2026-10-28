#include "data_loader.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void load_ankara_energy_report(const char* filename, CityAnnualData* data, int* n) {
    FILE* f = fopen(filename, "r");
    if (!f) { 
        printf("[ERROR]: Unable to open transit energy file: %s\n", filename); 
        return; 
    }

    char line[LINE_LEN];
    // Skip CSV header line safely
    if (!fgets(line, LINE_LEN, f)) { fclose(f); return; }

    while (fgets(line, LINE_LEN, f)) {
        char clean[LINE_LEN];
        int c_i = 0;
        // Sanitize row string by removing enclosing quotes
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] != '"') clean[c_i++] = line[i];
        }
        clean[c_i] = '\0';

        char* save = NULL;
        char* tok = strtok_r(clean, ";", &save);
        if (!tok) continue;
        int year = atoi(tok);
        if (year < 2000 || year > 2100) continue;

        // Allocate or fetch tracking index slot via pointer modification
        int slot = get_or_create_year_slot(data, n, year);
        if (slot < 0) continue;

        double total_year_energy = 0.0;
        for (int m = 0; m < 12; m++) {
            tok = strtok_r(NULL, ";\r\n", &save);
            if (tok) {
                total_year_energy += parse_ankara_val(tok);
            }
        }
        
        data[slot].energy_kwh += total_year_energy;
        data[slot].co2_baseline_ton += (total_year_energy * 0.00045);
    }
    fclose(f);
}

int load_nasa_climatology(const char* filename, NasaClimatology* nasa) {
    FILE* f = fopen(filename, "r");
    if (!f) { 
        printf("[ERROR]: Unable to open NASA climatology file: %s\n", filename); 
        return 0; 
    }

    char line[LINE_LEN];
    // Initialize model validation flags
    nasa->has_allsky = 0;
    nasa->has_ws10m = 0;

    while (fgets(line, LINE_LEN, f)) {
        // Filter discrete rows matching targeted climate variables
        int is_allsky = (strncmp(line, "ALLSKY_SFC_SW_DWN", 17) == 0);
        int is_ws10m  = (strncmp(line, "WS10M", 5) == 0 && strncmp(line, "WS10M_MAX", 9) != 0);
        
        if (!is_allsky && !is_ws10m) continue;

        char* save = NULL;
        char* tok = strtok_r(line, ",", &save);
        
        // Parse 12 month climate values sequentially
        for (int m = 0; m < 12; m++) {
            tok = strtok_r(NULL, ",", &save);
            if (!tok) break;
            double v = atof(tok);
            if (v == -999.0) v = NAN; // Handle missing data indicators

            if (is_allsky) { nasa->allsky[m] = v; nasa->has_allsky = 1; }
            if (is_ws10m)  { nasa->ws10m[m] = v;  nasa->has_ws10m = 1; }
        }
    }
    fclose(f);
    return (nasa->has_allsky && nasa->has_ws10m);
}