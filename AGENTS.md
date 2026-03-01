# oscillating-film

Basilisk-based CFD project for oscillating thin-film simulations with three-phase interface dynamics.

## Structure

```text
oscillating-film/
├── basilisk/                  # Local Basilisk checkout (ignored by git)
├── simulationCases/           # Simulation source + generated case folders
├── src-local/                 # Project-specific headers and helpers
├── default.params             # Default simulation parameters
├── runSimulation.sh           # Primary execution entry point
└── .project_config            # Exports BASILISK and updates PATH
```

## Development Commands

```bash
# Run default case (serial)
bash runSimulation.sh

# Run default case with MPI
bash runSimulation.sh default.params --mpi --CPUs 4
```

## Guidelines

- Keep simulation source in `simulationCases/*.c`; keep shared headers in `src-local/*.h`.
- Treat `simulationCases/<CaseNo>/` as generated output (do not edit as source).
- Keep `basilisk/` as a local dependency checkout; it is intentionally not versioned here.
- Keep `.project_config` portable and repository-relative.
- Prefer parameterized runs through `default.params` or case-specific params files.
