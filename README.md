# Space Communications

This repository contains a modular C++ simulation of interplanetary optical communication and the matched Monte Carlo experiment used to evaluate Research Question 4 (RQ4): whether physical-link-aware adaptive routing improves packet-delivery reliability, delivered data, and conditional latency relative to shortest-delay routing in a dynamically varying Earth-Mars optical relay network.

The Phase 13 production experiment is the authoritative implementation for the journal results. Earlier Monte Carlo phases are retained as development and validation history; they should not be substituted for Phase 13 when reproducing the paper.

## RQ4 production files

- `Monte_Carlo_Experiments/Monte_Carlo_Experiment_4_RQ4_Production_Input_Generator.cpp` — creates 1,000 matched worlds and 200 packet templates per world.
- `Monte_Carlo_Experiments/Monte_Carlo_Experiment_4_Phase_RQ4_Production_Experiment.cpp` — evaluates B1 through B4 in four controlled environments.
- `Monte_Carlo_Experiments/Monte_Carlo_Experiment_4_Phase_13B_RQ4_Production_Statistical_Analysis.cpp` — computes paired world-level statistics.
- `Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/` — compact trial summaries and route scores used by the statistical analysis.
- `Monte_Carlo_Experiments/Phase13B_RQ4_Statistical_Results/` — reported descriptive statistics, paired comparisons, primary differences, and route-selection summaries.

## Requirements

- A C++17 compiler. The commands below use `g++`.
- A POSIX-like shell such as Linux or WSL.
- Eigen 3 is required by the production-input generator. On Ubuntu/WSL, install `libeigen3-dev`. The Phase 13 production and Phase 13B statistical programs use only the C++ standard library.

## Build

Run these commands from the repository root:

```bash
mkdir -p Builds

g++ -std=c++17 -O2 -Wall -Wextra -pedantic -I. -I/usr/include/eigen3 \
  Monte_Carlo_Experiments/Monte_Carlo_Experiment_4_RQ4_Production_Input_Generator.cpp \
  Interstellar_Communications_Network.cpp Kepler_Physics_Engine.cpp \
  -o Builds/rq4_production_input

g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  Monte_Carlo_Experiments/Monte_Carlo_Experiment_4_Phase_RQ4_Production_Experiment.cpp \
  -o Builds/phase13_rq4_production

g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  Monte_Carlo_Experiments/Monte_Carlo_Experiment_4_Phase_13B_RQ4_Production_Statistical_Analysis.cpp \
  -o Builds/phase13b_rq4_statistics
```

Each program runs embedded checks before beginning its main work and returns a nonzero exit status on validation or input failure.

## Reproduce the Phase 13 analysis from the committed compact outputs

The fastest verification path reruns the statistical analysis using the committed production summary and route-score files:

```bash
./Builds/phase13b_rq4_statistics \
  Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Trial_Summary.csv \
  Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Route_Scores.csv \
  Reproduced_Phase13B_Results
```

Compare the generated files in `Reproduced_Phase13B_Results/` with the committed files in `Monte_Carlo_Experiments/Phase13B_RQ4_Statistical_Results/`. Descriptive statistics, primary differences, and route summaries should match byte-for-byte. Extremely small analytical p-values can differ in their last few digits across C++ standard-library implementations; effect estimates, confidence intervals, test statistics, and decisions should remain unchanged.

## Reproduce the complete Phase 13 simulation

Generate the frozen inputs:

```bash
./Builds/rq4_production_input
```

The generator writes the trial and packet input files beneath `Monte_Carlo_Experiment 4/Phase_13_Production_Input/`. The packet input and packet-audit outputs are intentionally not committed because of their size. Run the production experiment with explicit paths:

```bash
./Builds/phase13_rq4_production \
  "Monte_Carlo_Experiment 4/Phase_13_Production_Input/RQ4_Production_Trial_1_Monte_Carlo_Experiment_4.csv" \
  "Monte_Carlo_Experiment 4/Phase_13_Production_Input/RQ4_Production_Packet_1_Monte_Carlo_Experiment_4.csv" \
  Reproduced_Phase13_RQ4_Production_Results
```

Then pass its trial summary and route scores to `phase13b_rq4_statistics` as shown above. If the input directory already contains a prior numbered run, use the new run number printed by the generator.

## Experimental unit and inference

The experimental unit is one Monte Carlo world/seed. Policies are evaluated in matched worlds with shared packet-level contact and optical random numbers. Statistical comparisons use trial-level paired differences rather than treating packets as independent replicates. Reliability is the primary outcome; delivered packets, delivered bits, failure counts, route selection, and latency conditional on joint delivery are secondary or exploratory outcomes.

## Large generated data

The complete Phase 13 packet audit is approximately 213 MB in the supplied project, and the Phase 12H matched packet audit is approximately 100 MB. They are excluded from GitHub. They can be regenerated locally, or deposited as separate release/archive assets if journal reviewers require the complete packet-level audit.

## Repository status

This public-ready copy removes compiled executables and machine-specific absolute paths. A license and archival DOI should be added before the repository is cited as the permanent journal reproducibility record.
