#include <thread>
#include <future>
#include <barrier>
#include "MPIR_ChudnovskyPiBS.h"
#include <sstream>

#include <Windows.h> //this should be the last import somehow?

////Global variables for configuration:
const int MPIR_VERSION_NUM_THREADS_IN_CPU = std::thread::hardware_concurrency();
const int MPIR_VERSION_NUM_MAIN_WORKER_THREADS = MPIR_VERSION_NUM_THREADS_IN_CPU;
const int MPIR_VERSION_TOTAL_THREAD_COUNT = MPIR_ChudnovskyPiBS::getTotalNumThreadsFromUsefulNumThreads(MPIR_VERSION_NUM_MAIN_WORKER_THREADS);
std::barrier MPIR_VERSION_ALL_THREADS_SPAWNED(MPIR_VERSION_NUM_MAIN_WORKER_THREADS);


//Actual definitions:

MPIR_ChudnovskyPiBS::MPIR_ChudnovskyPiBS(unsigned long _digits)
{
	digits = _digits;
	if (_digits != (unsigned long)((long double)_digits))
		throw _EXCEPTION_; //precision can't be handled by long double.
	N = (unsigned long)(digits / DIGITS_PER_TERM + 1);

	futSqrtC = std::async(std::launch::async, &MPIR_ChudnovskyPiBS::getSqrtC, this, digits);

	mpz_ui_pow_ui(intBigC3_OVER_24.get_mpz_t(), C, 3);
	mpz_class twentyfour = 24;
	mpz_fdiv_q(intBigC3_OVER_24.get_mpz_t(), intBigC3_OVER_24.get_mpz_t(), twentyfour.get_mpz_t());
	//intBigC3_OVER_24.get_str();
}

MPIR_VERSION_bsReturn MPIR_ChudnovskyPiBS::bs(mpz_class a, mpz_class b) //clearly thread safe because nothing from outside the function is written to.
{
	MPIR_VERSION_bsReturn result;
	mpz_class Pab;
	mpz_class Qab;
	mpz_class Tab;
	mpz_class m;
	if (b - a == 1)
	{
		directlyCompute__P_Q_T__from_A_to_AplusOne(a, Pab, Qab, Tab);
	}
	else
	{
		recursivelyComputePabQabTab_SingleThreaded(a, b, m, Pab, Qab, Tab);
	}
	result.P = Pab;
	result.Q = Qab;
	result.T = Tab;
	//std::string pabStr = result.P.get_str();
	//std::string qabStr = result.Q.get_str();
	//std::string tabStr = result.T.get_str();
	return result;
}
void MPIR_ChudnovskyPiBS::directlyCompute__P_Q_T__from_A_to_AplusOne(mpz_class& a, mpz_class& Pab, mpz_class& Qab, mpz_class& Tab)
{
	//Directly compute P(a,a+1), Q(a,a+1) and T(a,a+1)
	if (a == 0)
		Pab = Qab = 1;
	else
	{
		Pab = (6 * a - 5) * (2 * a - 1) * (6 * a - 1);
		Qab = a * a * a * intBigC3_OVER_24;
	}

	Tab = Pab * (13591409 + 545140134 * a); // a(a) * p(a)
	//Pab.get_str();
	//Tab.get_str();
	//a.get_str();
	//Qab.get_str();
	mpz_class toCheck;
	mpz_and(toCheck.get_mpz_t(), a.get_mpz_t(), one);
	if (toCheck == 1) //note to self: works as expected
		Tab = -Tab;
}
void MPIR_ChudnovskyPiBS::recursivelyComputePabQabTab_SingleThreaded(mpz_class& a, mpz_class& b, mpz_class& m, mpz_class& Pab, mpz_class& Qab, mpz_class& Tab)
{
	// Recursively compute P(a,b), Q(a,b) and T(a,b)
	// m is the midpoint of and b
	mpz_class aplusb = a + b;
	mpz_div_ui(m.get_mpz_t(), aplusb.get_mpz_t(), 2);//equivalent pseudocode: m=floor((a+b)/2)
	// Recursively calculate P(a, m), Q(a, m) and T(a, m)
	MPIR_VERSION_bsReturn am = bs(a, m);
	// Recursively calculate P(m, b), Q(m, b) and T(m, b)
	MPIR_VERSION_bsReturn mb = bs(m, b);
	// Now combine
	Pab = am.P * mb.P;
	Qab = am.Q * mb.Q;
	Tab = (mb.Q * am.T) + (am.P * mb.T);
}

MPIR_VERSION_bsReturn MPIR_ChudnovskyPiBS::bs_multithreaded(mpz_class a, mpz_class b, int threadCount) //clearly thread safe because nothing from outside the function is written to.
{
	MPIR_VERSION_bsReturn result;
	mpz_class Pab;
	mpz_class Qab;
	mpz_class Tab;
	mpz_class m;
	if (b - a == 1)
	{
		directlyCompute__P_Q_T__from_A_to_AplusOne(a, Pab, Qab, Tab);
	}
	else
	{
		// Recursively compute P(a,b), Q(a,b) and T(a,b)
		// m is the midpoint of and b
		mpz_class aplusb = a + b;
		mpz_div_ui(m.get_mpz_t(), aplusb.get_mpz_t(), 2);//equivalent pseudocode: m=floor((a+b)/2)
		// Recursively calculate P(a, m), Q(a, m) and T(a, m)
		MPIR_VERSION_bsReturn am; am.Q = NULL;
		std::future<MPIR_VERSION_bsReturn> futAm;
		if (threadCount > 0)
		{
			futAm = std::async(std::launch::async, &MPIR_ChudnovskyPiBS::bs_multithreaded, this, a, m, (--threadCount - 1) / 2);
		}
		else
		{
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			am = bs(a, m);
		}
		// Recursively calculate P(m, b), Q(m, b) and T(m, b)
		MPIR_VERSION_bsReturn mb; mb.Q = NULL;
		std::future<MPIR_VERSION_bsReturn> futMb;
		if (threadCount > 0)
		{
			futMb = std::async(std::launch::async, &MPIR_ChudnovskyPiBS::bs_multithreaded, this, m, b, (--threadCount) / 2);
		}
		else
		{
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			mb = bs(m, b);
		}

		//Wait for threads:
		threadCount -= 2;
		if (am.Q == NULL)
			am = futAm.get();
		if (mb.Q == NULL)
			mb = futMb.get();
		// Now combine
		Pab = am.P * mb.P;
		Qab = am.Q * mb.Q;
		Tab = (mb.Q * am.T) + (am.P * mb.T);
	}
	result.P = Pab;
	result.Q = Qab;
	result.T = Tab;
	//std::string pabStr = result.P.get_str();
	//std::string qabStr = result.Q.get_str();
	//std::string tabStr = result.T.get_str();
	return result;
}

MPIR_VERSION_bsReturn MPIR_ChudnovskyPiBS::bs_multithreaded_barrier(mpz_class a, mpz_class b, int threadCount, int depth) //clearly thread safe because nothing from outside the function is written to.
{
	if (threadCount == 0)
	{
		MPIR_VERSION_ALL_THREADS_SPAWNED.arrive_and_wait();
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	}
	depth++;
	MPIR_VERSION_bsReturn result;
	mpz_class Pab;
	mpz_class Qab;
	mpz_class Tab;
	mpz_class m;
	if (b - a == 1)
	{
		directlyCompute__P_Q_T__from_A_to_AplusOne(a, Pab, Qab, Tab);
	}
	else
	{
		// Recursively compute P(a,b), Q(a,b) and T(a,b)
		// m is the midpoint of and b
		mpz_class aplusb = a + b;
		mpz_div_ui(m.get_mpz_t(), aplusb.get_mpz_t(), 2);//equivalent pseudocode: m=floor((a+b)/2)
		// Recursively calculate P(a, m), Q(a, m) and T(a, m)
		MPIR_VERSION_bsReturn am; am.Q = NULL;
		std::future<MPIR_VERSION_bsReturn> futAm;
		if (threadCount > 0)
		{
			int subThreadCountLeft = (--threadCount - 1) / 2; //(int)powl(2, depth); //About the following comment, I might have been wrong. Can't blame me, I'm sleep deprived. //you divide threadCount by powl(2,depth) because 2^depth tells you the number of parallel threads of the binary recursion (or the nodes of a binary tree) at this depth and so you can calculate how to divide number of threads to spawn.
			futAm = std::async(std::launch::async, &MPIR_ChudnovskyPiBS::bs_multithreaded_barrier, this, a, m, subThreadCountLeft, depth);
		}
		else
		{
			//std::ostringstream stream;
			//stream << "Depth "<<depth<<":Main worker thread (Thread ID: " << GetThreadId(GetCurrentThread()) << ") ready for duty! (Working on Left now)" << std::endl;
			//std::cout << stream.str();
			am = bs(a, m);
		}
		// Recursively calculate P(m, b), Q(m, b) and T(m, b)
		MPIR_VERSION_bsReturn mb; mb.Q = NULL;
		std::future<MPIR_VERSION_bsReturn> futMb;
		if (threadCount > 0)
		{
			int subThreadCountRight = (--threadCount) / 2;//(int)powl(2, depth); //About the following comment, I might have been wrong. Can't blame me, I'm sleep deprived. //you divide threadCount by powl(2,depth) because 2^depth tells you the number of parallel threads of the binary recursion (or the nodes of a binary tree) at this depth and so you can calculate how to divide number of threads to spawn.
			futMb = std::async(std::launch::async, &MPIR_ChudnovskyPiBS::bs_multithreaded_barrier, this, m, b, subThreadCountRight, depth);
		}
		else
		{
			//std::ostringstream stream;
			//stream << "Depth " << depth << ":Main worker thread (Thread ID: " << GetThreadId(GetCurrentThread()) << ") ready for duty! (Working on Right now.)" << std::endl;
			//std::cout << stream.str();
			mb = bs(m, b);
		}

		//Wait for threads:
		threadCount -= 2;
		if (am.Q == NULL)
			am = futAm.get();
		if (mb.Q == NULL)
			mb = futMb.get();
		// Now combine
		Pab = am.P * mb.P;
		Qab = am.Q * mb.Q;
		Tab = (mb.Q * am.T) + (am.P * mb.T);
	}
	result.P = Pab;
	result.Q = Qab;
	result.T = Tab;
	//std::string pabStr = result.P.get_str();
	//std::string qabStr = result.Q.get_str();
	//std::string tabStr = result.T.get_str();
	return result;
}

int MPIR_ChudnovskyPiBS::getTotalNumThreadsFromUsefulNumThreads(int usefulThreadCountWanted)
{
	if (usefulThreadCountWanted == 2)
		return 2;
	else if (usefulThreadCountWanted <= 1 || floor(log2(usefulThreadCountWanted)) != log2(usefulThreadCountWanted))
	{
		std::cout << "Invalid thread count. Needs to be some power of 2 and more than 1." << std::endl;
		throw _EXCEPTION_;
	}
	//usefulThreadCount is basically the same as the maximum leaves of a binary tree at some level, meanwhile total includes useless (those blocking/waititng for the computation threads).
	int depthOfBinaryTree = log2(usefulThreadCountWanted);//num leaves = 2^depth
	int totalThreadCountNeeded = (int)pow(2, 1 + depthOfBinaryTree) - 2;	//total nodes = 2^(k+1) -1, if without root, -1 becomes -2
	return totalThreadCountNeeded;
}

mpz_class MPIR_ChudnovskyPiBS::calculatePi()
{
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

	//std::cout << MPIR_VERSION_NUM_THREADS_IN_CPU << std::endl;
	//std::cout << MPIR_VERSION_TOTAL_THREAD_COUNT << std::endl;
	MPIR_VERSION_bsReturn BSResult = bs_multithreaded(0, N, MPIR_VERSION_TOTAL_THREAD_COUNT); //bs_multithreaded(0, N, MPIR_VERSION_TOTAL_THREAD_COUNT); //bs_multithreaded_barrier(0, N, MPIR_VERSION_TOTAL_THREAD_COUNT, 0); //bs(0, N); //apparently Q and T gotten are wrong.
	//BSResult.Q.get_str();
	//BSResult.T.get_str();
	//return mpz_fdiv_q((Q * 426880 * sqrtC) / T
	mpz_class result = (BSResult.Q * 426880 * futSqrtC.get());
	mpz_fdiv_q(result.get_mpz_t(), result.get_mpz_t(), BSResult.T.get_mpz_t());

	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	return result;
}

/// <summary>
/// Constructor for MPIR_ChudnovskyPiBS Class.
/// </summary>
/// <param name="_digits">Used to show number of digits.</param>

int MPIR_ChudnovskyPiBS::compatbilityMainFunctionAlternative(unsigned long digits, std::string outputTextFilePath)
{
	//Configuration
	std::cout << "Recieved " << digits << "." << std::endl;

	///Computing Pi
	std::cout << "Starting computation." << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

	MPIR_ChudnovskyPiBS* piCalc = new MPIR_ChudnovskyPiBS(digits);
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
	{
		duration = duration_MS.count();
	}
	std::cout << "Computed " << digits << " (10^" << log10l(digits) << ") digits (excluding the first digit ('3')) in " << execution_time << "seconds" << "." << std::endl << std::endl;

	///Writing to file:
	std::cout << "Writing computed value to output file." << std::endl;
	std::string calculatedPiStr = calculatedPi.get_str();
	std::ofstream out(outputTextFilePath);
	out << calculatedPiStr;
	out.close();
	std::cout << "Wrote computed values to :" << outputTextFilePath << "!" << std::endl << std::endl;

	///Running simple tests
	std::cout << "Running some basic tests " << std::endl;
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

mpz_class MPIR_ChudnovskyPiBS::getSqrtC(unsigned long digits)
{
	mpz_class one_squared;
	mpz_ui_pow_ui(one_squared.get_mpz_t(), 10, 2 * digits);

	//one_squared.get_str();
	mpz_class thousandandfive__times__one_squared = 10005 * one_squared;
	mpz_class sqrtC;
	mpz_sqrt(sqrtC.get_mpz_t(), thousandandfive__times__one_squared.get_mpz_t());

	//sqrtC.get_str();

	return sqrtC;
}

/* DEPRECATED/OLD MULTITHREADING SOLUTION
mpz_class ChudnovskyPiBS::calculatePi()
{
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);


	//for (int i = 1; i < 5; i++)
	//{
	//	std::cout << (int)pow(2, i) <<" - " << getTotalNumThreadsFromUsefulNumThreads((int)pow(2,i)) << std::endl;
	//}
	int numberOfThreadsInCPU = std::thread::hardware_concurrency();
	int totalThreadCount = getTotalNumThreadsFromUsefulNumThreads(numberOfThreadsInCPU);
	MPIR_VERSION_bsReturn BSResult = bs_multithreaded(0, N, totalThreadCount, 0);//bs(0, N);
	//MPIR_VERSION_bsReturn BSResult = bs(0, N); //apparently Q and T gotten are wrong.
	//BSResult.Q.get_str();
	//BSResult.T.get_str();
	//return mpz_fdiv_q((Q * 426880 * sqrtC) / T
	mpz_class result = (BSResult.Q * 426880 * sqrtC);
	mpz_fdiv_q(result.get_mpz_t(), result.get_mpz_t(), BSResult.T.get_mpz_t());


	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	return result;
}
*/