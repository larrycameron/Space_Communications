# RQ4 Robustness Automation

This directory contains the automated robustness pipeline for the RQ4 routing-policy study. It preserves the Phase 13 optical equations, B1–B4 routing equations, outcome order, seeds, and common-random-number construction.

The pipeline changes only the declared factor in each family:

- `PAT_Sensitivity`: 30, 60, and 120 seconds per hop;
- `No_Relay2`: Relay 2 unavailable, with PAT fixed at 60 seconds per hop;
- `Pointing_Loss_Sweep`: direct signal factors 1.00, 0.75, 0.50, and 0.25.

Run the mandatory 20-world PAT gate first:

```bash
bash Monte_Carlo_Experiments/Robustness/run_robustness.sh debug
```

The gate uses the first 20 frozen production seeds, includes all four Phase 13 environments, verifies PAT-factor isolation, and compares every PAT=60 trial result with the Phase 13 summary. Any mismatch stops the runner.

After inspecting the validation report and runtime log, run all three 1,000-world families:

```bash
bash Monte_Carlo_Experiments/Robustness/run_robustness.sh production
```

CSV outputs, manifests, validation reports, and runtime logs remain in their family-specific folders. Production mode does not create packet-level audit CSVs because those files would be very large; it validates packet accounting while outcomes are accumulated. Existing Phase 13 files are read-only inputs and are never overwritten.

Do not publish or add results to the manuscript until every validation report begins with `PASS` and the scientific results have been reviewed for nulls, reversals, interval width, Relay 2 dependence, and the precommitted 10-percentage-point practical-significance rule.

The paired-comparison CSV applies that rule to each B4-versus-B1 reliability cell: `Preserved`, `Qualified_Remainder`, `Inconclusive`, or `Reversal`. This label is descriptive and must be reported with the numerical estimate and confidence interval.
