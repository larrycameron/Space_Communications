#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ============================================================
// MONTE CARLO EXPERIMENT 4 - RQ4
// PHASE 7A - POLICY-DISCRIMINATION STRESS TEST
//
// PURPOSE
// -------
// This is a deterministic VALIDATION executable for the Phase 7
// B1-B4 scoring and route-selection logic. It does NOT generate
// Monte Carlo research data and must NOT be pooled with pilot or
// production results.
//
// Phase 7 Run 11 showed mechanically correct scoring/selection but
// all B1-B4 decisions selected the direct route in the five debug
// worlds. Phase 7A therefore creates controlled synthetic route-state
// cases with analytically known winners so that route discrimination,
// hard availability gates, tie-breaking, and decision boundaries can
// be tested directly.
//
// IMPORTANT
// ---------
// The scoring functions below intentionally duplicate Phase 7 logic.
// If Phase 7 scoring is later changed, Phase 7A must be updated and
// revalidated before use.
// ============================================================

enum class Routing_Policy
{
    B1,
    B2,
    B3,
    B4
};

struct RQ4_Candidate_Route
{
    std::string Route_Name;
    bool Available{true};
};

struct RQ4_Route_State
{
    RQ4_Candidate_Route Route;

    double Route_Contact_Realization_Probability{};
    double PAT_Cost_Seconds{};
    double Link_Stability{};
    double Propagation_Delay_Seconds{};

    bool Contact_Available{true};
};

struct RQ4_Policy_Decision
{
    Routing_Policy Policy{};
    std::array<double, 4> Route_Scores{};
    int Selected_Route_Index{-1};
    std::string Selected_Route_Name{"NO_ROUTE"};
    double Selected_Route_Score{-1.0};
};

struct RQ4_Stress_Test_Case
{
    std::string Case_ID;
    std::string Purpose;
    std::array<RQ4_Route_State, 4> Routes;
    std::array<int, 4> Expected_Route_Index_By_Policy{};
};

static constexpr double SCORE_TIE_TOLERANCE = 1.0e-12;

std::string Policy_Name(Routing_Policy policy)
{
    switch (policy)
    {
        case Routing_Policy::B1: return "B1";
        case Routing_Policy::B2: return "B2";
        case Routing_Policy::B3: return "B3";
        case Routing_Policy::B4: return "B4";
    }

    return "UNKNOWN";
}

std::string Route_Name_From_Index(int index)
{
    switch (index)
    {
        case 0: return "Direct_Earth_to_Mars";
        case 1: return "Earth_Relay0_Mars";
        case 2: return "Earth_Relay1_Mars";
        case 3: return "Earth_Relay2_Mars";
        default: return "NO_ROUTE";
    }
}

double Clamp_Zero_To_One(double value)
{
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
}

double Find_Minimum_Available_PAT_Cost(
    const std::vector<RQ4_Route_State>& route_states)
{
    double minimum = -1.0;

    for (const RQ4_Route_State& state : route_states)
    {
        if (!state.Route.Available ||
            !state.Contact_Available ||
            state.PAT_Cost_Seconds <= 0.0)
        {
            continue;
        }

        if (minimum < 0.0 || state.PAT_Cost_Seconds < minimum)
        {
            minimum = state.PAT_Cost_Seconds;
        }
    }

    return minimum;
}

double Find_Minimum_Available_Propagation_Delay(
    const std::vector<RQ4_Route_State>& route_states)
{
    double minimum = -1.0;

    for (const RQ4_Route_State& state : route_states)
    {
        if (!state.Route.Available ||
            !state.Contact_Available ||
            state.Propagation_Delay_Seconds <= 0.0)
        {
            continue;
        }

        if (minimum < 0.0 ||
            state.Propagation_Delay_Seconds < minimum)
        {
            minimum = state.Propagation_Delay_Seconds;
        }
    }

    return minimum;
}

double Calculate_Policy_Score(
    Routing_Policy policy,
    const RQ4_Route_State& state,
    double minimum_available_pat_cost,
    double minimum_available_propagation_delay)
{
    if (!state.Route.Available || !state.Contact_Available)
    {
        return -1.0;
    }

    const double contact_score =
        Clamp_Zero_To_One(
            state.Route_Contact_Realization_Probability);

    double pat_efficiency = 0.0;

    if (minimum_available_pat_cost > 0.0 &&
        state.PAT_Cost_Seconds > 0.0)
    {
        pat_efficiency =
            Clamp_Zero_To_One(
                minimum_available_pat_cost /
                state.PAT_Cost_Seconds);
    }

    double propagation_efficiency = 0.0;

    if (minimum_available_propagation_delay > 0.0 &&
        state.Propagation_Delay_Seconds > 0.0)
    {
        propagation_efficiency =
            Clamp_Zero_To_One(
                minimum_available_propagation_delay /
                state.Propagation_Delay_Seconds);
    }

    switch (policy)
    {
        case Routing_Policy::B1:
            return contact_score;

        case Routing_Policy::B2:
            return contact_score * pat_efficiency;

        case Routing_Policy::B3:
            return pat_efficiency *
                   Clamp_Zero_To_One(state.Link_Stability);

        case Routing_Policy::B4:
            return pat_efficiency *
                   Clamp_Zero_To_One(state.Link_Stability) *
                   propagation_efficiency;
    }

    return -1.0;
}

RQ4_Policy_Decision Evaluate_Policy(
    Routing_Policy policy,
    const std::vector<RQ4_Route_State>& route_states)
{
    RQ4_Policy_Decision decision;
    decision.Policy = policy;
    decision.Route_Scores.fill(-1.0);

    const double minimum_pat_cost =
        Find_Minimum_Available_PAT_Cost(route_states);

    const double minimum_propagation_delay =
        Find_Minimum_Available_Propagation_Delay(route_states);

    for (std::size_t i = 0; i < route_states.size(); ++i)
    {
        const double score =
            Calculate_Policy_Score(
                policy,
                route_states[i],
                minimum_pat_cost,
                minimum_propagation_delay);

        decision.Route_Scores[i] = score;

        if (score < 0.0)
        {
            continue;
        }

        bool choose_route = false;

        if (decision.Selected_Route_Index < 0 ||
            score > decision.Selected_Route_Score +
                    SCORE_TIE_TOLERANCE)
        {
            choose_route = true;
        }
        else if (std::abs(
                     score - decision.Selected_Route_Score) <=
                 SCORE_TIE_TOLERANCE)
        {
            const RQ4_Route_State& current =
                route_states[
                    static_cast<std::size_t>(
                        decision.Selected_Route_Index)];

            // Exact Phase 7 deterministic tie-break order:
            // 1) lower propagation delay
            // 2) lower PAT cost
            // 3) lower route index (implicit: keep current)
            if (route_states[i].Propagation_Delay_Seconds <
                current.Propagation_Delay_Seconds)
            {
                choose_route = true;
            }
            else if (
                route_states[i].Propagation_Delay_Seconds ==
                    current.Propagation_Delay_Seconds &&
                route_states[i].PAT_Cost_Seconds <
                    current.PAT_Cost_Seconds)
            {
                choose_route = true;
            }
        }

        if (choose_route)
        {
            decision.Selected_Route_Index =
                static_cast<int>(i);

            decision.Selected_Route_Name =
                route_states[i].Route.Route_Name;

            decision.Selected_Route_Score = score;
        }
    }

    return decision;
}

RQ4_Route_State Make_Route(
    int route_index,
    double contact_probability,
    double pat_seconds,
    double link_stability,
    double propagation_delay_seconds,
    bool available = true)
{
    RQ4_Route_State state;
    state.Route.Route_Name = Route_Name_From_Index(route_index);
    state.Route.Available = available;
    state.Route_Contact_Realization_Probability = contact_probability;
    state.PAT_Cost_Seconds = pat_seconds;
    state.Link_Stability = link_stability;
    state.Propagation_Delay_Seconds = propagation_delay_seconds;
    state.Contact_Available = available;
    return state;
}

std::vector<RQ4_Stress_Test_Case> Build_Stress_Test_Cases()
{
    std::vector<RQ4_Stress_Test_Case> cases;

    // --------------------------------------------------------
    // CASE 1: Direct dominates every policy.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_01_DIRECT_DOMINANT",
        "Control case: direct route should win B1-B4.",
        {
            Make_Route(0, 0.90, 60.0, 0.30, 500.0),
            Make_Route(1, 0.80, 120.0, 0.20, 700.0),
            Make_Route(2, 0.75, 120.0, 0.18, 800.0),
            Make_Route(3, 0.70, 120.0, 0.16, 900.0)
        },
        {0, 0, 0, 0}
    });

    // --------------------------------------------------------
    // CASE 2: Relay0 dominates every policy.
    // Synthetic equal-PAT conditions isolate route discrimination.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_02_RELAY0_DOMINANT",
        "Route-index discrimination: Relay0 should win B1-B4.",
        {
            Make_Route(0, 0.70, 60.0, 0.40, 500.0),
            Make_Route(1, 0.95, 60.0, 0.95, 400.0),
            Make_Route(2, 0.60, 60.0, 0.30, 700.0),
            Make_Route(3, 0.55, 60.0, 0.25, 800.0)
        },
        {1, 1, 1, 1}
    });

    // --------------------------------------------------------
    // CASE 3: Relay1 dominates every policy.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_03_RELAY1_DOMINANT",
        "Route-index discrimination: Relay1 should win B1-B4.",
        {
            Make_Route(0, 0.70, 60.0, 0.40, 500.0),
            Make_Route(1, 0.60, 60.0, 0.30, 700.0),
            Make_Route(2, 0.96, 60.0, 0.96, 390.0),
            Make_Route(3, 0.55, 60.0, 0.25, 800.0)
        },
        {2, 2, 2, 2}
    });

    // --------------------------------------------------------
    // CASE 4: Relay2 dominates every policy.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_04_RELAY2_DOMINANT",
        "Route-index discrimination: Relay2 should win B1-B4.",
        {
            Make_Route(0, 0.70, 60.0, 0.40, 500.0),
            Make_Route(1, 0.60, 60.0, 0.30, 700.0),
            Make_Route(2, 0.55, 60.0, 0.25, 800.0),
            Make_Route(3, 0.97, 60.0, 0.97, 380.0)
        },
        {3, 3, 3, 3}
    });

    // --------------------------------------------------------
    // CASE 5: Numerically strongest route is unavailable.
    // Hard gate must reject Relay1; direct should win.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_05_UNAVAILABLE_BEST_ROUTE",
        "Availability hard gate: unavailable Relay1 must never be selected.",
        {
            Make_Route(0, 0.90, 60.0, 0.40, 500.0),
            Make_Route(1, 0.80, 120.0, 0.30, 700.0),
            Make_Route(2, 1.00, 10.0, 1.00, 100.0, false),
            Make_Route(3, 0.70, 120.0, 0.25, 800.0)
        },
        {0, 0, 0, 0}
    });

    // --------------------------------------------------------
    // CASE 6: Incremental-information discrimination.
    // B3 values the relay's stronger stability enough to choose it.
    // B4 adds propagation delay and returns to direct.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_06_B3_B4_DIVERGENCE",
        "Incremental-information test: B3 chooses Relay0; B4 chooses direct after propagation penalty.",
        {
            Make_Route(0, 0.90, 60.0, 0.10, 500.0),
            Make_Route(1, 0.80, 120.0, 0.40, 1500.0),
            Make_Route(2, 1.00, 1.0, 1.00, 1.0, false),
            Make_Route(3, 1.00, 1.0, 1.00, 1.0, false)
        },
        {0, 0, 1, 0}
    });

    // --------------------------------------------------------
    // CASE 7: Equal scores; lower propagation delay breaks tie.
    // Relay0 should win all policies.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_07_TIE_BREAK_PROPAGATION",
        "Tie-break validation: equal primary scores, Relay0 wins on lower propagation delay.",
        {
            Make_Route(0, 0.80, 60.0, 0.40, 500.0),
            Make_Route(1, 0.80, 60.0, 0.40, 400.0),
            Make_Route(2, 1.00, 1.0, 1.00, 1.0, false),
            Make_Route(3, 1.00, 1.0, 1.00, 1.0, false)
        },
        {1, 1, 1, 1}
    });

    // --------------------------------------------------------
    // CASE 8: Equal B1 score and equal propagation delay;
    // lower PAT cost breaks B1 tie. For B2-B4 the lower PAT cost
    // also directly creates the higher score.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_08_TIE_BREAK_PAT",
        "Tie-break validation: Relay0 wins B1 on lower PAT after equal score/delay; B2-B4 also favor lower PAT.",
        {
            Make_Route(0, 0.80, 120.0, 0.40, 500.0),
            Make_Route(1, 0.80, 60.0, 0.40, 500.0),
            Make_Route(2, 1.00, 1.0, 1.00, 1.0, false),
            Make_Route(3, 1.00, 1.0, 1.00, 1.0, false)
        },
        {1, 1, 1, 1}
    });

    // --------------------------------------------------------
    // CASE 9: Exact equality through score, delay, and PAT.
    // Lower route index must remain selected.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_09_TIE_BREAK_ROUTE_INDEX",
        "Final tie-break validation: exact equality must preserve lower route index (direct).",
        {
            Make_Route(0, 0.80, 60.0, 0.40, 500.0),
            Make_Route(1, 0.80, 60.0, 0.40, 500.0),
            Make_Route(2, 1.00, 1.0, 1.00, 1.0, false),
            Make_Route(3, 1.00, 1.0, 1.00, 1.0, false)
        },
        {0, 0, 0, 0}
    });

    // --------------------------------------------------------
    // CASE 10: Score difference is just ABOVE tie tolerance.
    // Relay0 should replace direct for B1-B4.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_10_ABOVE_TIE_TOLERANCE",
        "Decision-boundary test: a score advantage > 1e-12 must be treated as a real improvement.",
        {
            Make_Route(0, 0.8000000000000, 60.0, 0.4000000000000, 500.0),
            Make_Route(1, 0.8000000000020, 60.0, 0.4000000000020, 500.0),
            Make_Route(2, 1.00, 1.0, 1.00, 1.0, false),
            Make_Route(3, 1.00, 1.0, 1.00, 1.0, false)
        },
        {1, 1, 1, 1}
    });

    // --------------------------------------------------------
    // CASE 11: Score difference is WITHIN tie tolerance.
    // It must be treated as a tie; direct wins on lower delay.
    // --------------------------------------------------------
    cases.push_back({
        "CASE_11_WITHIN_TIE_TOLERANCE",
        "Decision-boundary test: a score difference <= 1e-12 is a tie; direct wins lower-delay tie-break.",
        {
            Make_Route(0, 0.8000000000000, 60.0, 0.4000000000000, 400.0),
            Make_Route(1, 0.8000000000005, 60.0, 0.4000000000005, 500.0),
            Make_Route(2, 1.00, 1.0, 1.00, 1.0, false),
            Make_Route(3, 1.00, 1.0, 1.00, 1.0, false)
        },
        {0, 0, 0, 0}
    });

    return cases;
}

void Write_CSV_Header(std::ofstream& file)
{
    file
        << "Case_ID,Purpose,Policy,Expected_Route_Index,Expected_Route_Name,"
        << "Actual_Route_Index,Actual_Route_Name,Actual_Selected_Score,Pass,"
        << "Direct_Score,Relay0_Score,Relay1_Score,Relay2_Score,"
        << "Direct_Available,Relay0_Available,Relay1_Available,Relay2_Available,"
        << "Direct_Contact_P,Relay0_Contact_P,Relay1_Contact_P,Relay2_Contact_P,"
        << "Direct_PAT_s,Relay0_PAT_s,Relay1_PAT_s,Relay2_PAT_s,"
        << "Direct_Stability,Relay0_Stability,Relay1_Stability,Relay2_Stability,"
        << "Direct_Delay_s,Relay0_Delay_s,Relay1_Delay_s,Relay2_Delay_s\n";
}

std::string CSV_Escape(const std::string& text)
{
    std::string escaped = "\"";

    for (char ch : text)
    {
        if (ch == '"')
        {
            escaped += "\"\"";
        }
        else
        {
            escaped += ch;
        }
    }

    escaped += "\"";
    return escaped;
}

void Write_Result_Row(
    std::ofstream& file,
    const RQ4_Stress_Test_Case& test_case,
    Routing_Policy policy,
    int expected_index,
    const RQ4_Policy_Decision& decision,
    bool passed)
{
    file << std::setprecision(15);

    file
        << test_case.Case_ID << ","
        << CSV_Escape(test_case.Purpose) << ","
        << Policy_Name(policy) << ","
        << expected_index << ","
        << Route_Name_From_Index(expected_index) << ","
        << decision.Selected_Route_Index << ","
        << decision.Selected_Route_Name << ","
        << decision.Selected_Route_Score << ","
        << (passed ? 1 : 0) << ",";

    for (double score : decision.Route_Scores)
    {
        file << score << ",";
    }

    for (const auto& route : test_case.Routes)
    {
        file << (route.Route.Available && route.Contact_Available ? 1 : 0) << ",";
    }

    for (const auto& route : test_case.Routes)
    {
        file << route.Route_Contact_Realization_Probability << ",";
    }

    for (const auto& route : test_case.Routes)
    {
        file << route.PAT_Cost_Seconds << ",";
    }

    for (const auto& route : test_case.Routes)
    {
        file << route.Link_Stability << ",";
    }

    for (std::size_t i = 0; i < test_case.Routes.size(); ++i)
    {
        file << test_case.Routes[i].Propagation_Delay_Seconds;
        if (i + 1 < test_case.Routes.size()) file << ",";
    }

    file << "\n";
}

int main()
{
    std::cout
        << "\n====================================================\n"
        << "MONTE CARLO EXPERIMENT 4 - RQ4\n"
        << "PHASE 7A - POLICY-DISCRIMINATION STRESS TEST\n"
        << "====================================================\n\n"
        << "VALIDATION DATA ONLY - NOT MONTE CARLO RESEARCH DATA\n\n";

    const std::array<Routing_Policy, 4> policies = {
        Routing_Policy::B1,
        Routing_Policy::B2,
        Routing_Policy::B3,
        Routing_Policy::B4
    };

    const std::vector<RQ4_Stress_Test_Case> cases =
        Build_Stress_Test_Cases();

    const std::string Results_Folder =
        "./"
        "Monte_Carlo_Experiment 4/Phase_7A_Validation";

    std::filesystem::create_directories(Results_Folder);

    const std::string Results_File =
        Results_Folder +
        "/Phase_7A_Policy_Discrimination_Stress_Test.csv";

    const std::string Summary_File =
        Results_Folder +
        "/Phase_7A_Policy_Discrimination_Summary.csv";

    std::ofstream results(Results_File);

    if (!results)
    {
        std::cerr << "ERROR: Could not open results file:\n"
                  << Results_File << "\n";
        return 1;
    }

    Write_CSV_Header(results);

    int passed_decisions = 0;
    int total_decisions = 0;
    int passed_cases = 0;

    for (const RQ4_Stress_Test_Case& test_case : cases)
    {
        std::vector<RQ4_Route_State> route_states(
            test_case.Routes.begin(),
            test_case.Routes.end());

        bool case_passed = true;

        std::cout
            << test_case.Case_ID << "\n"
            << "  " << test_case.Purpose << "\n";

        for (std::size_t policy_index = 0;
             policy_index < policies.size();
             ++policy_index)
        {
            const Routing_Policy policy =
                policies[policy_index];

            const RQ4_Policy_Decision decision =
                Evaluate_Policy(policy, route_states);

            const int expected_index =
                test_case.Expected_Route_Index_By_Policy[
                    policy_index];

            const bool passed =
                decision.Selected_Route_Index == expected_index;

            ++total_decisions;

            if (passed)
            {
                ++passed_decisions;
            }
            else
            {
                case_passed = false;
            }

            Write_Result_Row(
                results,
                test_case,
                policy,
                expected_index,
                decision,
                passed);

            std::cout
                << "  " << Policy_Name(policy)
                << " | Expected: "
                << Route_Name_From_Index(expected_index)
                << " | Actual: "
                << decision.Selected_Route_Name
                << " | Score: "
                << std::setprecision(15)
                << decision.Selected_Route_Score
                << " | "
                << (passed ? "PASS" : "FAIL")
                << "\n";
        }

        if (case_passed)
        {
            ++passed_cases;
        }

        std::cout
            << "  CASE RESULT: "
            << (case_passed ? "PASS" : "FAIL")
            << "\n\n";
    }

    results.close();

    const bool overall_pass =
        passed_decisions == total_decisions &&
        passed_cases == static_cast<int>(cases.size());

    std::ofstream summary(Summary_File);

    if (!summary)
    {
        std::cerr << "ERROR: Could not open summary file:\n"
                  << Summary_File << "\n";
        return 1;
    }

    summary
        << "Metric,Value\n"
        << "Phase,7A\n"
        << "Test_Type,Deterministic policy-discrimination stress test\n"
        << "Research_Data,NO\n"
        << "Stress_Test_Cases," << cases.size() << "\n"
        << "Policies_Per_Case," << policies.size() << "\n"
        << "Total_Decisions," << total_decisions << "\n"
        << "Passed_Decisions," << passed_decisions << "\n"
        << "Failed_Decisions," << (total_decisions - passed_decisions) << "\n"
        << "Passed_Cases," << passed_cases << "\n"
        << "Failed_Cases," << (static_cast<int>(cases.size()) - passed_cases) << "\n"
        << "Score_Tie_Tolerance," << std::setprecision(15)
        << SCORE_TIE_TOLERANCE << "\n"
        << "Overall_Status," << (overall_pass ? "PASS" : "FAIL") << "\n";

    summary.close();

    std::cout
        << "====================================================\n"
        << "PHASE 7A VALIDATION SUMMARY\n"
        << "====================================================\n"
        << "Cases passed: " << passed_cases
        << " / " << cases.size() << "\n"
        << "Policy decisions passed: " << passed_decisions
        << " / " << total_decisions << "\n"
        << "OVERALL STATUS: "
        << (overall_pass ? "PASS" : "FAIL")
        << "\n\n"
        << "Results file:\n" << Results_File << "\n\n"
        << "Summary file:\n" << Summary_File << "\n\n";

    return overall_pass ? 0 : 1;
}
