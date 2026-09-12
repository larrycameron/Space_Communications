#ifndef INTERSTELLAR_COMMUNICATIONS_SIMULATOR_DISPLAY_H
#define INTERSTELLAR_COMMUNICATIONS_SIMULATOR_DISPLAY_H

enum class Simulator_Display
{
    MainMenu,

    KeplerPhysicsEngine,
    SpacecraftGeometry,
    ShapiroTimeDelay,

    SignalAnalysis,
    OpticalCommunications,
    SpaceCommunicationsLink,

    InterstellarNetwork,
    EncryptedMessageTransmission,

    RelayStation,
    RelayTopology,
    RelayRouting,

    ContactWindow,
    PATTiming,

    MonteCarloExperiment1,
    MonteCarloExperiment2,
    MonteCarloExperiment3,
    MonteCarloExperiment4,

    ResearchResults,

    Exit
};

class Interstellar_Communications_Simulator_Display
{
public:

    // Displays the main program menu.
    void Display_Main_Menu();

    // Displays the heading/menu associated with
    // the selected simulator section.
    void Display_Menu(Simulator_Display display_menu);

    // Displays the Monte Carlo experiment menu.
    void Display_Monte_Carlo_Menu();

    // Displays the networking/encryption menu.
    void Display_Network_Menu();

    // Displays the physics and geometry menu.
    void Display_Physics_Menu();

    // Displays the optical communications menu.
    void Display_Communications_Menu();

    // Displays research experiment/results options.
    void Display_Research_Menu();
};

#endif