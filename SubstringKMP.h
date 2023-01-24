#pragma once
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace std;

char* readPiSubstr(string& filename, int start, int end);
// Find and open PI file
// Return: True, if found. False, if could not find.
bool OpenPIFile(ifstream& PIFile);

// Verify that user input is correct
bool IsValidRange(const int MinRange, const int MaxRange, const size_t PILength);

// Calculate "Partial match" table
// Return: array of integers
int* KMP_Table(string substring);

// Knuth–Morris–Pratt algorithm
// Return: the starting index of number in PI from interval [MinRange, MaxRange]. Returns -1, if there is no such number in this interval.
int KMP(string Substring, string PI);

int kmpMain();

