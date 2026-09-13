#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
robustness_dir="$repo_root/Monte_Carlo_Experiments/Robustness"
build_dir="$repo_root/Builds"
input_dir="$repo_root/Monte_Carlo_Experiment 4/Phase_13_Production_Input"
world_file="$repo_root/Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Route_Scores.csv"
packet_file="$repo_root/Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Packet_Audit.csv"
phase13_summary="$repo_root/Monte_Carlo_Experiments/Phase13_RQ4_Production_Results/Phase13_RQ4_Production_Trial_Summary.csv"
mode="${1:-debug}"

mkdir -p "$build_dir" "$robustness_dir/PAT_Sensitivity" "$robustness_dir/No_Relay2" "$robustness_dir/Pointing_Loss_Sweep"

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
  echo "ERROR: Expected frozen input filenames were not generated." >&2
  echo "Inspect: $input_dir" >&2
  exit 1
fi

run_family() {
  local family="$1" output="$2" worlds="$3" include_null="${4:-}"
  local started ended
  started=$(date +%s)
  "$build_dir/rq4_robustness" "$family" "$world_file" "$packet_file" "$output" "$worlds" $include_null
  python3 "$robustness_dir/analyze_robustness.py" \
    "$output/$(case "$family" in pat) echo PAT;; no-relay2) echo No_Relay2;; pointing) echo Pointing;; esac)_Trial_Summary.csv" \
    "$output/$(case "$family" in pat) echo PAT;; no-relay2) echo No_Relay2;; pointing) echo Pointing;; esac)_Route_Scores.csv" \
    "$output" \
    $(if [[ "$family" == pat ]]; then printf '%q %q' --phase13-summary "$phase13_summary"; fi)
  ended=$(date +%s)
  echo "$family runtime_seconds=$((ended-started)) worlds=$worlds" | tee -a "$output/Runtime_Log.txt"
}

if [[ "$mode" == "debug" ]]; then
  run_family pat "$robustness_dir/PAT_Sensitivity" 20 --include-null
  echo "PASS: PAT 20-world replication and leakage gate."
  echo "Review PAT_Sensitivity/PAT_Validation_Report.txt before production."
elif [[ "$mode" == "production" ]]; then
  debug_report="$robustness_dir/PAT_Sensitivity/PAT_Validation_Report.txt"
  if [[ ! -f "$debug_report" ]] || [[ "$(head -n 1 "$debug_report")" != "PASS" ]]; then
    echo "ERROR: Run './Monte_Carlo_Experiments/Robustness/run_robustness.sh debug' first." >&2
    exit 1
  fi
  run_family pat "$robustness_dir/PAT_Sensitivity" 1000
  run_family no-relay2 "$robustness_dir/No_Relay2" 1000
  run_family pointing "$robustness_dir/Pointing_Loss_Sweep" 1000
  (cd "$repo_root" && find Monte_Carlo_Experiments/Robustness -type f ! -path '*/__pycache__/*' \
    ! -name '*.cpp' ! -name '*.py' ! -name '*.sh' ! -name 'SHA256SUMS' -print0 | sort -z | xargs -0 sha256sum \
    > Monte_Carlo_Experiments/Robustness/SHA256SUMS)
  echo "PASS: all three robustness families completed and validated."
else
  echo "Usage: $0 [debug|production]" >&2
  exit 2
fi
