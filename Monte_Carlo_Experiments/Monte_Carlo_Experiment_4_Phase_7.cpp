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

#include "Interstellar_Communications_Network.h"
#include "Kepler_Physics_Engine.h"
// ============================================================
// MONTE CARLO EXPERIMENT 4
// RQ4 - MATCHED ROUTING POLICY COMPARISON
//
// Phase 7 diagnostic:
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
// WRITE ONE TRIAL / POLICY ROW
// ============================================================

void Write_Trial_Row(
    std::ofstream& file,
    int trial,
    Routing_Policy policy,
    const RQ4_Shared_World& world,
    const std::vector<RQ4_Candidate_Route>& routes,
    const std::vector<RQ4_Route_State>& route_states,
    const RQ4_Policy_Decision& decision)
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

    file << "\n";
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
        << "PHASE 7 - B1-B4 POLICY SCORING AND ROUTE SELECTION\n";

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


    // ========================================================
    // OPEN OUTPUT FILES
    // ========================================================

    std::ofstream Trial_File(
        Trial_File_Name);


    std::ofstream Summary_File(
        Summary_File_Name);


    std::ofstream Matching_File(
        Matching_File_Name);


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
        << "Selected_Link_Stability\n";


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
        << "Selected_Link_Stability\n";


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
        // Phase 7: policy-specific scoring changes; the shared world does not.
        // ====================================================

        for (Routing_Policy policy :
             Policies)
        {
            const RQ4_Policy_Decision decision =
                Evaluate_Policy(
                    policy,
                    Route_States);

            std::cout
                << "    POLICY DECISION "
                << Policy_Name(policy)
                << " | Selected: "
                << decision.Selected_Route_Name
                << " | Score: "
                << decision.Selected_Route_Score
                << " | Route scores [D,R0,R1,R2]: ["
                << decision.Route_Scores[0]
                << ", "
                << decision.Route_Scores[1]
                << ", "
                << decision.Route_Scores[2]
                << ", "
                << decision.Route_Scores[3]
                << "]\n";

            Write_Trial_Row(
                Trial_File,
                trial,
                policy,
                world,
                Candidate_Routes,
                Route_States,
                decision);

            Write_Trial_Row(
                Matching_File,
                trial,
                policy,
                world,
                Candidate_Routes,
                Route_States,
                decision);
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
        << "Run_Number,"
        << "Number_Of_Trials,"
        << "Policies_Per_Trial,"
        << "Total_Policy_Rows,"
        << "First_Seed,"
        << "Last_Seed\n";


    Summary_File
        << Run_Number
        << ","

        << Number_Of_Debug_Trials
        << ","

        << Policies.size()
        << ","

        << Number_Of_Debug_Trials *
           static_cast<int>(
               Policies.size())
        << ","

        << Base_Seed
        << ","

        << Base_Seed +
           static_cast<std::uint32_t>(
               Number_Of_Debug_Trials - 1)

        << "\n";


    // ========================================================
    // CLOSE FILES
    // ========================================================

    Trial_File.close();

    Summary_File.close();

    Matching_File.close();


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
        << "For each Trial/Seed, "
        << "B1-B4 must have identical "
        << "shared-world values.\n";


    return 0;
}
