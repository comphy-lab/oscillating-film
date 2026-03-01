# oscillating-film

Basilisk-based CFD project for oscillating thin-film simulations with three-phase interface dynamics.

## Development Commands

```bash
# Single-case run (uses default.params)
bash runSimulation.sh

# Single-case MPI run
bash runSimulation.sh default.params --mpi --CPUs 4

# Sweep preview (generates cases, no execution)
bash runParameterSweep.sh --dry-run

# Sweep execution
bash runParameterSweep.sh sweep.params
```

## Structure

```
oscillating-film/ - repository root
├── simulationCases/ - source `.c` files and generated case folders
├── src-local/ - project-local Basilisk headers and parameter helpers
├── postProcess/ - analysis/visualization tools
├── default.params - baseline runtime parameter file
├── sweep.params - sweep definitions (`SWEEP_*` variables)
├── runSimulation.sh - single-case compile/run entry point
├── runParameterSweep.sh - deterministic sweep generator/runner
└── .project_config - local BASILISK/PATH bootstrap
```

## Parameter Architecture

- C-side runtime parsing:
  - `src-local/parse_params.h` handles low-level `key=value` loading.
  - `src-local/params.h` exposes typed `param_int`, `param_double`, `param_bool`, `param_string`.
  - Simulation entry points should call `params_init_from_argv(argc, argv)`.
- Shell-side parsing:
  - `src-local/parse_params.sh` is the shared helper layer for `runSimulation.sh` and `runParameterSweep.sh`.
  - Avoid duplicating ad-hoc parsers in other scripts.

## Project Guidelines

- Keep simulation source in `simulationCases/*.c`.
- Treat `simulationCases/<CaseNo>/` as generated output; do not edit as source.
- Keep shared headers/utilities in `src-local/*.h` and shared shell helpers in `src-local/*.sh`.
- Keep post-processing scripts and helper sources in `postProcess/`.
- Keep `basilisk/` and `.comphy-basilisk/` ignored (local dependencies/artifacts).
- Keep `CLAUDE.md` as a local pointer file (`@AGENTS.md`) and do not track it in git.
