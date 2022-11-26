#pragma once
#include <iostream>
#include <math.h>
#include <stdexcept>
#include <barrier>
#include "GMP/gmpxx.h"

//Helper structs:
struct bsReturn
{
	mpz_class P;
	mpz_class Q;
	mpz_class T;
};

//The class itself:
class ChudnovskyPiBS //Calculates Pi using Chudnovsky algorithm with Binary Splitting
{
private:
	const long double C = 640320;
	const long double C3_OVER_24 = powl(C,3)/24;
	mpz_class intBigC3_OVER_24 = 0;
	const long double DIGITS_PER_TERM = log10l(C3_OVER_24 / 6 / 2 / 6);

	unsigned long N;
	unsigned long digits;
	mpz_class one_squared;
	mpz_class sqrtC;

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
	bsReturn bs(mpz_class a, mpz_class b);
	bsReturn bs_multithreaded(mpz_class a, mpz_class b, int threadCount, int depth);

public:
	/// <summary>
	/// Constructor for ChudnovskyPiBS Class.
	/// </summary>
	/// <param name="_digits">Used to show number of digits.</param>
	ChudnovskyPiBS(unsigned long _digits);
	static int getTotalNumThreadsFromUsefulNumThreads(int usefulThreadCountWanted);

	mpz_class calculatePi();

	
};

