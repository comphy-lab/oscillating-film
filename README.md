# oscillating-film

Basilisk CFD workflow for oscillating three-phase thin-film simulations with optional viscoelastic effects.

## Overview

This repository uses a CoMPhy-style layout:
- simulation source in `simulationCases/*.c`
- shared project headers/parsers in `src-local/`
- post-processing scripts in `postProcess/`
- runtime orchestration via `runSimulation.sh` and `runParameterSweep.sh`

## Requirements

- Bash
- Local Basilisk checkout available at `basilisk/` (ignored by git)
- `qcc` in `PATH` (via `.project_config`)
- Optional MPI tools for parallel runs: `mpicc`, `mpirun`
- Optional post-processing tools: `python3`, `numpy`, `matplotlib`, `ffmpeg`

Example Basilisk setup:

```bash
git clone https://github.com/comphy-lab/basilisk.git basilisk
```

## Quick Start

```bash
# Run one case with default parameters
bash runSimulation.sh

# Run one case with MPI
bash runSimulation.sh default.params --mpi --CPUs 4

# Preview generated sweep cases without running simulations
bash runParameterSweep.sh --dry-run

# Run sweep cases (serial)
bash runParameterSweep.sh sweep.params
```

## Repository Structure

```
oscillating-film/ - repository root
├── AGENTS.md - project guidance for coding agents
├── README.md - project usage and structure
├── default.params - default single-run runtime parameters
├── sweep.params - sweep definition (`SWEEP_*`, `CASE_START`, `CASE_END`)
├── runSimulation.sh - single-case runner (creates `simulationCases/<CaseNo>/`)
├── runParameterSweep.sh - sweep generator/runner built on `src-local/parse_params.sh`
├── simulationCases/ - simulation source and generated case directories
│   └── oscillatingFilm.c - primary Basilisk simulation entry point
├── src-local/ - local headers and parser helpers
│   ├── parse_params.h - low-level C key/value parser for runtime params
│   ├── params.h - typed C accessors (`param_int`, `param_double`, ...)
│   └── parse_params.sh - shared shell parser/sweep helper library
├── postProcess/ - visualization and data extraction tools
│   ├── Video-film-generic.py - snapshot-to-frame/video pipeline (supports `--cpus`)
│   ├── getFacet-threePhase.c - interface facet extractor helper source
│   └── getData-elastic-nonCoalescence.c - field sampler helper source
├── basilisk/ - local Basilisk checkout (ignored by git)
└── .project_config - exports `BASILISK` and updates `PATH`
```

## Parameter Model

- `default.params` stores baseline runtime values for a single run.
- `sweep.params` defines sweep combinations using `SWEEP_<key>=v1,v2,...`.
- `runSimulation.sh` copies the selected params file to `simulationCases/<CaseNo>/case.params` and executes the binary with `case.params` as `argv[1]`.
- `simulationCases/oscillatingFilm.c` reads params through `params_init_from_argv(argc, argv)` and typed `param_*` accessors.

## Post-Processing Notes

`postProcess/Video-film-generic.py` supports deterministic serial/parallel frame rendering:

```bash
python3 postProcess/Video-film-generic.py --case-dir simulationCases/1001 --cpus 4
```

Use `--max-frames` and `--skip-video` for quick checks.
