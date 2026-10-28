# TransitZero: Scalable Net-Zero Urban Rail Simulation Engine

**TransitZero** is a modular computational platform developed in **C** to simulate and evaluate net-zero carbon transition strategies for urban rail transit networks (case study: Ankara Metro & ANKARAY).

## 🚀 Key Features
- **Dynamic Data Integration:** Processes municipal rail traction energy datasets dynamically alongside multi-year NASA POWER solar irradiance and wind climatology metrics.
- **Deterministic Asset Sizing:** Calculates exact PV panel counts, physical land footprint ($m^2$), wind turbine requirements, and annual displaced $CO_2$ emissions.
- **Global Scalability:** Strictly modular software architecture (`types.h`, `data_loader`, `utils`, `main.c`) enabling instant adaptation to any city worldwide by updating input coordinates and datasets.

## 📁 Repository Structure
- `data/`: CSV datasets for rail energy consumption and NASA POWER climatology.
- `types.h`: Global constants, data structures, and engineering metrics.
- `utils.c / .h`: Leap-year algorithms, custom numeric parsers, and string cleaning functions.
- `data_loader.c / .h`: Memory-safe CSV loading and data sanitization via pointers.
- `main.c`: Core execution controller and simulation engine.

## 🛠️ Build & Run
To compile and run locally using GCC:
```bash
gcc main.c utils.c data_loader.c -o TransitZero -lm
./TransitZero
