#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "GMP/gmpxx.h"
#include "ChudnovskyPiBS.h"
#include <string>
#include "Windows.h"
#define OUTPUT_TXT_FILEPATH "./calculated_pi.txt"
#define WMPIR_COMPATIBILITY_PROGRAM_PATH L"./pcwmpir-CompatabilityProgram.procxe"
#define HUNDRED_MILLION 100000000
int main()
{
    ///Configuration
    std::cout << "Configuration started." << std::endl;
    unsigned long digits; //= 100000000; //apart from 3. number of decimal places.
    std::cout << "Enter the number of digits to calculate (excluding initial 3): ";
    std::cin >> digits;
    std::cout << std::endl;
    if (digits > HUNDRED_MILLION)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));


        std::wstring wpath=WMPIR_COMPATIBILITY_PROGRAM_PATH;
        std::wstring wdigits = std::to_wstring(digits);
        std::wstring wcmnd = wpath + L" " + wdigits;
        LPWSTR cmnd = LPWSTR(wcmnd.c_str());
        // Start the child process. 
        if (!CreateProcess(NULL,   // No module name (use command line)
            cmnd,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            printf("CreateProcess failed for wmpir Compatility Program (%d).\n", GetLastError());
            return -96;
        }

        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        //return from this prog
        std::cout << "Want to exit?" << std::endl;
        system("PAUSE");
        return 96;
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