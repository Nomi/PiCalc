#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "MPIR/gmpxx.h"
#include "ChudnovskyPiBS.h"

#define OUTPUT_TXT_FILEPATH "./calculated_pi.txt"

int main(int argc, char* argv[])
{

    if (argc != 2)
    {
        std::cout << "FATAL ERROR. INVALID ARGUMENTS COUNT FOR PiCalcMPIR COMPATIBILITY Program." << std::endl;
    }
    ///Configuration
    std::string digStr = argv[1];
    mpz_class t = mpz_class(digStr, 10);
    unsigned long digits = t.get_ui();
    std::cout <<"Recieved "<<digits<< "." << std::endl;

    ///Computing Pi
    std::cout << "Starting computation." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    ChudnovskyPiBS* piCalc = new ChudnovskyPiBS(digits);

    mpz_class calculatedPi = piCalc->calculatePi();
    auto stop = std::chrono::high_resolution_clock::now();
    
    auto duration_MS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    auto duration_S = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    std::chrono::duration<double> elapsed_time = stop - start;
    float execution_time = (float)elapsed_time.count();
    size_t duration;
    std::string suffix = "miliseconds";
    if (duration_S.count() != 0)
    {
        duration = duration_S.count();
        suffix = "seconds";
    }
    else
        duration = duration_MS.count();
    //std::cout << "Computed " << digits <<" (10^"<<log10l(digits) << ") digits (excluding the first digit ('3')) in " << duration << suffix << "." << std::endl << std::endl;
    std::cout << "Computed " << digits <<" (10^"<<log10l(digits) << ") digits (excluding the first digit ('3')) in " << execution_time << "seconds" << "." << std::endl << std::endl;

    ///Writing to file:
    std::string calculatedPiStr = calculatedPi.get_str();
    std::cout << "Writing computed value to output file." << std::endl;
    std::ofstream out(OUTPUT_TXT_FILEPATH);
    out << calculatedPiStr;
    out.close();
    std::cout << "Wrote computed values to :" << OUTPUT_TXT_FILEPATH << "!" << std::endl << std::endl;

    ///Running simple tests
    bool doesWork = (calculatedPiStr.length() == 1 + digits);
    std::string testStatus = "FAILED";
    if (doesWork)
    {
        testStatus = "passed (remember, tests are not exhaustive)";
    }
    std::cout << "Initial tests have " + testStatus + "." << std::endl << std::endl;

    ///Finished.
    std::cout << "Program finished." << std::endl;
    return EXIT_SUCCESS;
}