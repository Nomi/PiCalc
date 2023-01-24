#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <chrono>
#include <gmpxx.h>
#include "ChudnovskyPiBS.h"
#include "SubstringKMP.h"

#define OUTPUT_TXT_FILEPATH "./calculated_pi.txt"

int main()
{
    int modeIntent = -1;
    while (modeIntent != 1 && modeIntent != 2)
    {
        std::cout << "Available options:" << std::endl;
        std::cout << "1 = Calculate Pi Decimal Expansion." << std::endl;
        std::cout << "2 = Pattern Match (numbers from 1 to 10000)." << std::endl;
        std::cout << "What would you like to do? [Enter 1 or 2]: ";
        std::cin >> modeIntent;
    }
    if (modeIntent == 2)
    {
        return kmpMain();
    }
    ///Configuration
    std::cout << "Configuration started." << std::endl;
    unsigned long digits; //= 100000000; //apart from 3. number of decimal places.
    std::cout << "Enter the number of digits to calculate (excluding initial 3): ";
    std::cin >> digits;
    std::cout << "Recieved " << digits << "." << std::endl;

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
    
    std::cout << "Computed " << digits <<" (10^"<<log10l(digits) << ") digits (excluding the first digit ('3')) in " << execution_time << "seconds" << "." << std::endl << std::endl;
    

    ///Writing to file:
    std::cout << "Writing computed value to output file." << std::endl;
    FILE* pOutFile = fopen(OUTPUT_TXT_FILEPATH,"w");
    if(pOutFile==NULL)
    {
        perror("Error opening/creating output file. ");
        exit(EXIT_FAILURE);
    }
    size_t writtenChars = mpz_out_str(pOutFile, 10, calculatedPi.get_mpz_t());
    if(fclose(pOutFile)!=0)
    {
        perror("Error closing output file. ");
        exit(EXIT_FAILURE);
    }
    std::cout << "Wrote computed values to :" << OUTPUT_TXT_FILEPATH << "!" << std::endl << std::endl;

    ///Running simple tests
    std::cout << "Running simple tests. You may press \"ctrl+c\" to cancel them at any time." << std::endl;
    bool doesWork = (calculatedPiStr.length() == 1 + digits);
    std::string testStatus = "FAILED";
    if (doesWork)
    {
        testStatus = "passed (remember, tests are not exhaustive)";
    }
    std::cout << "Initial tests have " + testStatus + "." << std::endl << std::endl;

    ///Finished.
    std::cout << "Program finished. Press any key to exit. ";
    getch();
    std::cout << std::endl;
    return EXIT_SUCCESS;
}