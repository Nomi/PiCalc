#include <thread>
#include <future>
#include <barrier>
#include "ChudnovskyPiBS.h"
#include <sstream>

#include <Windows.h> //this should be the last import somehow?

//Global variables for configuration:
const int NUM_THREADS_IN_CPU = std::thread::hardware_concurrency();
const int NUM_MAIN_WORKER_THREADS = NUM_THREADS_IN_CPU;
const int TOTAL_THREAD_COUNT = ChudnovskyPiBS::getTotalNumThreadsFromUsefulNumThreads(NUM_MAIN_WORKER_THREADS);
std::barrier ALL_THREADS_SPAWNED(NUM_MAIN_WORKER_THREADS);


//Actual definitions:

ChudnovskyPiBS::ChudnovskyPiBS(unsigned long _digits)
{
	digits = _digits;
	if (_digits != (unsigned long)((long double)_digits))
		throw _EXCEPTION_; //precision can't be handled by long double.
	N = (unsigned long)(digits / DIGITS_PER_TERM + 1);

	mpz_ui_pow_ui(one_squared.get_mpz_t(), 10, 2 * digits);

	//one_squared.get_str();

	mpz_class thousandandfive__times__one_squared = 10005 * one_squared;
	mpz_sqrt(sqrtC.get_mpz_t(), thousandandfive__times__one_squared.get_mpz_t());

	//sqrtC.get_str();

	mpz_ui_pow_ui(intBigC3_OVER_24.get_mpz_t(), C, 3);
	mpz_class twentyfour = 24;
	mpz_fdiv_q(intBigC3_OVER_24.get_mpz_t(), intBigC3_OVER_24.get_mpz_t(), twentyfour.get_mpz_t());
	//intBigC3_OVER_24.get_str();
}

bsReturn ChudnovskyPiBS::bs(mpz_class a, mpz_class b) //clearly thread safe because nothing from outside the function is written to.
{
	bsReturn result;
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
void ChudnovskyPiBS::directlyCompute__P_Q_T__from_A_to_AplusOne(mpz_class& a, mpz_class& Pab, mpz_class& Qab, mpz_class& Tab)
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
void ChudnovskyPiBS::recursivelyComputePabQabTab_SingleThreaded(mpz_class& a, mpz_class& b, mpz_class& m, mpz_class& Pab, mpz_class& Qab, mpz_class& Tab)
{
	// Recursively compute P(a,b), Q(a,b) and T(a,b)
	// m is the midpoint of and b
	mpz_class aplusb = a + b;
	mpz_div_ui(m.get_mpz_t(), aplusb.get_mpz_t(), 2);//equivalent pseudocode: m=floor((a+b)/2)
	// Recursively calculate P(a, m), Q(a, m) and T(a, m)
	bsReturn am = bs(a, m);
	// Recursively calculate P(m, b), Q(m, b) and T(m, b)
	bsReturn mb = bs(m, b);
	// Now combine
	Pab = am.P * mb.P;
	Qab = am.Q * mb.Q;
	Tab = (mb.Q * am.T) + (am.P * mb.T);
}

bsReturn ChudnovskyPiBS::bs_multithreaded(mpz_class a, mpz_class b, int threadCount) //clearly thread safe because nothing from outside the function is written to.
{
	bsReturn result;
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
		bsReturn am; am.Q = NULL;
		std::future<bsReturn> futAm;
		if (threadCount > 0)
		{
			futAm = std::async(std::launch::async, &ChudnovskyPiBS::bs_multithreaded, this, a, m, (--threadCount - 1) / 2);
		}
		else
		{
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			am = bs(a, m);
		}
		// Recursively calculate P(m, b), Q(m, b) and T(m, b)
		bsReturn mb; mb.Q = NULL;
		std::future<bsReturn> futMb;
		if (threadCount > 0)
		{
			futMb = std::async(std::launch::async, &ChudnovskyPiBS::bs_multithreaded, this, m, b, (--threadCount) / 2);
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

bsReturn ChudnovskyPiBS::bs_multithreaded_barrier(mpz_class a, mpz_class b, int threadCount, int depth) //clearly thread safe because nothing from outside the function is written to.
{
	if (threadCount == 0)
	{
		ALL_THREADS_SPAWNED.arrive_and_wait();
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	}
	depth++;
	bsReturn result;
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
		bsReturn am; am.Q = NULL;
		std::future<bsReturn> futAm;
		if (threadCount > 0)
		{
			int subThreadCountLeft = (--threadCount - 1) / 2; //(int)powl(2, depth); //About the following comment, I might have been wrong. Can't blame me, I'm sleep deprived. //you divide threadCount by powl(2,depth) because 2^depth tells you the number of parallel threads of the binary recursion (or the nodes of a binary tree) at this depth and so you can calculate how to divide number of threads to spawn.
			futAm = std::async(std::launch::async, &ChudnovskyPiBS::bs_multithreaded_barrier, this, a, m, subThreadCountLeft, depth); 
		}
		else
		{
			//std::ostringstream stream;
			//stream << "Depth "<<depth<<":Main worker thread (Thread ID: " << GetThreadId(GetCurrentThread()) << ") ready for duty! (Working on Left now)" << std::endl;
			//std::cout << stream.str();
			am = bs(a, m);
		}
		// Recursively calculate P(m, b), Q(m, b) and T(m, b)
		bsReturn mb; mb.Q = NULL;
		std::future<bsReturn> futMb;
		if (threadCount > 0)
		{
			int subThreadCountRight = (--threadCount) / 2;//(int)powl(2, depth); //About the following comment, I might have been wrong. Can't blame me, I'm sleep deprived. //you divide threadCount by powl(2,depth) because 2^depth tells you the number of parallel threads of the binary recursion (or the nodes of a binary tree) at this depth and so you can calculate how to divide number of threads to spawn.
			futMb = std::async(std::launch::async, &ChudnovskyPiBS::bs_multithreaded_barrier, this, m, b, subThreadCountRight, depth);
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

int ChudnovskyPiBS::getTotalNumThreadsFromUsefulNumThreads(int usefulThreadCountWanted)
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

mpz_class ChudnovskyPiBS::calculatePi()
{
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

	//std::cout << NUM_THREADS_IN_CPU << std::endl;
	//std::cout << TOTAL_THREAD_COUNT << std::endl;
	bsReturn BSResult = bs_multithreaded_barrier(0, N, TOTAL_THREAD_COUNT, 0); //bs_multithreaded(0, N, TOTAL_THREAD_COUNT); //bs_multithreaded_barrier(0, N, TOTAL_THREAD_COUNT, 0); //bs(0, N); //apparently Q and T gotten are wrong.
	//BSResult.Q.get_str();
	//BSResult.T.get_str();
	//return mpz_fdiv_q((Q * 426880 * sqrtC) / T
	mpz_class result = (BSResult.Q * 426880 * sqrtC);
	mpz_fdiv_q(result.get_mpz_t(), result.get_mpz_t(), BSResult.T.get_mpz_t());

	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	return result;
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
	bsReturn BSResult = bs_multithreaded(0, N, totalThreadCount, 0);//bs(0, N); 
	//bsReturn BSResult = bs(0, N); //apparently Q and T gotten are wrong.
	//BSResult.Q.get_str();
	//BSResult.T.get_str();
	//return mpz_fdiv_q((Q * 426880 * sqrtC) / T
	mpz_class result = (BSResult.Q * 426880 * sqrtC);
	mpz_fdiv_q(result.get_mpz_t(), result.get_mpz_t(), BSResult.T.get_mpz_t());


	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	return result;
}
*/