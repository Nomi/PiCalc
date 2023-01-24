#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <chrono>
using namespace std;

// Find and open PI file
// Return: True, if found. False, if could not find.
bool OpenPIFile(ifstream& PIFile) {
	PIFile = ifstream("calculated_pi.txt");

	// Try to find PI file in different folders
	if (PIFile.fail()) {
		PIFile = ifstream("../1-calculatePI/calculated_pi.txt");
	}
	if (PIFile.fail()) {
		PIFile = ifstream("../../calculated_pi.txt");
	}
	if (PIFile.fail()) {
		PIFile = ifstream("../../examples/calculated_pi.txt");
	}
	if (PIFile.fail()) {
		PIFile = ifstream("../../executables/calculated_pi.txt");
	}
	if (PIFile.fail()) {
		PIFile = ifstream("../calculated_pi.txt");
	}
	if (PIFile.fail()) {
		PIFile = ifstream("../examples/calculated_pi.txt");
	}
	if (PIFile.fail()) {
		PIFile = ifstream("../source/1-calculatePI/calculated_pi.txt");
	}
	if (PIFile.fail()) {
		PIFile = ifstream("../source/2-substringKMP/calculated_pi.txt");
	}

	if (PIFile.fail()) { // Didn't find PI file
		cout << "\"calculated_pi.txt\" not found!" << endl;
		cout << "Put the \"calculated_pi.txt\" in any of these directories:" << endl;
		cout << "/ (Root)" << endl;
		cout << "/examples/" << endl;
		cout << "/executables/" << endl;
		cout << "/source/1-calculatePI/" << endl;
		cout << "/source/2-substringKMP" << endl;
		return false;
	}
	return true;
}

// Verify that user input is correct
bool IsValidRange(const int MinRange, const int MaxRange, const size_t PILength)
{
	return MinRange >= 0 && MinRange <= PILength && MaxRange >= MinRange && MaxRange <= PILength;
}

// Calculate "Partial match" table
// Return: array of integers
int* KMP_Table(string substring)
{
	const size_t SubstringLen = substring.length();
	int* const PartialMatchTable = new int[SubstringLen]();

	for (int pos = 1, len = 0; pos < SubstringLen;)
	{
		if (substring[pos] == substring[len])
		{
			PartialMatchTable[pos] = len;
			pos++;
			len++;
		}
		else
		{
			if (len != 0)
			{
				len = PartialMatchTable[len - 1];
			}
			else
			{
				PartialMatchTable[pos] = 0;
				pos++;
			}
		}
	}

	return PartialMatchTable;
}

// Knuth–Morris–Pratt algorithm
// Return: the starting index of number in PI from interval [MinRange, MaxRange]. Returns -1, if there is no such number in this interval.
int KMP(string Substring, string PI)
{
	const int* const Table = KMP_Table(Substring);

	const size_t SubstringLen = Substring.length();
	const size_t PI_Len = PI.length();
	int SubstringPos = 0;
	int PI_Pos = 0;

	while ((PI_Len - PI_Pos) >= (SubstringLen - SubstringPos))
	{
		if (Substring[SubstringPos] == PI[PI_Pos])
		{
			SubstringPos++;
			PI_Pos++;
		}

		if (SubstringPos == SubstringLen)
		{
			return PI_Pos - SubstringPos;
		}

		else if (PI_Pos < PI_Len && Substring[SubstringPos] != PI[PI_Pos])
		{
			if (SubstringPos != 0) SubstringPos = Table[SubstringPos - 1];
			else PI_Pos++;
		}
	}

	delete[] Table;
	return -1;
}
char* readPiSubstr(ifstream& file, int start, int end) {
	if (file.is_open())
	{
		file.seekg(start);
		char* s = new char[end - start + 1]; //+1 for \0
		file.read(s, end - start);
		s[end - start] = '\0';
		return s;
	}
	file.close();
	return nullptr;
}

int kmpMain()
{
	auto start = std::chrono::high_resolution_clock::now();
	// Get user input - range of substrings
	unsigned int MinRange = 0, MaxRange = 0;
	cout << "Enter min index: ";
	cin >> MinRange;
	cout << "Enter max index: ";
	cin >> MaxRange;

	// Read PI from file to string
	ifstream PIFile;
	if (!OpenPIFile(PIFile)) {
		system("pause");
		return 1;
	}
	cout << "Reading PI from file..." << endl;
	const string PI = readPiSubstr(PIFile, MinRange, MaxRange);

	// Create output file
	ofstream SubstringsFile("substrings_table.txt", ios::out | ios::trunc);

	// Calculate table and save to file at once
	std::string outPutFileContent = std::string();
	cout << "Pattern matching..." << endl;
	for (int i = 0; i < 10000; i++)
	{
		// Find index of substring
		const int Idx = KMP(to_string(i), PI);
		std::string str = to_string(i) + ", " + to_string(Idx) + "\n";
		outPutFileContent += str;
		// Print current substring
		cout << i << "\r";
	}
	// Save to file
	SubstringsFile << outPutFileContent;
	SubstringsFile.close();
	cout << "Saving to substring..." << endl;
	cout << endl << "Saved to substring_table.txt!" << endl;
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
	std::cout << "Finished in " << duration << suffix << "." << std::endl << std::endl;
	system("pause");
	return 0;
}
