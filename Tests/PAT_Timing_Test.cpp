#include "PAT_Timing.h"

#include <cmath>
#include <iostream>
#include <string>

bool Nearly_Equal(double a, double b, double tolerance = 1e-9)
{
    return std::fabs(a - b) <= tolerance;
}

void Print_Test_Result(const std::string& test_name, bool passed)
{
    std::cout << test_name << ": "
              << (passed ? "PASS" : "FAIL")
              << '\n';
}

int main()
{
    int passed_tests = 0;
    const int total_tests = 19;

    // =========================================================
    // PAT-001 — Normal total PAT time
    // =========================================================

    PAT_Timing pat1;

    double total_pat =
        pat1.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);

    bool test1 = Nearly_Equal(total_pat, 20.0);

    Print_Test_Result("PAT-001 Normal Total PAT Time", test1);

    if (test1)
        passed_tests++;

    // =========================================================
    // PAT-002 — Zero PAT time
    // =========================================================

    PAT_Timing pat2;

    total_pat =
        pat2.Calculate_Total_PAT_Time(0.0, 0.0, 0.0);

    bool test2 = Nearly_Equal(total_pat, 0.0);

    Print_Test_Result("PAT-002 Zero PAT Time", test2);

    if (test2)
        passed_tests++;

    // =========================================================
    // PAT-003 — Negative acquisition time
    // =========================================================

    PAT_Timing pat3;

    total_pat =
        pat3.Calculate_Total_PAT_Time(-1.0, 5.0, 5.0);

    bool test3 = Nearly_Equal(total_pat, 0.0);

    Print_Test_Result("PAT-003 Negative Acquisition Rejected", test3);

    if (test3)
        passed_tests++;

    // =========================================================
    // PAT-004 — Negative reacquisition time
    // =========================================================

    PAT_Timing pat4;

    total_pat =
        pat4.Calculate_Total_PAT_Time(10.0, -1.0, 5.0);

    bool test4 = Nearly_Equal(total_pat, 0.0);

    Print_Test_Result("PAT-004 Negative Reacquisition Rejected", test4);

    if (test4)
        passed_tests++;

    // =========================================================
    // PAT-005 — Negative retargeting time
    // =========================================================

    PAT_Timing pat5;

    total_pat =
        pat5.Calculate_Total_PAT_Time(10.0, 5.0, -1.0);

    bool test5 = Nearly_Equal(total_pat, 0.0);

    Print_Test_Result("PAT-005 Negative Retargeting Rejected", test5);

    if (test5)
        passed_tests++;

    // =========================================================
    // PAT-006 — Normal usable contact time
    // =========================================================

    PAT_Timing pat6;

    pat6.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);

    double usable =
        pat6.Calculate_Usable_Contact_Time(100.0);

    bool test6 = Nearly_Equal(usable, 80.0);

    Print_Test_Result("PAT-006 Normal Usable Contact Time", test6);

    if (test6)
        passed_tests++;

    // =========================================================
    // PAT-007 — PAT equals contact window
    // =========================================================

    PAT_Timing pat7;

    pat7.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);

    usable =
        pat7.Calculate_Usable_Contact_Time(20.0);

    bool test7 = Nearly_Equal(usable, 0.0);

    Print_Test_Result("PAT-007 PAT Equals Contact Window", test7);

    if (test7)
        passed_tests++;

    // =========================================================
    // PAT-008 — PAT exceeds contact window
    // =========================================================

    PAT_Timing pat8;

    pat8.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);

    usable =
        pat8.Calculate_Usable_Contact_Time(10.0);

    bool test8 = Nearly_Equal(usable, 0.0);

    Print_Test_Result("PAT-008 PAT Exceeds Contact Window", test8);

    if (test8)
        passed_tests++;

    // =========================================================
    // PAT-009 — Zero contact window
    // =========================================================

    PAT_Timing pat9;

    pat9.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);

    usable =
        pat9.Calculate_Usable_Contact_Time(0.0);

    bool test9 = Nearly_Equal(usable, 0.0);

    Print_Test_Result("PAT-009 Zero Contact Window", test9);

    if (test9)
        passed_tests++;

    // =========================================================
    // PAT-010 — Negative contact window
    // =========================================================

    PAT_Timing pat10;

    pat10.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);

    usable =
        pat10.Calculate_Usable_Contact_Time(-50.0);

    bool test10 = Nearly_Equal(usable, 0.0);

    Print_Test_Result("PAT-010 Negative Contact Window", test10);

    if (test10)
        passed_tests++;

    // =========================================================
    // PAT-011 — Normal contact efficiency
    // =========================================================

    PAT_Timing pat11;

    pat11.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat11.Calculate_Usable_Contact_Time(100.0);

    double efficiency =
        pat11.Calculate_Contact_Efficiency();

    bool test11 = Nearly_Equal(efficiency, 0.80);

    Print_Test_Result("PAT-011 Normal Contact Efficiency", test11);

    if (test11)
        passed_tests++;

    // =========================================================
    // PAT-012 — Zero usable contact efficiency
    // =========================================================

    PAT_Timing pat12;

    pat12.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat12.Calculate_Usable_Contact_Time(20.0);

    efficiency =
        pat12.Calculate_Contact_Efficiency();

    bool test12 = Nearly_Equal(efficiency, 0.0);

    Print_Test_Result("PAT-012 Zero Usable Contact Efficiency", test12);

    if (test12)
        passed_tests++;

    // =========================================================
    // PAT-013 — Normal overhead
    // =========================================================

    PAT_Timing pat13;

    pat13.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat13.Calculate_Usable_Contact_Time(100.0);

    double overhead =
        pat13.Calculate_PAT_Overhead_Percentage();

    bool test13 = Nearly_Equal(overhead, 20.0);

    Print_Test_Result("PAT-013 Normal PAT Overhead", test13);

    if (test13)
        passed_tests++;

    // =========================================================
    // PAT-014 — Exactly 100 percent overhead
    // =========================================================

    PAT_Timing pat14;

    pat14.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat14.Calculate_Usable_Contact_Time(20.0);

    overhead =
        pat14.Calculate_PAT_Overhead_Percentage();

    bool test14 = Nearly_Equal(overhead, 100.0);

    Print_Test_Result("PAT-014 Exactly 100 Percent Overhead", test14);

    if (test14)
        passed_tests++;

    // =========================================================
    // PAT-015 — Overhead clamps at 100 percent
    // =========================================================

    PAT_Timing pat15;

    pat15.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat15.Calculate_Usable_Contact_Time(10.0);

    overhead =
        pat15.Calculate_PAT_Overhead_Percentage();

    bool test15 = Nearly_Equal(overhead, 100.0);

    Print_Test_Result("PAT-015 Overhead Clamped to 100 Percent", test15);

    if (test15)
        passed_tests++;

    // =========================================================
    // PAT-016 — Getter values
    // =========================================================

    PAT_Timing pat16;

    pat16.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat16.Calculate_Usable_Contact_Time(100.0);
    pat16.Calculate_Contact_Efficiency();
    pat16.Calculate_PAT_Overhead_Percentage();

    bool test16 =
        Nearly_Equal(pat16.Get_Total_PAT_Time(), 20.0) &&
        Nearly_Equal(pat16.Get_Usable_Contact_Time(), 80.0) &&
        Nearly_Equal(pat16.Get_Contact_Efficiency(), 0.80) &&
        Nearly_Equal(pat16.Get_PAT_Overhead_Percentage(), 20.0);

    Print_Test_Result("PAT-016 Getter Values Match", test16);

    if (test16)
        passed_tests++;

    // =========================================================
    // PAT-017 — Repeated calculations update correctly
    // =========================================================

    PAT_Timing pat17;

    pat17.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat17.Calculate_Usable_Contact_Time(100.0);

    pat17.Calculate_Total_PAT_Time(2.0, 3.0, 5.0);
    usable =
        pat17.Calculate_Usable_Contact_Time(50.0);

    bool test17 =
        Nearly_Equal(pat17.Get_Total_PAT_Time(), 10.0) &&
        Nearly_Equal(usable, 40.0);

    Print_Test_Result("PAT-017 Repeated Calculation Updates State", test17);

    if (test17)
        passed_tests++;

    // =========================================================
    // PAT-018 — Invalid PAT input resets total PAT
    // =========================================================

    PAT_Timing pat18;

    pat18.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);

    double invalid_result =
        pat18.Calculate_Total_PAT_Time(-1.0, 5.0, 5.0);

    bool test18 =
        Nearly_Equal(invalid_result, 0.0) &&
        Nearly_Equal(pat18.Get_Total_PAT_Time(), 0.0);

    Print_Test_Result("PAT-018 Invalid PAT Resets Total PAT", test18);

    if (test18)
        passed_tests++;

    // =========================================================
    // PAT-019 — Invalid PAT input clears dependent state
    // =========================================================

    PAT_Timing pat19;

    pat19.Calculate_Total_PAT_Time(10.0, 5.0, 5.0);
    pat19.Calculate_Usable_Contact_Time(100.0);
    pat19.Calculate_Contact_Efficiency();
    pat19.Calculate_PAT_Overhead_Percentage();

    pat19.Calculate_Total_PAT_Time(-1.0, 5.0, 5.0);

    bool test19 =
        Nearly_Equal(pat19.Get_Total_PAT_Time(), 0.0) &&
        Nearly_Equal(pat19.Get_Usable_Contact_Time(), 0.0) &&
        Nearly_Equal(pat19.Get_Contact_Efficiency(), 0.0) &&
        Nearly_Equal(pat19.Get_PAT_Overhead_Percentage(), 0.0);

    Print_Test_Result("PAT-019 Invalid PAT Clears Dependent State", test19);

    if (test19)
        passed_tests++;

    // =========================================================
    // SUMMARY
    // =========================================================

    std::cout << "\n========================\n";
    std::cout << "PAT VALIDATION SUMMARY\n";
    std::cout << "========================\n";
    std::cout << "Passed: "
              << passed_tests
              << " / "
              << total_tests
              << '\n';

    std::cout << "BATCH RESULT: "
              << (passed_tests == total_tests ? "PASS" : "FAIL")
              << '\n';


    return 0;
}