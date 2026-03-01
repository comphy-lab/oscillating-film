# oscillating-film

Basilisk CFD setup for simulating an oscillating three-phase thin film with optional viscoelastic effects.

## Overview

This repository contains a reproducible simulation workflow around `simulationCases/oscillatingFilm.c` and project-specific headers in `src-local/`. The `runSimulation.sh` wrapper handles case setup, compilation with `qcc`, and execution (serial or MPI).

## Structure

```text
oscillating-film/
├── basilisk/                  # Local Basilisk checkout (ignored by git)
├── simulationCases/           # Case templates and generated case runs
│   └── oscillatingFilm.c      # Primary simulation source
├── src-local/                 # Project-local Basilisk headers/utilities
├── default.params             # Default parameter set
├── runSimulation.sh           # End-to-end run script
└── .project_config            # Local BASILISK path bootstrap
```

## Requirements

- Bash
- Basilisk source available at `basilisk/src` locally (configured via `.project_config`)
- `qcc` in `PATH` (provided by `basilisk/src`)
- Optional MPI tools for parallel runs: `mpicc`, `mpirun`

Local setup for Basilisk:

```bash
git clone https://github.com/comphy-lab/basilisk.git basilisk
```

## Usage

```bash
# Serial run with default parameters
bash runSimulation.sh

# Serial run with explicit parameter file
bash runSimulation.sh default.params

# MPI run with 8 ranks
bash runSimulation.sh default.params --mpi --CPUs 8
```

Simulation output is written to `simulationCases/<CaseNo>/`, where `CaseNo` is read from the parameter file.

## Parameters

Edit `default.params` (or pass another params file) to control:
- mesh and runtime (`MAXlevel`, `tmax`)
- material properties for lower/film/upper media
- initial perturbation (`amp`, `lambda_wave`)

## License

No license file is currently included.
