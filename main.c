#include "types.h"
#include "utils.h"
#include "data_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void prompt_double(const char* msg, double* out) {
    printf("%s", msg);
    fflush(stdout);
    if (scanf("%lf", out) != 1) {
        printf("\nInvalid user console entry. Terminating process.\n");
        exit(1);
    }
}

static long long ceil_ll(double x) {
    if (isnan(x) || x <= 0.0) return 0;
    return (long long)ceil(x);
}

static double calculate_solar_yield(const NasaClimatology* nasa, int year, double pv_factor) {
    double total = 0.0;
    for (int m = 0; m < 12; m++) {
        if (isnan(nasa->allsky[m])) return NAN;
        total += nasa->allsky[m] * (double)days_in_month(year, m + 1);
    }
    return total * pv_factor;
}

static double calculate_wind_yield(const NasaClimatology* nasa, int year, 
                                   double rotor_d, double cp, double rho, double eff) {
    double A = M_PI * (rotor_d / 2.0) * (rotor_d / 2.0);
    double total = 0.0;
    for (int m = 0; m < 12; m++) {
        if (isnan(nasa->ws10m[m])) return NAN;
        double hours = (double)days_in_month(year, m + 1) * 24.0;
        double kwh_month = (0.5 * rho * A * cp * pow(nasa->ws10m[m], 3.0) * hours * eff) / 1000.0;
        total += kwh_month;
    }
    return total;
}

int main(void) {
    CityAnnualData city_data[MAX_YEARS];
    int nYears = 0;

printf("[SYSTEM INITIALIZED]: Loading core data models and resource files...\n");

    // Pass target variable address via pointer to guarantee cross-file updates 
    load_ankara_energy_report("data/Ankara_2018-2025_Rayli_Sistemler_Enerji_Tuketim_Verileri.csv", city_data, &nYears);

    NasaClimatology nasa;
    if (!load_nasa_climatology("data/POWER_Point_Climatology_Climatology_2018_2024_039d90N_032d86E_LST.csv", &nasa)) {
        printf("\n[FATAL ERROR]: NASA climatology file could not be parsed or is missing under 'data/'!\n");
        return 1;
    }

    printf("[INFO] : Successfully indexed and allocated %d active historical data years.\n", nYears);

    if (nYears == 0) {
        printf("\n[ERROR]: Transit workload log records are completely empty.\n");
        printf(" Please verify the baseline filename and contents within the 'data/' folder.\n");
        return 1;
    }

    double solar_share_pct, wind_share_pct;
    double panel_eff_pct, pr_pct, panel_area_m2;
    double rotor_d_m, cp;

    printf("\n========================================================\n");
    printf("               Welcome to TransitZero      \n");
    printf("========================================================\n");
    
// Fetch target renewable share allocations from user console 
    prompt_double("Target Solar Energy Share (%) [e.g., 30] : ", &solar_share_pct);
    prompt_double("Target Wind Energy Share (%)  [e.g., 70] : ", &wind_share_pct);
    
    if (fabs((solar_share_pct + wind_share_pct) - 100.0) > 0.001) {
        printf("[ERROR]: Aggregated target allocations must equal exactly 100%%.\n");
        return 1;
    }

// Fetch technical engineering parameters for infrastructure sizing 
    prompt_double("Solar PV Panel Efficiency (%) [e.g., 22]  : ", &panel_eff_pct);
    prompt_double("System Performance Ratio PR (%) [e.g., 82]: ", &pr_pct);
    prompt_double("Single Panel Physical Area (m2) [e.g., 2.1]: ", &panel_area_m2);
    prompt_double("Wind Turbine Rotor Diameter (m) [e.g., 15]  : ", &rotor_d_m);
    prompt_double("Aerodynamic Power Coefficient Cp [e.g., 0.38]: ", &cp);

    double pv_factor = (panel_eff_pct / 100.0) * (pr_pct / 100.0);
    const double rho = 1.225;
    const double wind_sys_eff = 0.92;

    FILE* csv = fopen("output/annual_net_zero.csv", "w");
    FILE* txt = fopen("output/annual_net_zero_summary.txt", "w");
    if (!csv || !txt) {
        printf("\n[ERROR]: Unable to write files to the 'output/' directory!\n");
        printf("Please ensure that a directory named 'output' exists and is writeable.\n");
        if (csv) fclose(csv);
        if (txt) fclose(txt);
        return 1;
    }

    fprintf(csv,
        "Year;Energy_kWh;CO2_Baseline_Tons;"
        "SolarShare_pct;WindShare_pct;Solar_Yield_kWh_m2_year;Wind_Yield_kWh_turb_year;"
        "SolarNeed_kWh;WindNeed_kWh;PanelsRequired;TotalSolarArea_m2;TurbinesRequired;CO2_Avoided_Tons\r\n"
    );

    fprintf(txt, "\n");
    fprintf(txt, "TransitZero Report\n");
    fprintf(txt, "-----------------------\n\n");

    for (int i = 0; i < nYears; i++) {
        int year = city_data[i].year;
        double E = city_data[i].energy_kwh;

        if (E <= 0 || isnan(E)) continue;

        double solar_yield = calculate_solar_yield(&nasa, year, pv_factor);
        double wind_yield  = calculate_wind_yield(&nasa, year, rotor_d_m, cp, rho, wind_sys_eff);

        double solar_need = E * (solar_share_pct / 100.0);
        double wind_need  = E * (wind_share_pct / 100.0);

        double yield_per_panel = solar_yield * panel_area_m2;
        long long panels = ceil_ll(solar_need / yield_per_panel);
        
        double total_solar_area = (double)panels * panel_area_m2;
        long long turbines = ceil_ll(wind_need / wind_yield);

        double co2_avoided = city_data[i].co2_baseline_ton;

        fprintf(csv, "%d;%.0f;%.2f;%.2f;%.2f;%.3f;%.3f;%.0f;%.0f;%lld;%.2f;%lld;%.2f\r\n",
                year, E, city_data[i].co2_baseline_ton,
                solar_share_pct, wind_share_pct, solar_yield, wind_yield,
                solar_need, wind_need, panels, total_solar_area, turbines, co2_avoided);

fprintf(txt, "========================================================================\n");
        fprintf(txt, "-DATA YEAR: %d\n", year);
        fprintf(txt, "========================================================================\n");
        fprintf(txt, "  [Baseline Load]      Total Traction Consumption : %.0f kWh\n", E);
        fprintf(txt, "  [Carbon Liability]   Estimated Grid CO2 Impact  : %.2f Metric Tons/Year\n", city_data[i].co2_baseline_ton);
        fprintf(txt, "  [Yield Assessment]   Solar Generation Potential : %.2f kWh/m2/year\n", solar_yield);
        fprintf(txt, "                       Wind Generation Potential  : %.2f kWh/Turbine/year\n", wind_yield);
        fprintf(txt, "  [Asset Allocation]   Solar Needed (%.0f%)       : %lld Panels (Footprint: %.2f m2)\n", solar_share_pct, panels, total_solar_area);
        fprintf(txt, "                       Wind Needed  (%.0f%)       : %lld Turbines\n\n", wind_share_pct, turbines);
    }

    // Close all active file streams safely 
    fclose(csv);
    fclose(txt);
    
    // Display terminal success notification 
    printf("\n[SUCCESS]: Asset matrix structures successfully evaluated.Thank you for using this program.\n");
    printf("[INFO]: Output logs and ledgers stored within the 'output/' directory.\n");
    
    return 0;
}