#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

#include "Interstellar_Communications_Network.h"
#include "Kepler_Physics_Engine.h"
// ============================================================
// MONTE CARLO EXPERIMENT 4
// RQ4 - MATCHED ROUTING POLICY COMPARISON
//
// Phase 9 diagnostic:
// Preserve the validated Phase 6 route-state physics, then introduce
// policy-specific B1-B4 scoring and actual route selection while preserving
// the matched-world design. No packet delivery outcomes or inferential
// statistics are calculated in Phase 7. This isolates routing decisions
// so the policy ladder can be validated before end-to-end simulation.
//
// This experiment is being developed for the RQ4
// peer-reviewed journal study.
// ============================================================

enum class Routing_Policy
{
    B1,
    B2,
    B3,
    B4
};

// ============================================================
// PHASE 5C ORBITAL + RELAY GEOMETRY
//
// JPL approximate planetary elements and rates, Table 1.
// EM Bary is used as the Earth-system proxy for this diagnostic.
// The trial orbital phase selects one point across an Earth-Mars
// synodic cycle. Earth and Mars are then propagated to the SAME
// shared epoch using their own JPL element rates. Phase 5C adds:
//   Relay 0: Mars-leading heliocentric relay, +18 deg mean longitude,
//            approximately +10 deg inclination relative to Mars baseline.
//   Relay 1: Mars-trailing heliocentric relay, -18 deg mean longitude,
//            approximately -10 deg inclination relative to Mars baseline.
//   Relay 2: simplified Sun-Mars L2 relay center proxy, placed
//            ~1,000,000 km anti-sunward of Mars along the Sun-Mars line.
//
// The lead/trail geometry is based on published JPL deep-space-relay
// architecture studies. Relay 2 is intentionally a diagnostic L2-center
// proxy, NOT a propagated CR3BP halo orbit.
// ============================================================

struct RQ4_Orbital_Elements
{
    double Semi_Major_Axis_AU{};
    double Eccentricity{};
    double Inclination_Degrees{};
    double Mean_Longitude_Degrees{};
    double Longitude_Perihelion_Degrees{};
    double Longitude_Ascending_Node_Degrees{};
};

struct RQ4_Orbital_Element_Rates
{
    double Semi_Major_Axis_AU_Per_Century{};
    double Eccentricity_Per_Century{};
    double Inclination_Degrees_Per_Century{};
    double Mean_Longitude_Degrees_Per_Century{};
    double Longitude_Perihelion_Degrees_Per_Century{};
    double Longitude_Ascending_Node_Degrees_Per_Century{};
};

struct RQ4_Planet_State
{
    Eigen::Vector3d Position_Kilometers{};
    double Mean_Anomaly_Radians{};
    double Eccentric_Anomaly_Radians{};
    double True_Anomaly_Radians{};
    double Orbital_Radius_Kilometers{};
};

static constexpr double RQ4_PI =
    3.14159265358979323846;

static constexpr double AU_IN_KILOMETERS =
    149597870.7;

// Mean Earth-Mars synodic period. Used only to map the existing
// 0-2pi trial phase onto a shared simulation epoch.
static constexpr double EARTH_MARS_SYNODIC_DAYS =
    779.94;

static constexpr double DAYS_PER_JULIAN_CENTURY =
    36525.0;

// Published deep-space relay architecture geometry uses Mars-leading
// and Mars-trailing relays separated from Mars by about 18 degrees and
// inclined approximately +/-10 degrees.
static constexpr double RELAY_LEAD_TRAIL_SEPARATION_DEGREES = 18.0;
static constexpr double RELAY_INCLINATION_OFFSET_DEGREES = 10.0;

// Diagnostic proxy for the center region of a Sun-Mars L2 halo relay.
// Published expanded-architecture studies describe typical Mars-to-halo
// distances on the order of 1 million km. Full halo propagation is deferred.
static constexpr double MARS_L2_PROXY_OFFSET_KILOMETERS = 1000000.0;

// ============================================================
// PHASE 6 ROUTE-STATE PHYSICS CONSTANTS
//
// These are diagnostic, normalized state-model constants. They are
// intentionally explicit so the plumbing can be validated before
// route scoring is introduced.
//
// Optical quality is a dimensionless relative index. It uses the
// inverse-square free-space scaling relationship and the trial-level
// optical-degradation factor. It is NOT yet a full received-power,
// photon-budget, atmospheric, or pointing-error link budget.
// ============================================================
static constexpr double REFERENCE_OPTICAL_DISTANCE_METERS = 5.46e10;


double Degrees_To_Radians(double degrees)
{
    return degrees * RQ4_PI / 180.0;
}

double Normalize_Radians(double angle)
{
    while (angle > RQ4_PI)
    {
        angle -= 2.0 * RQ4_PI;
    }

    while (angle < -RQ4_PI)
    {
        angle += 2.0 * RQ4_PI;
    }

    return angle;
}

RQ4_Orbital_Elements Advance_Orbital_Elements(
    const RQ4_Orbital_Elements& baseline,
    const RQ4_Orbital_Element_Rates& rates,
    double centuries_since_j2000)
{
    RQ4_Orbital_Elements advanced;

    advanced.Semi_Major_Axis_AU =
        baseline.Semi_Major_Axis_AU +
        rates.Semi_Major_Axis_AU_Per_Century * centuries_since_j2000;

    advanced.Eccentricity =
        baseline.Eccentricity +
        rates.Eccentricity_Per_Century * centuries_since_j2000;

    advanced.Inclination_Degrees =
        baseline.Inclination_Degrees +
        rates.Inclination_Degrees_Per_Century * centuries_since_j2000;

    advanced.Mean_Longitude_Degrees =
        baseline.Mean_Longitude_Degrees +
        rates.Mean_Longitude_Degrees_Per_Century * centuries_since_j2000;

    advanced.Longitude_Perihelion_Degrees =
        baseline.Longitude_Perihelion_Degrees +
        rates.Longitude_Perihelion_Degrees_Per_Century * centuries_since_j2000;

    advanced.Longitude_Ascending_Node_Degrees =
        baseline.Longitude_Ascending_Node_Degrees +
        rates.Longitude_Ascending_Node_Degrees_Per_Century * centuries_since_j2000;

    return advanced;
}

RQ4_Planet_State Build_Planet_State(
    Keplers_Physics_Engine& kepler,
    const RQ4_Orbital_Elements& elements)
{
    RQ4_Planet_State state;

    const double semi_major_axis_km =
        elements.Semi_Major_Axis_AU *
        AU_IN_KILOMETERS;

    const double mean_anomaly_degrees =
        elements.Mean_Longitude_Degrees -
        elements.Longitude_Perihelion_Degrees;

    const double argument_of_periapsis_degrees =
        elements.Longitude_Perihelion_Degrees -
        elements.Longitude_Ascending_Node_Degrees;

    state.Mean_Anomaly_Radians =
        Normalize_Radians(
            Degrees_To_Radians(
                mean_anomaly_degrees));

    state.Eccentric_Anomaly_Radians =
        kepler.Calculate_Eccentric_Anomaly(
            state.Mean_Anomaly_Radians,
            elements.Eccentricity);

    state.True_Anomaly_Radians =
        kepler.Calculate_True_Anomaly(
            state.Eccentric_Anomaly_Radians,
            elements.Eccentricity);

    state.Orbital_Radius_Kilometers =
        kepler.Calculate_Orbital_Radius(
            semi_major_axis_km,
            elements.Eccentricity,
            state.Eccentric_Anomaly_Radians);

    state.Position_Kilometers =
        kepler.Calculate_Cartesian_Position(
            state.Orbital_Radius_Kilometers,
            state.True_Anomaly_Radians,
            Degrees_To_Radians(
                elements.Inclination_Degrees),
            Degrees_To_Radians(
                elements.Longitude_Ascending_Node_Degrees),
            Degrees_To_Radians(
                argument_of_periapsis_degrees));

    return state;
}


// ============================================================
// SHARED MONTE CARLO WORLD
//
// This is generated ONCE for a seed.
//
// B1, B2, B3, and B4 must all receive the same world.
// ============================================================

struct RQ4_Shared_World
{
    std::uint32_t Seed{};

    double Orbital_Phase_Radians{};

    // Phase 5B shared dynamic epoch.
    double Simulation_Days_Since_J2000{};

    // Heliocentric geometry at the shared simulation epoch.
    Eigen::Vector3d Earth_Position_Kilometers{};
    Eigen::Vector3d Mars_Position_Kilometers{};

    // Phase 5C physically distinct relay positions.
    std::array<Eigen::Vector3d, 3> Relay_Position_Kilometers{};

    // Derived from the 3-D Earth/Mars position vectors.
    double Earth_Mars_Distance_Meters{};

    double Optical_Degradation_Factor{};

    std::array<bool, 3> Relay_Online{};

    double Contact_Realization_Probability{};

    int Generated_Packets{};

    std::vector<double> Packet_Release_Times_Seconds;

    double PAT_Delay_Seconds{};
};


// ============================================================
// RQ4 CANDIDATE ROUTE
//
// Phase 2 diagnostic representation only.
// These routes give B1-B4 legitimate alternatives to compare.
// Scientific route costs are added later.
// ============================================================

struct RQ4_Candidate_Route
{
    std::string Route_Name;
    std::vector<std::string> Nodes;
    bool Available{true};
};

struct RQ4_Route_State
{
    RQ4_Candidate_Route Route;

    int Hop_Count{};

    double Earth_To_Relay_Distance_Meters{};
    double Relay_To_Mars_Distance_Meters{};
    double Total_Route_Distance_Meters{};
    double Propagation_Delay_Seconds{};

    // Base contact probability is shared by the world. Route contact
    // probability is p^N for N required optical hops, using the Phase 6
    // diagnostic assumption of independent hop contact realization.
    double Base_Contact_Realization_Probability{};
    double Route_Contact_Realization_Probability{};

    // PAT burden is one acquisition/tracking event per optical hop.
    double PAT_Cost_Seconds{};

    // Relative 0-1 optical quality indices. For a relay route, the
    // route quality is the bottleneck (minimum) hop quality.
    double Hop_1_Optical_Quality{};
    double Hop_2_Optical_Quality{};
    double Predicted_Optical_Quality{};

    // Diagnostic 0-1 stability index combining route contact probability
    // and bottleneck optical quality. Offline relay routes are forced to 0.
    double Link_Stability{};

    bool Relay_Available{true};
    bool Contact_Available{true};
};

// ============================================================
// BUILD CANDIDATE ROUTES
//
// For now availability is driven only by the shared relay state.
// No policy-specific scoring is performed here.
// ============================================================

std::vector<RQ4_Candidate_Route> Build_Candidate_Routes(
    const RQ4_Shared_World& world)
{
    std::vector<RQ4_Candidate_Route> routes;

    routes.push_back(
        {
            "Direct_Earth_to_Mars",
            {"Earth", "Mars"},
            true
        });

    routes.push_back(
        {
            "Earth_Relay0_Mars",
            {"Earth", "Relay_0", "Mars"},
            world.Relay_Online[0]
        });

    routes.push_back(
        {
            "Earth_Relay1_Mars",
            {"Earth", "Relay_1", "Mars"},
            world.Relay_Online[1]
        });

    routes.push_back(
        {
            "Earth_Relay2_Mars",
            {"Earth", "Relay_2", "Mars"},
            world.Relay_Online[2]
        });

    return routes;
}


double Clamp_Zero_To_One(double value)
{
    if (value < 0.0)
    {
        return 0.0;
    }

    if (value > 1.0)
    {
        return 1.0;
    }

    return value;
}

double Calculate_Relative_Optical_Quality(
    double hop_distance_meters,
    double optical_degradation_factor)
{
    if (hop_distance_meters <= 0.0)
    {
        return 0.0;
    }

    const double distance_ratio =
        REFERENCE_OPTICAL_DISTANCE_METERS /
        hop_distance_meters;

    const double inverse_square_quality =
        distance_ratio * distance_ratio;

    return Clamp_Zero_To_One(
        optical_degradation_factor *
        inverse_square_quality);
}

std::vector<RQ4_Route_State> Build_Route_States(
    const RQ4_Shared_World& world,
    const std::vector<RQ4_Candidate_Route>& routes)
{
    std::vector<RQ4_Route_State> route_states;

    Interstellar_Communications_Network network;

    // --------------------------------------------------------
    // PHASE 6 ROUTE-STATE PHYSICS
    // --------------------------------------------------------
    // Phase 5C geometry is preserved. Phase 6 adds only derived
    // state variables. No new random draws are consumed here.
    // Therefore every B1-B4 policy still sees the same realized
    // world and the same route-state values.
    // --------------------------------------------------------

    for (std::size_t i = 0; i < routes.size(); ++i)
    {
        const RQ4_Candidate_Route& route = routes[i];

        RQ4_Route_State state;
        state.Route = route;
        state.Relay_Available = route.Available;

        if (i == 0)
        {
            // Direct Earth -> Mars is one optical hop.
            state.Hop_Count = 1;
            state.Total_Route_Distance_Meters =
                world.Earth_Mars_Distance_Meters;

            state.Hop_1_Optical_Quality =
                Calculate_Relative_Optical_Quality(
                    state.Total_Route_Distance_Meters,
                    world.Optical_Degradation_Factor);

            state.Hop_2_Optical_Quality = 0.0;
            state.Predicted_Optical_Quality =
                state.Hop_1_Optical_Quality;
        }
        else
        {
            // Earth -> Relay_i -> Mars is two optical hops.
            state.Hop_Count = 2;

            const std::size_t relay_index = i - 1;

            const Eigen::Vector3d Earth_To_Relay_Vector_Kilometers =
                world.Relay_Position_Kilometers[relay_index] -
                world.Earth_Position_Kilometers;

            const Eigen::Vector3d Relay_To_Mars_Vector_Kilometers =
                world.Mars_Position_Kilometers -
                world.Relay_Position_Kilometers[relay_index];

            state.Earth_To_Relay_Distance_Meters =
                Earth_To_Relay_Vector_Kilometers.norm() * 1000.0;

            state.Relay_To_Mars_Distance_Meters =
                Relay_To_Mars_Vector_Kilometers.norm() * 1000.0;

            state.Total_Route_Distance_Meters =
                state.Earth_To_Relay_Distance_Meters +
                state.Relay_To_Mars_Distance_Meters;

            state.Hop_1_Optical_Quality =
                Calculate_Relative_Optical_Quality(
                    state.Earth_To_Relay_Distance_Meters,
                    world.Optical_Degradation_Factor);

            state.Hop_2_Optical_Quality =
                Calculate_Relative_Optical_Quality(
                    state.Relay_To_Mars_Distance_Meters,
                    world.Optical_Degradation_Factor);

            state.Predicted_Optical_Quality =
                std::min(
                    state.Hop_1_Optical_Quality,
                    state.Hop_2_Optical_Quality);
        }

        state.Propagation_Delay_Seconds =
            network.Calculate_Propagation_Delay(
                state.Total_Route_Distance_Meters);

        state.Base_Contact_Realization_Probability =
            world.Contact_Realization_Probability;

        state.Route_Contact_Realization_Probability =
            std::pow(
                world.Contact_Realization_Probability,
                static_cast<double>(state.Hop_Count));

        // One PAT acquisition/tracking burden per optical hop.
        state.PAT_Cost_Seconds =
            world.PAT_Delay_Seconds *
            static_cast<double>(state.Hop_Count);

        // Phase 6 does not yet draw a separate packet-level contact event.
        // Contact_Available therefore represents predicted route-state
        // feasibility: the direct route is available; relay routes also
        // require the corresponding relay to be online.
        state.Contact_Available =
            state.Relay_Available;

        state.Link_Stability =
            state.Route_Contact_Realization_Probability *
            state.Predicted_Optical_Quality;

        if (!state.Relay_Available)
        {
            state.Link_Stability = 0.0;
        }

        state.Link_Stability =
            Clamp_Zero_To_One(
                state.Link_Stability);

        route_states.push_back(state);
    }

    return route_states;
}

// ============================================================
// POLICY NAME
// ============================================================

std::string Policy_Name(Routing_Policy policy)
{
    switch (policy)
    {
        case Routing_Policy::B1:
            return "B1";

        case Routing_Policy::B2:
            return "B2";

        case Routing_Policy::B3:
            return "B3";

        case Routing_Policy::B4:
            return "B4";
    }

    return "UNKNOWN";
}


// ============================================================
// PHASE 7 POLICY SCORING AND ROUTE SELECTION
//
// Higher score is better. Availability/contact feasibility is a
// hard gate for every policy. The policies then progressively use
// richer validated route-state information:
//
// B1 = route contact-realization probability.
//
// B2 = B1 contact information multiplied by normalized PAT
//      efficiency. A one-hop route has PAT efficiency 1.0 and a
//      two-hop route has 0.5 with the current 60 s/hop model.
//
// B3 = normalized PAT efficiency multiplied by Link_Stability.
//      Link_Stability was validated in Phase 6 as
//      route-contact-probability * predicted-optical-quality.
//      Because optical quality is derived from physical hop geometry,
//      B3 incorporates contact, PAT, geometry-sensitive optical state,
//      and stability without multiplying contact probability twice.
//
// B4 = B3 score multiplied by normalized propagation efficiency.
//      This jointly retains contact/PAT/optical/geometry/stability and
//      adds route propagation delay. Relay/contact availability remains
//      a hard feasibility gate.
//
// These are transparent diagnostic utility functions for validating
// the policy ladder. Phase 7 does not claim that these weights are
// optimized or that packet-level outcomes have been established.
// ============================================================

struct RQ4_Policy_Decision
{
    Routing_Policy Policy{};
    std::array<double, 4> Route_Scores{};
    int Selected_Route_Index{-1};
    std::string Selected_Route_Name{"NO_ROUTE"};
    double Selected_Route_Score{-1.0};
};

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

        if (minimum < 0.0 ||
            state.PAT_Cost_Seconds < minimum)
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
    if (!state.Route.Available ||
        !state.Contact_Available)
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
            return contact_score *
                   pat_efficiency;

        case Routing_Policy::B3:
            return pat_efficiency *
                   Clamp_Zero_To_One(
                       state.Link_Stability);

        case Routing_Policy::B4:
            return pat_efficiency *
                   Clamp_Zero_To_One(
                       state.Link_Stability) *
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
        Find_Minimum_Available_PAT_Cost(
            route_states);

    const double minimum_propagation_delay =
        Find_Minimum_Available_Propagation_Delay(
            route_states);

    static constexpr double SCORE_TIE_TOLERANCE = 1.0e-12;

    for (std::size_t i = 0;
         i < route_states.size();
         ++i)
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
                     score -
                     decision.Selected_Route_Score) <=
                 SCORE_TIE_TOLERANCE)
        {
            const RQ4_Route_State& current =
                route_states[static_cast<std::size_t>(
                    decision.Selected_Route_Index)];

            // Deterministic tie-breaker:
            // 1) lower propagation delay
            // 2) lower PAT cost
            // 3) lower route index
            if (route_states[i].Propagation_Delay_Seconds <
                current.Propagation_Delay_Seconds)
            {
                choose_route = true;
            }
            else if (route_states[i].Propagation_Delay_Seconds ==
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


bool Run_Phase_7_Policy_Self_Check()
{
    // Synthetic deterministic route states are used only to verify the
    // scoring/selection logic. They do not enter Monte Carlo results.
    // Expected behavior:
    // B1 -> Direct (highest contact probability)
    // B2 -> Direct (contact + PAT burden)
    // B3 -> Relay0 (stronger link stability offsets PAT burden)
    // B4 -> Direct (propagation penalty makes the long relay path inferior)

    std::vector<RQ4_Route_State> test_states(4);

    test_states[0].Route = {"Direct_Earth_to_Mars", {"Earth", "Mars"}, true};
    test_states[0].Route_Contact_Realization_Probability = 0.90;
    test_states[0].PAT_Cost_Seconds = 60.0;
    test_states[0].Link_Stability = 0.10;
    test_states[0].Propagation_Delay_Seconds = 500.0;
    test_states[0].Contact_Available = true;

    test_states[1].Route = {"Earth_Relay0_Mars", {"Earth", "Relay_0", "Mars"}, true};
    test_states[1].Route_Contact_Realization_Probability = 0.80;
    test_states[1].PAT_Cost_Seconds = 120.0;
    test_states[1].Link_Stability = 0.40;
    test_states[1].Propagation_Delay_Seconds = 1500.0;
    test_states[1].Contact_Available = true;

    // Remaining synthetic routes are deliberately unavailable so they
    // cannot affect the expected decisions.
    for (std::size_t i = 2; i < test_states.size(); ++i)
    {
        test_states[i].Route = {
            "Synthetic_Unavailable_" + std::to_string(i),
            {},
            false};
        test_states[i].Route_Contact_Realization_Probability = 1.0;
        test_states[i].PAT_Cost_Seconds = 1.0;
        test_states[i].Link_Stability = 1.0;
        test_states[i].Propagation_Delay_Seconds = 1.0;
        test_states[i].Contact_Available = false;
    }

    const RQ4_Policy_Decision b1 =
        Evaluate_Policy(Routing_Policy::B1, test_states);
    const RQ4_Policy_Decision b2 =
        Evaluate_Policy(Routing_Policy::B2, test_states);
    const RQ4_Policy_Decision b3 =
        Evaluate_Policy(Routing_Policy::B3, test_states);
    const RQ4_Policy_Decision b4 =
        Evaluate_Policy(Routing_Policy::B4, test_states);

    return
        b1.Selected_Route_Index == 0 &&
        b2.Selected_Route_Index == 0 &&
        b3.Selected_Route_Index == 1 &&
        b4.Selected_Route_Index == 0;
}


// ============================================================
// GENERATE SHARED MONTE CARLO WORLD
// ============================================================

RQ4_Shared_World Generate_Shared_World(
    std::uint32_t seed)
{
    std::mt19937 generator(seed);


    std::uniform_real_distribution<double>
        Orbital_Phase_Distribution(
            0.0,
            6.28318530717958647692);


    std::uniform_real_distribution<double>
        Optical_Degradation_Distribution(
            0.85,
            1.00);


    std::bernoulli_distribution
        Relay_Online_Distribution(
            0.90);


    std::uniform_real_distribution<double>
        Contact_Probability_Distribution(
            0.80,
            1.00);


    // Packet releases occur during the first
    // 12 hours of the 24-hour trial window.
    std::uniform_real_distribution<double>
        Packet_Release_Distribution(
            0.0,
            12.0 * 60.0 * 60.0);


    RQ4_Shared_World world;


    world.Seed = seed;


    world.Orbital_Phase_Radians =
        Orbital_Phase_Distribution(
            generator);


    // --------------------------------------------------------
    // Phase 5B: dynamic shared Earth/Mars geometry.
    //
    // The existing Orbital_Phase_Radians value is interpreted
    // as progress through one Earth-Mars synodic cycle. This
    // produces one shared epoch for the trial. Earth and Mars
    // are then advanced independently to that SAME epoch using
    // JPL Table 1 element rates. No additional random draw is
    // consumed, so the previously validated RNG sequence stays
    // unchanged.
    // --------------------------------------------------------

    world.Simulation_Days_Since_J2000 =
        (world.Orbital_Phase_Radians / (2.0 * RQ4_PI)) *
        EARTH_MARS_SYNODIC_DAYS;

    const double centuries_since_j2000 =
        world.Simulation_Days_Since_J2000 /
        DAYS_PER_JULIAN_CENTURY;

    Keplers_Physics_Engine kepler;

    const RQ4_Orbital_Elements Earth_Baseline{
        1.00000261, 0.01671123, -0.00001531,
        100.46457166, 102.93768193, 0.0
    };

    const RQ4_Orbital_Element_Rates Earth_Rates{
        0.00000562, -0.00004392, -0.01294668,
        35999.37244981, 0.32327364, 0.0
    };

    const RQ4_Orbital_Elements Mars_Baseline{
        1.52371034, 0.09339410, 1.84969142,
        -4.55343205, -23.94362959, 49.55953891
    };

    const RQ4_Orbital_Element_Rates Mars_Rates{
        0.00001847, 0.00007882, -0.00813131,
        19140.30268499, 0.44441088, -0.29257343
    };

    const RQ4_Orbital_Elements Earth_Elements =
        Advance_Orbital_Elements(
            Earth_Baseline,
            Earth_Rates,
            centuries_since_j2000);

    const RQ4_Orbital_Elements Mars_Elements =
        Advance_Orbital_Elements(
            Mars_Baseline,
            Mars_Rates,
            centuries_since_j2000);

    const RQ4_Planet_State Earth_State =
        Build_Planet_State(
            kepler,
            Earth_Elements);

    const RQ4_Planet_State Mars_State =
        Build_Planet_State(
            kepler,
            Mars_Elements);

    world.Earth_Position_Kilometers =
        Earth_State.Position_Kilometers;

    world.Mars_Position_Kilometers =
        Mars_State.Position_Kilometers;

    // --------------------------------------------------------
    // Phase 5C relay geometry.
    // --------------------------------------------------------
    // Relay 0 and Relay 1 use the Mars-level heliocentric
    // architecture described in JPL deep-space relay studies.
    // They share Mars' epoch-dependent orbital-element values,
    // but are offset in mean longitude and inclination.
    //
    // This is a parametric diagnostic approximation suitable for
    // route-geometry plumbing; it does not claim high-fidelity
    // station-kept trajectories.
    // --------------------------------------------------------

    RQ4_Orbital_Elements Relay0_Elements = Mars_Elements;
    Relay0_Elements.Mean_Longitude_Degrees +=
        RELAY_LEAD_TRAIL_SEPARATION_DEGREES;
    Relay0_Elements.Inclination_Degrees +=
        RELAY_INCLINATION_OFFSET_DEGREES;

    RQ4_Orbital_Elements Relay1_Elements = Mars_Elements;
    Relay1_Elements.Mean_Longitude_Degrees -=
        RELAY_LEAD_TRAIL_SEPARATION_DEGREES;
    Relay1_Elements.Inclination_Degrees -=
        RELAY_INCLINATION_OFFSET_DEGREES;

    const RQ4_Planet_State Relay0_State =
        Build_Planet_State(kepler, Relay0_Elements);

    const RQ4_Planet_State Relay1_State =
        Build_Planet_State(kepler, Relay1_Elements);

    world.Relay_Position_Kilometers[0] =
        Relay0_State.Position_Kilometers;

    world.Relay_Position_Kilometers[1] =
        Relay1_State.Position_Kilometers;

    // Relay 2 is a simplified proxy for the center region of a
    // Sun-Mars L2 halo relay: anti-sunward from Mars along the
    // instantaneous Sun->Mars radial direction. A true halo orbit
    // requires CR3BP/numerical propagation and is intentionally
    // deferred beyond this geometry-plumbing validation.
    const double Mars_Heliocentric_Range_Kilometers =
        world.Mars_Position_Kilometers.norm();

    Eigen::Vector3d Mars_Radial_Unit_Vector{0.0, 0.0, 0.0};

    if (Mars_Heliocentric_Range_Kilometers > 0.0)
    {
        Mars_Radial_Unit_Vector =
            world.Mars_Position_Kilometers /
            Mars_Heliocentric_Range_Kilometers;
    }

    world.Relay_Position_Kilometers[2] =
        world.Mars_Position_Kilometers +
        Mars_Radial_Unit_Vector *
            MARS_L2_PROXY_OFFSET_KILOMETERS;

    const Eigen::Vector3d Earth_Mars_Vector_Kilometers =
        world.Mars_Position_Kilometers -
        world.Earth_Position_Kilometers;

    world.Earth_Mars_Distance_Meters =
        Earth_Mars_Vector_Kilometers.norm() *
        1000.0;


    world.Optical_Degradation_Factor =
        Optical_Degradation_Distribution(
            generator);


    for (bool& relay_online :
         world.Relay_Online)
    {
        relay_online =
            Relay_Online_Distribution(
                generator);
    }


    world.Contact_Realization_Probability =
        Contact_Probability_Distribution(
            generator);


    world.Generated_Packets = 200;


    world.Packet_Release_Times_Seconds.reserve(
        static_cast<std::size_t>(
            world.Generated_Packets));


    for (int packet = 0;
         packet < world.Generated_Packets;
         ++packet)
    {
        world.Packet_Release_Times_Seconds.push_back(
            Packet_Release_Distribution(
                generator));
    }


    world.PAT_Delay_Seconds = 60.0;


    return world;
}



// ============================================================
// PHASE 9 - FULL TRIAL INTEGRATION
//
// IMPORTANT SCOPE:
// Phase 8 does NOT reinterpret Phase 6 Link_Stability as a calibrated
// packet-success probability. Link_Stability remains a diagnostic routing
// state used by B3/B4.
//
// Phase 8 validates the deterministic mechanics that occur AFTER a route
// has been selected:
//   1) selected-route feasibility
//   2) per-hop packet transmission time
//   3) PAT time
//   4) propagation time
//   5) end-to-end latency
//   6) arrival time / TTL deadline
//   7) mechanical delivered / expired / dropped outcome
//
// Stochastic link/contact failures and multi-packet trial statistics are
// intentionally deferred to the next isolated phase. This prevents the
// validated Phase 7 selector from being mixed with an unvalidated physical
// packet-loss probability model.
// ============================================================

static constexpr double PHASE_8_PACKET_SIZE_BITS = 8192.0;       // 1 KiB diagnostic packet
static constexpr double PHASE_8_DATA_RATE_BPS = 1.0e8;           // 100 Mbps diagnostic rate
static constexpr double PHASE_8_PACKET_TTL_SECONDS = 1500.0;     // diagnostic lifetime

struct RQ4_Packet_Outcome
{
    int Packet_Index{0};

    double Release_Time_Seconds{};
    double Packet_Size_Bits{};
    double Data_Rate_Bps{};

    double Transmission_Time_Per_Hop_Seconds{};
    double Total_Transmission_Time_Seconds{};

    double PAT_Time_Seconds{};
    double Propagation_Time_Seconds{};

    double End_To_End_Latency_Seconds{};
    double Arrival_Time_Seconds{};

    double TTL_Seconds{};
    double Deadline_Time_Seconds{};

    std::string Outcome{"UNINITIALIZED"};

    bool Delivered{false};
    bool Expired{false};
    bool Dropped{false};
};

RQ4_Packet_Outcome Evaluate_Packet_Outcome(
    int packet_index,
    double release_time_seconds,
    double packet_size_bits,
    double data_rate_bps,
    double ttl_seconds,
    const RQ4_Policy_Decision& decision,
    const std::vector<RQ4_Route_State>& route_states)
{
    RQ4_Packet_Outcome outcome;

    outcome.Packet_Index = packet_index;
    outcome.Release_Time_Seconds = release_time_seconds;
    outcome.Packet_Size_Bits = packet_size_bits;
    outcome.Data_Rate_Bps = data_rate_bps;
    outcome.TTL_Seconds = ttl_seconds;
    outcome.Deadline_Time_Seconds =
        release_time_seconds + ttl_seconds;

    if (decision.Selected_Route_Index < 0 ||
        static_cast<std::size_t>(
            decision.Selected_Route_Index) >= route_states.size())
    {
        outcome.Outcome = "DROPPED_NO_ROUTE";
        outcome.Dropped = true;
        return outcome;
    }

    const RQ4_Route_State& selected =
        route_states[static_cast<std::size_t>(
            decision.Selected_Route_Index)];

    if (!selected.Route.Available ||
        !selected.Relay_Available ||
        !selected.Contact_Available)
    {
        outcome.Outcome = "DROPPED_ROUTE_UNAVAILABLE";
        outcome.Dropped = true;
        return outcome;
    }

    if (packet_size_bits < 0.0 ||
        data_rate_bps <= 0.0 ||
        ttl_seconds < 0.0 ||
        selected.Hop_Count <= 0 ||
        selected.PAT_Cost_Seconds < 0.0 ||
        selected.Propagation_Delay_Seconds < 0.0)
    {
        outcome.Outcome = "DROPPED_INVALID_PACKET_STATE";
        outcome.Dropped = true;
        return outcome;
    }

    outcome.Transmission_Time_Per_Hop_Seconds =
        packet_size_bits / data_rate_bps;

    // Store-and-forward diagnostic assumption:
    // a complete packet must be transmitted once per optical hop.
    outcome.Total_Transmission_Time_Seconds =
        outcome.Transmission_Time_Per_Hop_Seconds *
        static_cast<double>(
            selected.Hop_Count);

    outcome.PAT_Time_Seconds =
        selected.PAT_Cost_Seconds;

    outcome.Propagation_Time_Seconds =
        selected.Propagation_Delay_Seconds;

    outcome.End_To_End_Latency_Seconds =
        outcome.PAT_Time_Seconds +
        outcome.Total_Transmission_Time_Seconds +
        outcome.Propagation_Time_Seconds;

    outcome.Arrival_Time_Seconds =
        outcome.Release_Time_Seconds +
        outcome.End_To_End_Latency_Seconds;

    static constexpr double TIME_TOLERANCE = 1.0e-12;

    if (outcome.Arrival_Time_Seconds >
        outcome.Deadline_Time_Seconds +
        TIME_TOLERANCE)
    {
        outcome.Outcome = "EXPIRED_BEFORE_ARRIVAL";
        outcome.Expired = true;
        return outcome;
    }

    outcome.Outcome = "DELIVERED_MECHANICALLY";
    outcome.Delivered = true;

    return outcome;
}



// ============================================================
// PHASE 9 - FULL TRIAL INTEGRATION
// ============================================================

static constexpr double PHASE_9_PACKET_SIZE_BITS = PHASE_8_PACKET_SIZE_BITS;
static constexpr double PHASE_9_DATA_RATE_BPS = PHASE_8_DATA_RATE_BPS;
static constexpr double PHASE_9_PACKET_TTL_SECONDS = PHASE_8_PACKET_TTL_SECONDS;
static constexpr double PHASE_9_TRAFFIC_WINDOW_SECONDS = 12.0 * 60.0 * 60.0;

struct RQ4_Phase9_Packet_Record
{
    RQ4_Packet_Outcome Packet;
    int Selected_Route_Index{-1};
    std::string Selected_Route_Name{"NO_ROUTE"};
    double Route_Contact_Probability{-1.0};
    double Shared_Contact_Uniform{-1.0};
    bool Contact_Realized{false};
};

struct RQ4_Phase9_Trial_Metrics
{
    int Generated_Packets{};
    int Delivered_Packets{};
    int Expired_Packets{};
    int Dropped_Packets{};
    int Contact_Failure_Drops{};
    int No_Route_Drops{};
    int Route_Unavailable_Drops{};
    int Invalid_State_Drops{};
    double Reliability{};
    double Delivered_Bits{};
    double Delivered_Throughput_Bps{};
    double Mean_Delivered_Latency_Seconds{-1.0};
    double Median_Delivered_Latency_Seconds{-1.0};
    double P95_Delivered_Latency_Seconds{-1.0};
    double Total_PAT_Burden_Seconds{};
    double Mean_PAT_Burden_Per_Generated_Packet_Seconds{};
    std::array<int,4> Route_Selection_Counts{{0,0,0,0}};
    int No_Route_Selection_Count{};
    int Route_Switch_Count{};
};

std::vector<std::array<double,4>> Generate_Shared_Contact_Uniforms(
    std::uint32_t world_seed, int packet_count)
{
    std::mt19937 rng(world_seed ^ 0x9E3779B9u);
    std::uniform_real_distribution<double> uniform_01(0.0,1.0);
    std::vector<std::array<double,4>> values;
    values.reserve(static_cast<std::size_t>(packet_count));
    for(int p=0;p<packet_count;++p)
    {
        std::array<double,4> row{};
        for(std::size_t r=0;r<4;++r) row[r]=uniform_01(rng);
        values.push_back(row);
    }
    return values;
}

RQ4_Phase9_Packet_Record Evaluate_Phase9_Packet(
    int packet_index,
    double release_time_seconds,
    const RQ4_Policy_Decision& decision,
    const std::vector<RQ4_Route_State>& route_states,
    const std::array<double,4>& shared_contact_uniforms)
{
    RQ4_Phase9_Packet_Record record;
    record.Selected_Route_Index=decision.Selected_Route_Index;
    record.Selected_Route_Name=decision.Selected_Route_Name;
    record.Packet=Evaluate_Packet_Outcome(
        packet_index,release_time_seconds,
        PHASE_9_PACKET_SIZE_BITS,PHASE_9_DATA_RATE_BPS,
        PHASE_9_PACKET_TTL_SECONDS,decision,route_states);

    if(decision.Selected_Route_Index<0 ||
       static_cast<std::size_t>(decision.Selected_Route_Index)>=route_states.size())
        return record;

    const std::size_t route_index=static_cast<std::size_t>(decision.Selected_Route_Index);
    const RQ4_Route_State& selected=route_states[route_index];
    record.Route_Contact_Probability=
        Clamp_Zero_To_One(selected.Route_Contact_Realization_Probability);
    record.Shared_Contact_Uniform=shared_contact_uniforms[route_index];

    if(!selected.Route.Available || !selected.Relay_Available || !selected.Contact_Available)
        return record;

    record.Contact_Realized=
        record.Shared_Contact_Uniform<=record.Route_Contact_Probability;

    if(!record.Contact_Realized)
    {
        record.Packet.Outcome="DROPPED_CONTACT_NOT_REALIZED";
        record.Packet.Delivered=false;
        record.Packet.Expired=false;
        record.Packet.Dropped=true;
    }
    return record;
}

double Calculate_Percentile(std::vector<double> values,double percentile)
{
    if(values.empty()) return -1.0;
    std::sort(values.begin(),values.end());
    if(values.size()==1) return values.front();
    const double p=std::max(0.0,std::min(1.0,percentile));
    const double pos=p*static_cast<double>(values.size()-1);
    const std::size_t lo=static_cast<std::size_t>(std::floor(pos));
    const std::size_t hi=static_cast<std::size_t>(std::ceil(pos));
    if(lo==hi) return values[lo];
    const double f=pos-static_cast<double>(lo);
    return values[lo]+f*(values[hi]-values[lo]);
}

RQ4_Phase9_Trial_Metrics Calculate_Phase9_Trial_Metrics(
    const std::vector<RQ4_Phase9_Packet_Record>& records)
{
    RQ4_Phase9_Trial_Metrics m;
    m.Generated_Packets=static_cast<int>(records.size());
    std::vector<double> latencies;
    int prev=-2; bool have_prev=false;
    for(const auto& record:records)
    {
        const auto& p=record.Packet;
        if(record.Selected_Route_Index>=0 && record.Selected_Route_Index<4)
            ++m.Route_Selection_Counts[static_cast<std::size_t>(record.Selected_Route_Index)];
        else ++m.No_Route_Selection_Count;

        if(have_prev && record.Selected_Route_Index!=prev) ++m.Route_Switch_Count;
        prev=record.Selected_Route_Index; have_prev=true;

        if(p.Delivered)
        {
            ++m.Delivered_Packets;
            m.Delivered_Bits+=p.Packet_Size_Bits;
            latencies.push_back(p.End_To_End_Latency_Seconds);
        }
        if(p.Expired) ++m.Expired_Packets;
        if(p.Dropped)
        {
            ++m.Dropped_Packets;
            if(p.Outcome=="DROPPED_CONTACT_NOT_REALIZED") ++m.Contact_Failure_Drops;
            else if(p.Outcome=="DROPPED_NO_ROUTE") ++m.No_Route_Drops;
            else if(p.Outcome=="DROPPED_ROUTE_UNAVAILABLE") ++m.Route_Unavailable_Drops;
            else if(p.Outcome=="DROPPED_INVALID_PACKET_STATE") ++m.Invalid_State_Drops;
        }
        if(record.Selected_Route_Index>=0) m.Total_PAT_Burden_Seconds+=p.PAT_Time_Seconds;
    }
    if(m.Generated_Packets>0)
    {
        m.Reliability=static_cast<double>(m.Delivered_Packets)/m.Generated_Packets;
        m.Mean_PAT_Burden_Per_Generated_Packet_Seconds=
            m.Total_PAT_Burden_Seconds/m.Generated_Packets;
    }
    m.Delivered_Throughput_Bps=m.Delivered_Bits/PHASE_9_TRAFFIC_WINDOW_SECONDS;
    if(!latencies.empty())
    {
        double total=0.0; for(double x:latencies) total+=x;
        m.Mean_Delivered_Latency_Seconds=total/latencies.size();
        m.Median_Delivered_Latency_Seconds=Calculate_Percentile(latencies,0.50);
        m.P95_Delivered_Latency_Seconds=Calculate_Percentile(latencies,0.95);
    }
    return m;
}

std::vector<RQ4_Phase9_Packet_Record> Evaluate_Phase9_Full_Trial(
    const RQ4_Shared_World& world,
    const RQ4_Policy_Decision& decision,
    const std::vector<RQ4_Route_State>& route_states,
    const std::vector<std::array<double,4>>& shared_contact_uniforms)
{
    std::vector<RQ4_Phase9_Packet_Record> records;
    const std::size_t n=std::min(world.Packet_Release_Times_Seconds.size(),
                                 shared_contact_uniforms.size());
    records.reserve(n);
    for(std::size_t i=0;i<n;++i)
        records.push_back(Evaluate_Phase9_Packet(
            static_cast<int>(i),world.Packet_Release_Times_Seconds[i],
            decision,route_states,shared_contact_uniforms[i]));
    return records;
}

bool Run_Phase_9_Full_Trial_Self_Check()
{
    bool ok=true;
    auto check=[&ok](const std::string& name,bool pass){
        std::cout<<"    "<<name<<": "<<(pass?"PASS":"FAIL")<<"\n";
        if(!pass) ok=false;
    };
    std::cout<<"Phase 9 full-trial integration self-check:\n";

    std::vector<RQ4_Route_State> states(1);
    states[0].Route.Route_Name="Direct_Earth_to_Mars";
    states[0].Route.Available=true;
    states[0].Relay_Available=true;
    states[0].Contact_Available=true;
    states[0].Hop_Count=1;
    states[0].PAT_Cost_Seconds=60.0;
    states[0].Propagation_Delay_Seconds=300.0;
    states[0].Route_Contact_Realization_Probability=0.50;

    RQ4_Policy_Decision d;
    d.Selected_Route_Index=0;
    d.Selected_Route_Name="Direct_Earth_to_Mars";

    std::vector<RQ4_Phase9_Packet_Record> r;
    std::array<double,4> u0{{0.10,0,0,0}},u1{{0.90,0,0,0}},
                         u2{{0.20,0,0,0}},u3{{0.80,0,0,0}};
    r.push_back(Evaluate_Phase9_Packet(0,0,d,states,u0));
    r.push_back(Evaluate_Phase9_Packet(1,10,d,states,u1));
    r.push_back(Evaluate_Phase9_Packet(2,20,d,states,u2));
    r.push_back(Evaluate_Phase9_Packet(3,30,d,states,u3));
    const auto m=Calculate_Phase9_Trial_Metrics(r);

    check("TEST 1 - shared contact gate",
          r[0].Packet.Delivered && r[2].Packet.Delivered &&
          r[1].Packet.Outcome=="DROPPED_CONTACT_NOT_REALIZED" &&
          r[3].Packet.Outcome=="DROPPED_CONTACT_NOT_REALIZED");
    check("TEST 2 - reliability aggregation",
          m.Generated_Packets==4 && m.Delivered_Packets==2 &&
          m.Dropped_Packets==2 && m.Contact_Failure_Drops==2 &&
          std::abs(m.Reliability-0.5)<=1e-12);
    check("TEST 3 - delivered throughput",
          std::abs(m.Delivered_Throughput_Bps-
          ((2.0*PHASE_9_PACKET_SIZE_BITS)/PHASE_9_TRAFFIC_WINDOW_SECONDS))<=1e-12);
    check("TEST 4 - latency aggregation",
          m.Mean_Delivered_Latency_Seconds>0 &&
          m.Median_Delivered_Latency_Seconds>0 &&
          m.P95_Delivered_Latency_Seconds>0);
    check("TEST 5 - route-frequency aggregation",
          m.Route_Selection_Counts[0]==4 && m.Route_Switch_Count==0);

    const auto a=Generate_Shared_Contact_Uniforms(40000u,10);
    const auto b=Generate_Shared_Contact_Uniforms(40000u,10);
    const auto c=Generate_Shared_Contact_Uniforms(40001u,10);
    bool same=true,diff=false;
    for(std::size_t i=0;i<a.size();++i) for(std::size_t j=0;j<4;++j)
    { if(a[i][j]!=b[i][j]) same=false; if(a[i][j]!=c[i][j]) diff=true; }
    check("TEST 6 - deterministic shared contact stream",same&&diff);

    std::cout<<"Phase 9 full-trial integration self-check: "
             <<(ok?"PASS":"FAIL")<<"\n\n";
    return ok;
}


bool Nearly_Equal(
    double a,
    double b,
    double tolerance = 1.0e-12)
{
    return std::abs(a - b) <= tolerance;
}


bool Run_Phase_8_Packet_Mechanics_Self_Check()
{
    bool all_passed = true;

    auto Print_Result =
        [&all_passed](
            const std::string& name,
            bool passed)
        {
            std::cout
                << "    "
                << name
                << ": "
                << (passed ? "PASS" : "FAIL")
                << "\n";

            if (!passed)
            {
                all_passed = false;
            }
        };

    std::cout
        << "Phase 8 deterministic packet-mechanics self-check:\n";

    // --------------------------------------------------------
    // TEST 1: Direct route delivered.
    // Expected latency:
    // PAT 60 + transmission 8192/1e8 + propagation 300.
    // --------------------------------------------------------
    {
        std::vector<RQ4_Route_State> states(1);

        states[0].Route.Route_Name = "Direct_Earth_to_Mars";
        states[0].Route.Available = true;
        states[0].Relay_Available = true;
        states[0].Contact_Available = true;
        states[0].Hop_Count = 1;
        states[0].PAT_Cost_Seconds = 60.0;
        states[0].Propagation_Delay_Seconds = 300.0;

        RQ4_Policy_Decision decision;
        decision.Selected_Route_Index = 0;
        decision.Selected_Route_Name = "Direct_Earth_to_Mars";

        const RQ4_Packet_Outcome result =
            Evaluate_Packet_Outcome(
                0,
                100.0,
                8192.0,
                1.0e8,
                1000.0,
                decision,
                states);

        const double expected_tx =
            8192.0 / 1.0e8;

        const double expected_latency =
            60.0 + expected_tx + 300.0;

        Print_Result(
            "TEST 1 - direct delivered",
            result.Delivered &&
            !result.Expired &&
            !result.Dropped &&
            result.Outcome == "DELIVERED_MECHANICALLY" &&
            Nearly_Equal(
                result.End_To_End_Latency_Seconds,
                expected_latency));
    }

    // --------------------------------------------------------
    // TEST 2: Two-hop relay doubles packet transmission time.
    // --------------------------------------------------------
    {
        std::vector<RQ4_Route_State> states(1);

        states[0].Route.Route_Name = "Earth_Relay0_Mars";
        states[0].Route.Available = true;
        states[0].Relay_Available = true;
        states[0].Contact_Available = true;
        states[0].Hop_Count = 2;
        states[0].PAT_Cost_Seconds = 120.0;
        states[0].Propagation_Delay_Seconds = 400.0;

        RQ4_Policy_Decision decision;
        decision.Selected_Route_Index = 0;
        decision.Selected_Route_Name = "Earth_Relay0_Mars";

        const RQ4_Packet_Outcome result =
            Evaluate_Packet_Outcome(
                0,
                0.0,
                8192.0,
                1.0e8,
                1000.0,
                decision,
                states);

        const double expected_tx =
            2.0 * (8192.0 / 1.0e8);

        Print_Result(
            "TEST 2 - relay hop transmission scaling",
            result.Delivered &&
            Nearly_Equal(
                result.Total_Transmission_Time_Seconds,
                expected_tx));
    }

    // --------------------------------------------------------
    // TEST 3: TTL expiration is detected.
    // --------------------------------------------------------
    {
        std::vector<RQ4_Route_State> states(1);

        states[0].Route.Route_Name = "Direct_Earth_to_Mars";
        states[0].Route.Available = true;
        states[0].Relay_Available = true;
        states[0].Contact_Available = true;
        states[0].Hop_Count = 1;
        states[0].PAT_Cost_Seconds = 60.0;
        states[0].Propagation_Delay_Seconds = 500.0;

        RQ4_Policy_Decision decision;
        decision.Selected_Route_Index = 0;

        const RQ4_Packet_Outcome result =
            Evaluate_Packet_Outcome(
                0,
                50.0,
                8192.0,
                1.0e8,
                100.0,
                decision,
                states);

        Print_Result(
            "TEST 3 - TTL expiration",
            !result.Delivered &&
            result.Expired &&
            !result.Dropped &&
            result.Outcome == "EXPIRED_BEFORE_ARRIVAL");
    }

    // --------------------------------------------------------
    // TEST 4: No selected route causes a drop.
    // --------------------------------------------------------
    {
        std::vector<RQ4_Route_State> states(1);

        RQ4_Policy_Decision decision;
        decision.Selected_Route_Index = -1;

        const RQ4_Packet_Outcome result =
            Evaluate_Packet_Outcome(
                0,
                0.0,
                8192.0,
                1.0e8,
                1000.0,
                decision,
                states);

        Print_Result(
            "TEST 4 - no-route drop",
            result.Dropped &&
            !result.Delivered &&
            !result.Expired &&
            result.Outcome == "DROPPED_NO_ROUTE");
    }

    // --------------------------------------------------------
    // TEST 5: Selected but unavailable route causes a drop.
    // --------------------------------------------------------
    {
        std::vector<RQ4_Route_State> states(1);

        states[0].Route.Route_Name = "Earth_Relay0_Mars";
        states[0].Route.Available = false;
        states[0].Relay_Available = false;
        states[0].Contact_Available = false;
        states[0].Hop_Count = 2;

        RQ4_Policy_Decision decision;
        decision.Selected_Route_Index = 0;

        const RQ4_Packet_Outcome result =
            Evaluate_Packet_Outcome(
                0,
                0.0,
                8192.0,
                1.0e8,
                1000.0,
                decision,
                states);

        Print_Result(
            "TEST 5 - unavailable-route drop",
            result.Dropped &&
            result.Outcome == "DROPPED_ROUTE_UNAVAILABLE");
    }

    // --------------------------------------------------------
    // TEST 6: Invalid data rate is rejected.
    // --------------------------------------------------------
    {
        std::vector<RQ4_Route_State> states(1);

        states[0].Route.Route_Name = "Direct_Earth_to_Mars";
        states[0].Route.Available = true;
        states[0].Relay_Available = true;
        states[0].Contact_Available = true;
        states[0].Hop_Count = 1;
        states[0].PAT_Cost_Seconds = 60.0;
        states[0].Propagation_Delay_Seconds = 300.0;

        RQ4_Policy_Decision decision;
        decision.Selected_Route_Index = 0;

        const RQ4_Packet_Outcome result =
            Evaluate_Packet_Outcome(
                0,
                0.0,
                8192.0,
                0.0,
                1000.0,
                decision,
                states);

        Print_Result(
            "TEST 6 - invalid transmission state",
            result.Dropped &&
            result.Outcome == "DROPPED_INVALID_PACKET_STATE");
    }

    std::cout
        << "Phase 8 deterministic packet-mechanics self-check: "
        << (all_passed ? "PASS" : "FAIL")
        << "\n\n";

    return all_passed;
}


// ============================================================
// WRITE ONE TRIAL / POLICY ROW
// ============================================================

void Write_Trial_Row(
    std::ofstream& file,
    int trial,
    Routing_Policy policy,
    const RQ4_Shared_World& world,
    const std::vector<RQ4_Candidate_Route>& routes,
    const std::vector<RQ4_Route_State>& route_states,
    const RQ4_Policy_Decision& decision,
    const RQ4_Packet_Outcome& packet_outcome,
    const RQ4_Phase9_Trial_Metrics& phase9_metrics)
{
    file
        << trial
        << ","

        << world.Seed
        << ","

        << Policy_Name(policy)
        << ","

        << std::setprecision(17)

        << world.Orbital_Phase_Radians
        << ","

        << world.Simulation_Days_Since_J2000
        << ","

        << world.Earth_Position_Kilometers.x()
        << ","

        << world.Earth_Position_Kilometers.y()
        << ","

        << world.Earth_Position_Kilometers.z()
        << ","

        << world.Mars_Position_Kilometers.x()
        << ","

        << world.Mars_Position_Kilometers.y()
        << ","

        << world.Mars_Position_Kilometers.z()
        << ","

        << world.Relay_Position_Kilometers[0].x()
        << ","
        << world.Relay_Position_Kilometers[0].y()
        << ","
        << world.Relay_Position_Kilometers[0].z()
        << ","

        << world.Relay_Position_Kilometers[1].x()
        << ","
        << world.Relay_Position_Kilometers[1].y()
        << ","
        << world.Relay_Position_Kilometers[1].z()
        << ","

        << world.Relay_Position_Kilometers[2].x()
        << ","
        << world.Relay_Position_Kilometers[2].y()
        << ","
        << world.Relay_Position_Kilometers[2].z()
        << ","

        << world.Earth_Mars_Distance_Meters
        << ","

        << world.Optical_Degradation_Factor
        << ","

        << (world.Relay_Online[0] ? 1 : 0)
        << ","

        << (world.Relay_Online[1] ? 1 : 0)
        << ","

        << (world.Relay_Online[2] ? 1 : 0)
        << ","

        << world.Contact_Realization_Probability
        << ","

        << world.Generated_Packets
        << ","

        << world.PAT_Delay_Seconds
        << ","

        << world.Packet_Release_Times_Seconds.front()
        << ","

        << world.Packet_Release_Times_Seconds.back()
        << ","

        << (routes[0].Available ? 1 : 0)
        << ","

        << (routes[1].Available ? 1 : 0)
        << ","

        << (routes[2].Available ? 1 : 0)
        << ","

        << (routes[3].Available ? 1 : 0)
        << ",";

        int available_route_count = 0;

        for (const RQ4_Candidate_Route& route : routes)
        {
            if (route.Available)
            {
                ++available_route_count;
            }
        }

file << available_route_count
     << ","
     << route_states[1].Earth_To_Relay_Distance_Meters
     << ","
     << route_states[1].Relay_To_Mars_Distance_Meters
     << ","
     << route_states[2].Earth_To_Relay_Distance_Meters
     << ","
     << route_states[2].Relay_To_Mars_Distance_Meters
     << ","
     << route_states[3].Earth_To_Relay_Distance_Meters
     << ","
     << route_states[3].Relay_To_Mars_Distance_Meters
     << ","
     << route_states[0].Total_Route_Distance_Meters
     << ","
     << route_states[1].Total_Route_Distance_Meters
     << ","
     << route_states[2].Total_Route_Distance_Meters
     << ","
     << route_states[3].Total_Route_Distance_Meters
     << ","
     << route_states[0].Propagation_Delay_Seconds
     << ","
     << route_states[1].Propagation_Delay_Seconds
     << ","
     << route_states[2].Propagation_Delay_Seconds
     << ","
     << route_states[3].Propagation_Delay_Seconds
     << ","
     << route_states[0].Hop_Count
     << ","
     << route_states[1].Hop_Count
     << ","
     << route_states[2].Hop_Count
     << ","
     << route_states[3].Hop_Count
     << ","
     << route_states[0].Route_Contact_Realization_Probability
     << ","
     << route_states[1].Route_Contact_Realization_Probability
     << ","
     << route_states[2].Route_Contact_Realization_Probability
     << ","
     << route_states[3].Route_Contact_Realization_Probability
     << ","
     << route_states[0].PAT_Cost_Seconds
     << ","
     << route_states[1].PAT_Cost_Seconds
     << ","
     << route_states[2].PAT_Cost_Seconds
     << ","
     << route_states[3].PAT_Cost_Seconds
     << ","
     << route_states[0].Hop_1_Optical_Quality
     << ","
     << route_states[1].Hop_1_Optical_Quality
     << ","
     << route_states[1].Hop_2_Optical_Quality
     << ","
     << route_states[2].Hop_1_Optical_Quality
     << ","
     << route_states[2].Hop_2_Optical_Quality
     << ","
     << route_states[3].Hop_1_Optical_Quality
     << ","
     << route_states[3].Hop_2_Optical_Quality
     << ","
     << route_states[0].Predicted_Optical_Quality
     << ","
     << route_states[1].Predicted_Optical_Quality
     << ","
     << route_states[2].Predicted_Optical_Quality
     << ","
     << route_states[3].Predicted_Optical_Quality
     << ","
     << route_states[0].Link_Stability
     << ","
     << route_states[1].Link_Stability
     << ","
     << route_states[2].Link_Stability
     << ","
     << route_states[3].Link_Stability
     << ","
     << (route_states[0].Contact_Available ? 1 : 0)
     << ","
     << (route_states[1].Contact_Available ? 1 : 0)
     << ","
     << (route_states[2].Contact_Available ? 1 : 0)
     << ","
     << (route_states[3].Contact_Available ? 1 : 0)
     << ","
     << decision.Selected_Route_Index
     << ","
     << decision.Selected_Route_Name
     << ","
     << decision.Selected_Route_Score
     << ","
     << decision.Route_Scores[0]
     << ","
     << decision.Route_Scores[1]
     << ","
     << decision.Route_Scores[2]
     << ","
     << decision.Route_Scores[3]
     << ",";

    if (decision.Selected_Route_Index >= 0)
    {
        const RQ4_Route_State& selected =
            route_states[static_cast<std::size_t>(
                decision.Selected_Route_Index)];

        file
            << selected.Propagation_Delay_Seconds
            << ","
            << selected.PAT_Cost_Seconds
            << ","
            << selected.Predicted_Optical_Quality
            << ","
            << selected.Link_Stability;
    }
    else
    {
        file << "-1,-1,-1,-1";
    }

    file
        << ","
        << packet_outcome.Packet_Index
        << ","
        << packet_outcome.Release_Time_Seconds
        << ","
        << packet_outcome.Packet_Size_Bits
        << ","
        << packet_outcome.Data_Rate_Bps
        << ","
        << packet_outcome.Transmission_Time_Per_Hop_Seconds
        << ","
        << packet_outcome.Total_Transmission_Time_Seconds
        << ","
        << packet_outcome.PAT_Time_Seconds
        << ","
        << packet_outcome.Propagation_Time_Seconds
        << ","
        << packet_outcome.End_To_End_Latency_Seconds
        << ","
        << packet_outcome.Arrival_Time_Seconds
        << ","
        << packet_outcome.TTL_Seconds
        << ","
        << packet_outcome.Deadline_Time_Seconds
        << ","
        << packet_outcome.Outcome
        << ","
        << (packet_outcome.Delivered ? 1 : 0)
        << ","
        << (packet_outcome.Expired ? 1 : 0)
        << ","
        << (packet_outcome.Dropped ? 1 : 0)
        << "," << phase9_metrics.Generated_Packets
        << "," << phase9_metrics.Delivered_Packets
        << "," << phase9_metrics.Expired_Packets
        << "," << phase9_metrics.Dropped_Packets
        << "," << phase9_metrics.Contact_Failure_Drops
        << "," << phase9_metrics.No_Route_Drops
        << "," << phase9_metrics.Route_Unavailable_Drops
        << "," << phase9_metrics.Invalid_State_Drops
        << "," << phase9_metrics.Reliability
        << "," << phase9_metrics.Delivered_Bits
        << "," << phase9_metrics.Delivered_Throughput_Bps
        << "," << phase9_metrics.Mean_Delivered_Latency_Seconds
        << "," << phase9_metrics.Median_Delivered_Latency_Seconds
        << "," << phase9_metrics.P95_Delivered_Latency_Seconds
        << "," << phase9_metrics.Total_PAT_Burden_Seconds
        << "," << phase9_metrics.Mean_PAT_Burden_Per_Generated_Packet_Seconds
        << "," << phase9_metrics.Route_Selection_Counts[0]
        << "," << phase9_metrics.Route_Selection_Counts[1]
        << "," << phase9_metrics.Route_Selection_Counts[2]
        << "," << phase9_metrics.Route_Selection_Counts[3]
        << "," << phase9_metrics.No_Route_Selection_Count
        << "," << phase9_metrics.Route_Switch_Count
        << "\n";
}


// ============================================================
// GET NEXT RUN NUMBER
//
// Example:
//
// Trial_1_Monte_Carlo_Experiment_4.csv
// Trial_2_Monte_Carlo_Experiment_4.csv
//
// If Trial_2 exists, next run becomes 3.
// ============================================================

int Get_Next_Run_Number(
    const std::string& results_folder)
{
    int highest_run_number = 0;


    std::regex trial_pattern(
        R"(Trial_(\d+)_Monte_Carlo_Experiment_4\.csv)");


    for (const auto& entry :
         std::filesystem::directory_iterator(
             results_folder))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }


        std::string filename =
            entry.path()
                .filename()
                .string();


        std::smatch match;


        if (std::regex_match(
                filename,
                match,
                trial_pattern))
        {
            int run_number =
                std::stoi(
                    match[1].str());


            if (run_number >
                highest_run_number)
            {
                highest_run_number =
                    run_number;
            }
        }
    }


    return highest_run_number + 1;
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    std::cout
        << "\n====================================================\n";

    std::cout
        << "MONTE CARLO EXPERIMENT 4 - RQ4\n";

    std::cout
        << "PHASE 9 - FULL TRIAL INTEGRATION\n";

    std::cout
        << "====================================================\n\n";

    const bool Policy_Self_Check_Passed =
        Run_Phase_7_Policy_Self_Check();

    std::cout
        << "Phase 7 deterministic policy self-check: "
        << (Policy_Self_Check_Passed ? "PASS" : "FAIL")
        << "\n\n";

    if (!Policy_Self_Check_Passed)
    {
        std::cerr
            << "ERROR: Phase 7 policy selection self-check failed.\n";
        return 1;
    }


    const bool Packet_Mechanics_Self_Check_Passed =
        Run_Phase_8_Packet_Mechanics_Self_Check();

    if (!Packet_Mechanics_Self_Check_Passed)
    {
        std::cerr
            << "ERROR: Phase 8 packet mechanics self-check failed.\n";
        return 1;
    }

    const bool Full_Trial_Self_Check_Passed =
        Run_Phase_9_Full_Trial_Self_Check();

    if (!Full_Trial_Self_Check_Passed)
    {
        std::cerr
            << "ERROR: Phase 9 full-trial integration self-check failed.\n";
        return 1;
    }


    // ========================================================
    // EXPERIMENT SETTINGS
    // ========================================================

    // Start small for debugging.
    const int Number_Of_Debug_Trials = 5;


    const std::uint32_t Base_Seed =
        40000;


    const std::array<Routing_Policy, 4>
        Policies =
    {
        Routing_Policy::B1,
        Routing_Policy::B2,
        Routing_Policy::B3,
        Routing_Policy::B4
    };


    // ========================================================
    // RESULTS FOLDER
    // ========================================================

    const std::string Results_Folder =
        "./"
        "Monte_Carlo_Experiment 4";


    std::filesystem::create_directories(
        Results_Folder);


    // ========================================================
    // DETERMINE RUN NUMBER
    // ========================================================

    const int Run_Number =
        Get_Next_Run_Number(
            Results_Folder);


    // ========================================================
    // BUILD FILE NAMES
    // ========================================================

    const std::string Trial_File_Name =
        Results_Folder +
        "/Trial_" +
        std::to_string(
            Run_Number) +
        "_Monte_Carlo_Experiment_4.csv";


    const std::string Summary_File_Name =
        Results_Folder +
        "/Summary_" +
        std::to_string(
            Run_Number) +
        "_Monte_Carlo_Experiment_4.csv";


    const std::string Matching_File_Name =
        Results_Folder +
        "/RQ4_Matching_Check.csv";

    const std::string Packet_File_Name =
        Results_Folder +
        "/Packet_" +
        std::to_string(Run_Number) +
        "_Monte_Carlo_Experiment_4.csv";


    // ========================================================
    // OPEN OUTPUT FILES
    // ========================================================

    std::ofstream Trial_File(
        Trial_File_Name);


    std::ofstream Summary_File(
        Summary_File_Name);


    std::ofstream Matching_File(
        Matching_File_Name);

    std::ofstream Packet_File(
        Packet_File_Name);


    // ========================================================
    // VERIFY FILES OPENED
    // ========================================================

    if (!Trial_File.is_open())
    {
        std::cerr
            << "ERROR: Could not create "
            << "RQ4 trial CSV.\n";

        return 1;
    }


    if (!Summary_File.is_open())
    {
        std::cerr
            << "ERROR: Could not create "
            << "RQ4 summary CSV.\n";

        return 1;
    }


    if (!Matching_File.is_open())
    {
        std::cerr
            << "ERROR: Could not create "
            << "RQ4 matching CSV.\n";

        return 1;
    }

    if (!Packet_File.is_open())
    {
        std::cerr << "ERROR: Could not create Phase 9 packet-level CSV.\n";
        return 1;
    }


    // ========================================================
    // TRIAL CSV HEADER
    // ========================================================

    Trial_File
        << "Trial,"
        << "Seed,"
        << "Policy,"
        << "Orbital_Phase_Radians,"
        << "Simulation_Days_Since_J2000,"
        << "Earth_X_km,"
        << "Earth_Y_km,"
        << "Earth_Z_km,"
        << "Mars_X_km,"
        << "Mars_Y_km,"
        << "Mars_Z_km,"
        << "Relay0_X_km,"
        << "Relay0_Y_km,"
        << "Relay0_Z_km,"
        << "Relay1_X_km,"
        << "Relay1_Y_km,"
        << "Relay1_Z_km,"
        << "Relay2_X_km,"
        << "Relay2_Y_km,"
        << "Relay2_Z_km,"
        << "Earth_Mars_Distance_Meters,"
        << "Optical_Degradation_Factor,"
        << "Relay_0_Online,"
        << "Relay_1_Online,"
        << "Relay_2_Online,"
        << "Contact_Realization_Probability,"
        << "Generated_Packets,"
        << "PAT_Delay_Seconds,"
        << "First_Packet_Release_s,"
        << "Last_Packet_Release_s,"
        << "Direct_Available,"
        << "Relay0_Route_Available,"
        << "Relay1_Route_Available,"
        << "Relay2_Route_Available,"
        << "Available_Route_Count,"
        << "Relay0_Earth_Hop_Distance_m,"
        << "Relay0_Mars_Hop_Distance_m,"
        << "Relay1_Earth_Hop_Distance_m,"
        << "Relay1_Mars_Hop_Distance_m,"
        << "Relay2_Earth_Hop_Distance_m,"
        << "Relay2_Mars_Hop_Distance_m,"
        << "Direct_Total_Route_Distance_m,"
        << "Relay0_Total_Route_Distance_m,"
        << "Relay1_Total_Route_Distance_m,"
        << "Relay2_Total_Route_Distance_m,"
        << "Direct_Propagation_Delay_s,"
        << "Relay0_Propagation_Delay_s,"
        << "Relay1_Propagation_Delay_s,"
        << "Relay2_Propagation_Delay_s,"
        << "Direct_Hop_Count,"
        << "Relay0_Hop_Count,"
        << "Relay1_Hop_Count,"
        << "Relay2_Hop_Count,"
        << "Direct_Route_Contact_Probability,"
        << "Relay0_Route_Contact_Probability,"
        << "Relay1_Route_Contact_Probability,"
        << "Relay2_Route_Contact_Probability,"
        << "Direct_PAT_Cost_s,"
        << "Relay0_PAT_Cost_s,"
        << "Relay1_PAT_Cost_s,"
        << "Relay2_PAT_Cost_s,"
        << "Direct_Hop1_Optical_Quality,"
        << "Relay0_Hop1_Optical_Quality,"
        << "Relay0_Hop2_Optical_Quality,"
        << "Relay1_Hop1_Optical_Quality,"
        << "Relay1_Hop2_Optical_Quality,"
        << "Relay2_Hop1_Optical_Quality,"
        << "Relay2_Hop2_Optical_Quality,"
        << "Direct_Predicted_Optical_Quality,"
        << "Relay0_Predicted_Optical_Quality,"
        << "Relay1_Predicted_Optical_Quality,"
        << "Relay2_Predicted_Optical_Quality,"
        << "Direct_Link_Stability,"
        << "Relay0_Link_Stability,"
        << "Relay1_Link_Stability,"
        << "Relay2_Link_Stability,"
        << "Direct_Contact_Available,"
        << "Relay0_Contact_Available,"
        << "Relay1_Contact_Available,"
        << "Relay2_Contact_Available,"
        << "Selected_Route_Index,"
        << "Selected_Route_Name,"
        << "Selected_Route_Score,"
        << "Direct_Policy_Score,"
        << "Relay0_Policy_Score,"
        << "Relay1_Policy_Score,"
        << "Relay2_Policy_Score,"
        << "Selected_Propagation_Delay_s,"
        << "Selected_PAT_Cost_s,"
        << "Selected_Optical_Quality,"
        << "Selected_Link_Stability,"
        << "Phase8_Packet_Index,"
        << "Phase8_Packet_Release_s,"
        << "Phase8_Packet_Size_bits,"
        << "Phase8_Data_Rate_bps,"
        << "Phase8_Transmission_Time_Per_Hop_s,"
        << "Phase8_Total_Transmission_Time_s,"
        << "Phase8_PAT_Time_s,"
        << "Phase8_Propagation_Time_s,"
        << "Phase8_End_To_End_Latency_s,"
        << "Phase8_Arrival_Time_s,"
        << "Phase8_TTL_s,"
        << "Phase8_Deadline_s,"
        << "Phase8_Outcome,"
        << "Phase8_Delivered,"
        << "Phase8_Expired,"
        << "Phase8_Dropped,"
        << "Phase9_Generated_Packets,"
        << "Phase9_Delivered_Packets,"
        << "Phase9_Expired_Packets,"
        << "Phase9_Dropped_Packets,"
        << "Phase9_Contact_Failure_Drops,"
        << "Phase9_No_Route_Drops,"
        << "Phase9_Route_Unavailable_Drops,"
        << "Phase9_Invalid_State_Drops,"
        << "Phase9_Reliability,"
        << "Phase9_Delivered_Bits,"
        << "Phase9_Delivered_Throughput_bps,"
        << "Phase9_Mean_Delivered_Latency_s,"
        << "Phase9_Median_Delivered_Latency_s,"
        << "Phase9_P95_Delivered_Latency_s,"
        << "Phase9_Total_PAT_Burden_s,"
        << "Phase9_Mean_PAT_Per_Generated_Packet_s,"
        << "Phase9_Direct_Selection_Count,"
        << "Phase9_Relay0_Selection_Count,"
        << "Phase9_Relay1_Selection_Count,"
        << "Phase9_Relay2_Selection_Count,"
        << "Phase9_No_Route_Selection_Count,"
        << "Phase9_Route_Switch_Count\n";


    // ========================================================
    // MATCHING CHECK CSV HEADER
    // ========================================================

    Matching_File
        << "Trial,"
        << "Seed,"
        << "Policy,"
        << "Orbital_Phase_Radians,"
        << "Simulation_Days_Since_J2000,"
        << "Earth_X_km,"
        << "Earth_Y_km,"
        << "Earth_Z_km,"
        << "Mars_X_km,"
        << "Mars_Y_km,"
        << "Mars_Z_km,"
        << "Relay0_X_km,"
        << "Relay0_Y_km,"
        << "Relay0_Z_km,"
        << "Relay1_X_km,"
        << "Relay1_Y_km,"
        << "Relay1_Z_km,"
        << "Relay2_X_km,"
        << "Relay2_Y_km,"
        << "Relay2_Z_km,"
        << "Earth_Mars_Distance_Meters,"
        << "Optical_Degradation_Factor,"
        << "Relay_0_Online,"
        << "Relay_1_Online,"
        << "Relay_2_Online,"
        << "Contact_Realization_Probability,"
        << "Generated_Packets,"
        << "PAT_Delay_Seconds,"
        << "First_Packet_Release_s,"
        << "Last_Packet_Release_s,"
        << "Direct_Available,"
        << "Relay0_Route_Available,"
        << "Relay1_Route_Available,"
        << "Relay2_Route_Available,"
        << "Available_Route_Count,"
        << "Relay0_Earth_Hop_Distance_m,"
        << "Relay0_Mars_Hop_Distance_m,"
        << "Relay1_Earth_Hop_Distance_m,"
        << "Relay1_Mars_Hop_Distance_m,"
        << "Relay2_Earth_Hop_Distance_m,"
        << "Relay2_Mars_Hop_Distance_m,"
        << "Direct_Total_Route_Distance_m,"
        << "Relay0_Total_Route_Distance_m,"
        << "Relay1_Total_Route_Distance_m,"
        << "Relay2_Total_Route_Distance_m,"
        << "Direct_Propagation_Delay_s,"
        << "Relay0_Propagation_Delay_s,"
        << "Relay1_Propagation_Delay_s,"
        << "Relay2_Propagation_Delay_s,"
        << "Direct_Hop_Count,"
        << "Relay0_Hop_Count,"
        << "Relay1_Hop_Count,"
        << "Relay2_Hop_Count,"
        << "Direct_Route_Contact_Probability,"
        << "Relay0_Route_Contact_Probability,"
        << "Relay1_Route_Contact_Probability,"
        << "Relay2_Route_Contact_Probability,"
        << "Direct_PAT_Cost_s,"
        << "Relay0_PAT_Cost_s,"
        << "Relay1_PAT_Cost_s,"
        << "Relay2_PAT_Cost_s,"
        << "Direct_Hop1_Optical_Quality,"
        << "Relay0_Hop1_Optical_Quality,"
        << "Relay0_Hop2_Optical_Quality,"
        << "Relay1_Hop1_Optical_Quality,"
        << "Relay1_Hop2_Optical_Quality,"
        << "Relay2_Hop1_Optical_Quality,"
        << "Relay2_Hop2_Optical_Quality,"
        << "Direct_Predicted_Optical_Quality,"
        << "Relay0_Predicted_Optical_Quality,"
        << "Relay1_Predicted_Optical_Quality,"
        << "Relay2_Predicted_Optical_Quality,"
        << "Direct_Link_Stability,"
        << "Relay0_Link_Stability,"
        << "Relay1_Link_Stability,"
        << "Relay2_Link_Stability,"
        << "Direct_Contact_Available,"
        << "Relay0_Contact_Available,"
        << "Relay1_Contact_Available,"
        << "Relay2_Contact_Available,"
        << "Selected_Route_Index,"
        << "Selected_Route_Name,"
        << "Selected_Route_Score,"
        << "Direct_Policy_Score,"
        << "Relay0_Policy_Score,"
        << "Relay1_Policy_Score,"
        << "Relay2_Policy_Score,"
        << "Selected_Propagation_Delay_s,"
        << "Selected_PAT_Cost_s,"
        << "Selected_Optical_Quality,"
        << "Selected_Link_Stability,"
        << "Phase8_Packet_Index,"
        << "Phase8_Packet_Release_s,"
        << "Phase8_Packet_Size_bits,"
        << "Phase8_Data_Rate_bps,"
        << "Phase8_Transmission_Time_Per_Hop_s,"
        << "Phase8_Total_Transmission_Time_s,"
        << "Phase8_PAT_Time_s,"
        << "Phase8_Propagation_Time_s,"
        << "Phase8_End_To_End_Latency_s,"
        << "Phase8_Arrival_Time_s,"
        << "Phase8_TTL_s,"
        << "Phase8_Deadline_s,"
        << "Phase8_Outcome,"
        << "Phase8_Delivered,"
        << "Phase8_Expired,"
        << "Phase8_Dropped,"
        << "Phase9_Generated_Packets,"
        << "Phase9_Delivered_Packets,"
        << "Phase9_Expired_Packets,"
        << "Phase9_Dropped_Packets,"
        << "Phase9_Contact_Failure_Drops,"
        << "Phase9_No_Route_Drops,"
        << "Phase9_Route_Unavailable_Drops,"
        << "Phase9_Invalid_State_Drops,"
        << "Phase9_Reliability,"
        << "Phase9_Delivered_Bits,"
        << "Phase9_Delivered_Throughput_bps,"
        << "Phase9_Mean_Delivered_Latency_s,"
        << "Phase9_Median_Delivered_Latency_s,"
        << "Phase9_P95_Delivered_Latency_s,"
        << "Phase9_Total_PAT_Burden_s,"
        << "Phase9_Mean_PAT_Per_Generated_Packet_s,"
        << "Phase9_Direct_Selection_Count,"
        << "Phase9_Relay0_Selection_Count,"
        << "Phase9_Relay1_Selection_Count,"
        << "Phase9_Relay2_Selection_Count,"
        << "Phase9_No_Route_Selection_Count,"
        << "Phase9_Route_Switch_Count\n";


    Packet_File
        << "Trial,Seed,Policy,Packet_Index,Release_Time_s,"
        << "Selected_Route_Index,Selected_Route_Name,"
        << "Route_Contact_Probability,Shared_Contact_Uniform,Contact_Realized,"
        << "Outcome,Delivered,Expired,Dropped,"
        << "Transmission_Time_Per_Hop_s,Total_Transmission_Time_s,"
        << "PAT_Time_s,Propagation_Time_s,End_To_End_Latency_s,"
        << "Arrival_Time_s,TTL_s,Deadline_s\n";


    // ========================================================
    // MONTE CARLO TRIAL LOOP
    // ========================================================

    for (int trial = 1;
         trial <= Number_Of_Debug_Trials;
         ++trial)
    {
        const std::uint32_t seed =
            Base_Seed +
            static_cast<std::uint32_t>(
                trial - 1);


        // ====================================================
        // Generate the realized Monte Carlo world ONCE.
        // ====================================================

        const RQ4_Shared_World world =
            Generate_Shared_World(
                seed);


        const std::vector<RQ4_Candidate_Route> Candidate_Routes =
            Build_Candidate_Routes(
                world);

        const std::vector<RQ4_Route_State> Route_States =
            Build_Route_States(
                world,
                Candidate_Routes);

        const std::vector<std::array<double, 4>> Shared_Contact_Uniforms =
            Generate_Shared_Contact_Uniforms(
                seed,
                world.Generated_Packets);

        std::cout
            << "    Shared epoch days since J2000: "
            << world.Simulation_Days_Since_J2000
            << "\n";

        std::cout
            << "    EARTH DYNAMIC (km): ["
            << world.Earth_Position_Kilometers.x()
            << ", "
            << world.Earth_Position_Kilometers.y()
            << ", "
            << world.Earth_Position_Kilometers.z()
            << "]\n";

        std::cout
            << "    MARS DYNAMIC (km):  ["
            << world.Mars_Position_Kilometers.x()
            << ", "
            << world.Mars_Position_Kilometers.y()
            << ", "
            << world.Mars_Position_Kilometers.z()
            << "]\n";

        for (std::size_t relay_index = 0;
             relay_index < world.Relay_Position_Kilometers.size();
             ++relay_index)
        {
            std::cout
                << "    RELAY " << relay_index << " (km): ["
                << world.Relay_Position_Kilometers[relay_index].x()
                << ", "
                << world.Relay_Position_Kilometers[relay_index].y()
                << ", "
                << world.Relay_Position_Kilometers[relay_index].z()
                << "]\n";
        }

        std::cout
            << "    3-D Earth-Mars Distance (m): "
            << world.Earth_Mars_Distance_Meters
            << "\n";

        for (const RQ4_Route_State& state : Route_States)
        {
            std::cout
                << "    ROUTE STATE: "
                << state.Route.Route_Name
                << " | Earth->Relay (m): "
                << state.Earth_To_Relay_Distance_Meters
                << " | Relay->Mars (m): "
                << state.Relay_To_Mars_Distance_Meters
                << " | Total Distance (m): "
                << state.Total_Route_Distance_Meters
                << " | Propagation Delay (s): "
                << state.Propagation_Delay_Seconds
                << " | Route Contact P: "
                << state.Route_Contact_Realization_Probability
                << " | PAT Cost (s): "
                << state.PAT_Cost_Seconds
                << " | Optical Quality: "
                << state.Predicted_Optical_Quality
                << " | Stability: "
                << state.Link_Stability
                << " | Relay Available: "
                << (state.Relay_Available ? "YES" : "NO")
                << "\n";
        }

        int Available_Route_Count = 0;

        for (const RQ4_Candidate_Route& route :
             Candidate_Routes)
        {
            if (route.Available)
            {
                ++Available_Route_Count;
            }
        }


        std::cout
            << "Trial "
            << trial

            << " | Seed "
            << seed

            << " | Orbital phase "
            << world.Orbital_Phase_Radians

            << " | Optical factor "
            << world.Optical_Degradation_Factor

            << " | Available routes "
            << Available_Route_Count

            << "\n";


        for (const RQ4_Candidate_Route& route :
             Candidate_Routes)
        {
            std::cout
                << "    "
                << route.Route_Name
                << " : "
                << (route.Available ? "AVAILABLE" : "UNAVAILABLE")
                << "\n";
        }


        // ====================================================
        // Every routing policy receives the SAME world.
        //
        // Phase 8: Phase 7 policy scoring is preserved; packet mechanics are evaluated only after route selection.
        // ====================================================

        for (Routing_Policy policy :
             Policies)
        {
            const RQ4_Policy_Decision decision =
                Evaluate_Policy(
                    policy,
                    Route_States);

            const std::vector<RQ4_Phase9_Packet_Record> phase9_packets =
                Evaluate_Phase9_Full_Trial(
                    world,
                    decision,
                    Route_States,
                    Shared_Contact_Uniforms);

            const RQ4_Phase9_Trial_Metrics phase9_metrics =
                Calculate_Phase9_Trial_Metrics(
                    phase9_packets);

            const RQ4_Packet_Outcome packet_outcome =
                phase9_packets.empty()
                    ? Evaluate_Packet_Outcome(
                        0,0.0,PHASE_9_PACKET_SIZE_BITS,
                        PHASE_9_DATA_RATE_BPS,PHASE_9_PACKET_TTL_SECONDS,
                        decision,Route_States)
                    : phase9_packets.front().Packet;

            std::cout
                << "    POLICY DECISION "
                << Policy_Name(policy)
                << " | Selected: "
                << decision.Selected_Route_Name
                << " | Score: "
                << decision.Selected_Route_Score
                << " | Route scores [D,R0,R1,R2]: ["
                << decision.Route_Scores[0] << ", "
                << decision.Route_Scores[1] << ", "
                << decision.Route_Scores[2] << ", "
                << decision.Route_Scores[3] << "]\n";

            std::cout
                << "        PHASE 9 FULL TRIAL"
                << " | Generated: " << phase9_metrics.Generated_Packets
                << " | Delivered: " << phase9_metrics.Delivered_Packets
                << " | Expired: " << phase9_metrics.Expired_Packets
                << " | Dropped: " << phase9_metrics.Dropped_Packets
                << " | Contact drops: " << phase9_metrics.Contact_Failure_Drops
                << " | Reliability: " << phase9_metrics.Reliability
                << " | Throughput (bps): " << phase9_metrics.Delivered_Throughput_Bps
                << " | Median latency (s): " << phase9_metrics.Median_Delivered_Latency_Seconds
                << " | P95 latency (s): " << phase9_metrics.P95_Delivered_Latency_Seconds
                << "\n";

            for(const RQ4_Phase9_Packet_Record& record:phase9_packets)
            {
                Packet_File
                    << trial << "," << seed << "," << Policy_Name(policy) << ","
                    << record.Packet.Packet_Index << ","
                    << std::setprecision(17)
                    << record.Packet.Release_Time_Seconds << ","
                    << record.Selected_Route_Index << ","
                    << record.Selected_Route_Name << ","
                    << record.Route_Contact_Probability << ","
                    << record.Shared_Contact_Uniform << ","
                    << (record.Contact_Realized?1:0) << ","
                    << record.Packet.Outcome << ","
                    << (record.Packet.Delivered?1:0) << ","
                    << (record.Packet.Expired?1:0) << ","
                    << (record.Packet.Dropped?1:0) << ","
                    << record.Packet.Transmission_Time_Per_Hop_Seconds << ","
                    << record.Packet.Total_Transmission_Time_Seconds << ","
                    << record.Packet.PAT_Time_Seconds << ","
                    << record.Packet.Propagation_Time_Seconds << ","
                    << record.Packet.End_To_End_Latency_Seconds << ","
                    << record.Packet.Arrival_Time_Seconds << ","
                    << record.Packet.TTL_Seconds << ","
                    << record.Packet.Deadline_Time_Seconds << "\n";
            }

            Write_Trial_Row(
                Trial_File,trial,policy,world,Candidate_Routes,Route_States,
                decision,packet_outcome,phase9_metrics);

            Write_Trial_Row(
                Matching_File,trial,policy,world,Candidate_Routes,Route_States,
                decision,packet_outcome,phase9_metrics);
        }
    }


    // ========================================================
    // SUMMARY CSV
    //
    // This is currently a run-level verification summary.
    //
    // Later outcome-analysis phases will contain:
    //
    // reliability
    // throughput
    // latency
    // paired differences
    // confidence intervals
    // effect sizes
    // policy comparisons
    // ========================================================

    Summary_File
        << "Run_Number,Number_Of_Trials,Policies_Per_Trial,Total_Policy_Rows,"
        << "First_Seed,Last_Seed,Phase7_Policy_Self_Check,"
        << "Phase8_Packet_Mechanics_Self_Check,Phase9_Full_Trial_Self_Check,"
        << "Phase9_Packet_Size_bits,Phase9_Data_Rate_bps,Phase9_TTL_s,"
        << "Phase9_Packets_Evaluated_Per_Policy,Phase9_Contact_Stream_Design,"
        << "Phase9_Link_Stability_As_Success_Probability\n";

    Summary_File
        << Run_Number << ","
        << Number_Of_Debug_Trials << ","
        << Policies.size() << ","
        << Number_Of_Debug_Trials * static_cast<int>(Policies.size()) << ","
        << Base_Seed << ","
        << Base_Seed + static_cast<std::uint32_t>(Number_Of_Debug_Trials-1) << ","
        << (Policy_Self_Check_Passed?"PASS":"FAIL") << ","
        << (Packet_Mechanics_Self_Check_Passed?"PASS":"FAIL") << ","
        << (Full_Trial_Self_Check_Passed?"PASS":"FAIL") << ","
        << PHASE_9_PACKET_SIZE_BITS << ","
        << PHASE_9_DATA_RATE_BPS << ","
        << PHASE_9_PACKET_TTL_SECONDS << ","
        << 200 << ","
        << "SHARED_PER_PACKET_PER_ROUTE_CRN,"
        << "NO\n";


    // ========================================================
    // CLOSE FILES
    // ========================================================

    Trial_File.close();

    Summary_File.close();

    Matching_File.close();

    Packet_File.close();


    // ========================================================
    // COMPLETION OUTPUT
    // ========================================================

    std::cout
        << "\n====================================================\n";

    std::cout
        << "RQ4 RUN "
        << Run_Number
        << " COMPLETE\n";

    std::cout
        << "====================================================\n\n";


    std::cout
        << "Trial file:\n"
        << Trial_File_Name
        << "\n\n";


    std::cout
        << "Summary file:\n"
        << Summary_File_Name
        << "\n\n";


    std::cout
        << "Matching diagnostic:\n"
        << Matching_File_Name
        << "\n\n";

    std::cout
        << "Packet-level audit file:\n"
        << Packet_File_Name
        << "\n\n";


    std::cout
        << "For each Trial/Seed, "
        << "B1-B4 must retain identical shared-world and route-state inputs.\n";

    std::cout
        << "Phase 9 packet outcomes may differ because policies select "
        << "different routes.\n";

    std::cout
        << "NOTE: Phase 9 uses shared per-packet/per-route contact realizations; "
        << "Link_Stability is NOT used as a packet-success probability.\n";


    return 0;
}
