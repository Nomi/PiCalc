#pragma once
#include <iostream>
#include <math.h>
#include <stdexcept>
#include <barrier>
#include <thread>
#include <future>
#include <fstream>
#include "MPIR/mpirxx.h"

//Helper structs:
struct MPIR_VERSION_bsReturn
{
	mpz_class P;
	mpz_class Q;
	mpz_class T;
};

//The class itself:
class MPIR_ChudnovskyPiBS //Calculates Pi using Chudnovsky algorithm with Binary Splitting
{
private:
	const long double C = 640320;
	const long double C3_OVER_24 = powl(C, 3) / 24;
	mpz_class intBigC3_OVER_24 = 0;
	const long double DIGITS_PER_TERM = log10l(C3_OVER_24 / 6 / 2 / 6);

	unsigned long N;
	unsigned long digits;

	std::future<mpz_class> futSqrtC;

	mpz_class oneClass = 1;
	mpz_ptr one = oneClass.get_mpz_t();

	/// <summary>
	/// Computes the terms for binary splitting the Chudnovsky infinite series.
	/// 
	///    a(a) = +/- (13591409 + 545140134*a)
	///    p(a) = (6*a-5)*(2*a-1)*(6*a-1)
	///    b(a) = 1
	///    q(a) = a*a*a*C3_OVER_24
	///
	///    returns P(a,b), Q(a,b) and T(a,b)
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	MPIR_VERSION_bsReturn bs(mpz_class a, mpz_class b);
	void recursivelyComputePabQabTab_SingleThreaded(mpz_class& a, mpz_class& b, mpz_class& m, mpz_class& Pab, mpz_class& Qab, mpz_class& Tab);
	void directlyCompute__P_Q_T__from_A_to_AplusOne(mpz_class& a, mpz_class& Pab, mpz_class& Qab, mpz_class& Tab);
	MPIR_VERSION_bsReturn bs_multithreaded(mpz_class a, mpz_class b, int threadCount);
	MPIR_VERSION_bsReturn bs_multithreaded_barrier(mpz_class a, mpz_class b, int threadCount, int depth); //uses a barrier to wait for all main worker threads to spawn.
	mpz_class getSqrtC(unsigned long digits);
	/// <summary>
	/// Constructor for MPIR_ChudnovskyPiBS Class.
	/// </summary>
	/// <param name="_digits">Used to show number of digits.</param>
	MPIR_ChudnovskyPiBS(unsigned long _digits);
	mpz_class calculatePi();
public:
	static int getTotalNumThreadsFromUsefulNumThreads(int usefulThreadCountWanted);
	static int compatbilityMainFunctionAlternative(unsigned long digits, std::string outputTextFilePath);
};

