#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
robustness_dir="$repo_root/Monte_Carlo_Experiments/Robustness"
output_dir="$robustness_dir/TTL_Sensitivity"
build_dir="$repo_root/Builds"
input_dir="$repo_root/Monte_Carlo_Experiment 4/Phase_13_Production_Input"
world_file="$repo_root/Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Route_Scores.csv"
packet_file="$repo_root/Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Packet_Audit.csv"
phase13_summary="$repo_root/Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Trial_Summary.csv"
mode="${1:-debug}"

mkdir -p "$build_dir" "$output_dir"

g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  "$robustness_dir/Robustness_Experiment.cpp" \
  -o "$build_dir/rq4_robustness"

if [[ ! -f "$world_file" ]]; then
  echo "ERROR: Frozen Phase 13 route-score input is missing: $world_file" >&2
  exit 1
fi

shopt -s nullglob
packet_candidates=("$input_dir"/RQ4_Production_Packet_*_Monte_Carlo_Experiment_4.csv)
if [[ ! -f "$packet_file" && ${#packet_candidates[@]} -eq 0 ]]; then
  echo "Frozen Phase 13 inputs were not found; generating them now."
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic -I"$repo_root" -I/usr/include/eigen3 \
    "$repo_root/Monte_Carlo_Experiments/Monte_Carlo_Experiment_4_RQ4_Production_Input_Generator.cpp" \
    "$repo_root/Interstellar_Communications_Network.cpp" "$repo_root/Kepler_Physics_Engine.cpp" \
    "$repo_root/Statistical_Data.cpp" \
    -o "$build_dir/rq4_production_input"
  (cd "$repo_root" && "$build_dir/rq4_production_input")
fi

if [[ ! -f "$packet_file" ]]; then
  packet_candidates=("$input_dir"/RQ4_Production_Packet_*_Monte_Carlo_Experiment_4.csv)
  if [[ ${#packet_candidates[@]} -gt 0 ]]; then
    packet_file="${packet_candidates[${#packet_candidates[@]}-1]}"
  fi
fi

if [[ ! -f "$packet_file" ]]; then
  echo "ERROR: Expected frozen packet input was not found." >&2
  exit 1
fi

run_ttl() {
  local worlds="$1" started ended
  started=$(date +%s)
  "$build_dir/rq4_robustness" ttl "$world_file" "$packet_file" "$output_dir" "$worlds"
  python3 "$robustness_dir/analyze_robustness.py" \
    "$output_dir/TTL_Trial_Summary.csv" \
    "$output_dir/TTL_Route_Scores.csv" \
    "$output_dir" \
    --phase13-summary "$phase13_summary"
  ended=$(date +%s)
  echo "ttl runtime_seconds=$((ended-started)) worlds=$worlds" | tee "$output_dir/Runtime_Log.txt"
}

if [[ "$mode" == "debug" ]]; then
  run_ttl 20
  echo "PASS: TTL 20-world replication and factor-isolation gate."
  echo "Review TTL_Sensitivity/TTL_Validation_Report.txt before production."
elif [[ "$mode" == "production" ]]; then
  debug_report="$output_dir/TTL_Validation_Report.txt"
  if [[ ! -f "$debug_report" ]] || [[ "$(head -n 1 "$debug_report")" != "PASS" ]]; then
    echo "ERROR: Run './Monte_Carlo_Experiments/Robustness/run_ttl_sensitivity.sh debug' first." >&2
    exit 1
  fi
  run_ttl 1000
  (cd "$repo_root" && find Monte_Carlo_Experiments/Robustness/TTL_Sensitivity -type f \
    ! -name 'SHA256SUMS' -print0 | sort -z | xargs -0 sha256sum \
    > Monte_Carlo_Experiments/Robustness/TTL_Sensitivity/SHA256SUMS)
  echo "PASS: TTL sensitivity completed and validated."
else
  echo "Usage: $0 [debug|production]" >&2
  exit 2
fi
