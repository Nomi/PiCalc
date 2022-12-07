#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "GMP/gmpxx.h"
#include "ChudnovskyPiBS.h"
#include "MPIR_ChudnovskyPiBS.h"

#define OUTPUT_TXT_FILEPATH "./calculated_pi.txt"
#define HUNDRED_MILLION powl(10,4)

int main()
{
    ///Configuration
    std::cout << "Configuration started." << std::endl;
    unsigned long digits; //= 100000000; //apart from 3. number of decimal places.
    std::cout << "Enter the number of digits to calculate (excluding initial 3): ";
    std::cin >> digits;
    std::cout << std::endl;

    /// Switching to MPIR version if needed: //This is done because only the Windows version of MPIR is properly compatible with Windows (at least the memory checking aspect). So, for more than 10^8 digits, we use MPIR version. BUT, MPIR version is not really the best because it doesn't perform as well, so I keep GMP for digits <= 10^8.
    if (digits > HUNDRED_MILLION)
    {
        std::cout << "Entering Windows \"extremely large digit count\" compatibility mode." << std::endl;
        int retVal = MPIR_ChudnovskyPiBS::compatbilityMainFunctionAlternative(digits, OUTPUT_TXT_FILEPATH);
        system("PAUSE");
        return retVal;
    }
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
    std::cout << "Program finished. Want to exit?" << std::endl;
    system("PAUSE");
    return EXIT_SUCCESS;
}