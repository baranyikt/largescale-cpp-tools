#include "stdafx.h"

#include <functional>
#include <iostream>
#include <type_traits>
#include <string>
#include <limits>
#include <cmath>


// these defines have to precede #include "INTO.h"
#define __DEBUG_CHECK_INTEGER_OVERFLOW								// this switch changes unsignedo etc. typedefs back and forth between overflow checked and unchecked versions
#define __DEBUG_CHECK_INTEGER_OVERFLOW_ALIAS						// unsignedo etc. typedefs can be turned off if not needed
// #define __DEBUG_CHECK_INTEGER_OVERFLOW_NAMESPACE	ofchecked		// an optional namespace can be defined (not applies to unsignedo etc.)
#define __DEBUG_CHECK_INTEGER_OVERFLOW_USE_X86_ASM
#include "INTO.h"


auto TryOrExcept = [](std::string description, std::function<std::string(void)> tryThis) {
	try
	{
		std::cout << "Trying " << description << "...";
		std::string result = tryThis();
		std::cout << "OK! [" << result << "]\n";
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
};

#define TEST_CASE_DESC_FN2(_type1_, _initval1_, _type2_, _initval2_, _type3_, _function_)	\
	std::string(typeid(_type2_).name()) + " r = " + std::string(#_function_) + "((" + typeid(_type1_).name() + ")" + std::to_string(_initval1_) + ", " + \
	std::string("(") + typeid(_type2_).name() + ")" + std::to_string(_initval2_) + ")"

#define TEST_CASE_FUNC_FN2(_type1_, _initval1_, _type2_, _initval2_, _type3_, _function_)	\
	[](){ overflowchecked<_type1_> a(_initval1_); overflowchecked<_type2_> b(_initval2_); overflowchecked<_type3_> c =  _function_((_type1_)a, (_type2_)b); return std::to_string(c) + " common type: " +  typeid(INTO_common_t<_type1_,_type2_>).name(); }

#define TEST_CASE_FN2(_type1_, _initval1_, _type2_, _initval2_, _type3_, _function_)	\
	TEST_CASE_DESC_FN2(_type1_, _initval1_, _type2_, _initval2_, _type3_, _function_), TEST_CASE_FUNC_FN2(_type1_, _initval1_, _type2_, _initval2_, _type3_, _function_)


#define TEST_CASE_DESC_OP(_type1_, _initval1_, _type2_, _initval2_, _operation_)	\
	std::string("(") + typeid(_type1_).name() + ")" + std::to_string(_initval1_) + std::string(#_operation_) + \
	std::string("(") + typeid(_type2_).name() + ")" + std::to_string(_initval2_)

#define TEST_CASE_FUNC_OP(_type1_, _initval1_, _type2_, _initval2_, _operation_)	\
	[](){ overflowchecked<_type1_> a(_initval1_); overflowchecked<_type2_> b(_initval2_); auto c = a _operation_ b; return std::to_string(c) + " common type: " +  typeid(INTO_common_t<_type1_,_type2_>).name(); }

#define TEST_CASE_OP(_type1_, _initval1_, _type2_, _initval2_, _operation_)	\
	TEST_CASE_DESC_OP(_type1_, _initval1_, _type2_, _initval2_, _operation_), TEST_CASE_FUNC_OP(_type1_, _initval1_, _type2_, _initval2_, _operation_)

#define AUTO_TEST_OP	*
#define STRINGIZER2(param)	#param
#define STRINGIZER(param)	STRINGIZER2(param)
#define AUTO_TEST_OP_STR	STRINGIZER(AUTO_TEST_OP)

template <typename S, typename T> void AUTO_TEST()
{
	constexpr auto S_start = std::numeric_limits<S>::min();
	constexpr auto S_end = std::numeric_limits<S>::max();
	constexpr auto S_step = 128;
	constexpr auto T_start = std::numeric_limits<T>::min();
	constexpr auto T_end = std::numeric_limits<T>::max();
	constexpr auto T_step = 128;

	bool bError = false;
	S s = S_start;
	for (;;)
	{
		T t = T_start;
		for (;;)
		{
			//			std::cout << s << "\t" << t << "\n";
			bool bOverflow1, bOverflow2 = false;

			std::common_type_t<S, T> u = s + t;
			MaximumEncloser_t<std::common_type_t<S, T>> s_big = s, t_big = t;
			MaximumEncloser_t<std::common_type_t<S, T>> u_big = s_big + t_big;
			bOverflow1 = u != u_big;

			overflowchecked<S> s_oc(s);
			overflowchecked<T> t_oc(t);
			try
			{
				auto u_oc = s_oc + t_oc;
			}
			catch (INTO_exception&)
			{
				bOverflow2 = true;
			}
			if (bOverflow1 != bOverflow2)
			{
				bError = true;
				std::cout << "Error:\t" << s << "\t" << t << "\t" << bOverflow1 << "\t" << bOverflow2 << u << "\t" << u_big << "\n";
			}
			if (t == T_end) break;
			else if (T_end - t > 0 && T_end - t < T_step) t = T_end;
			else t += T_step;
		} // for s
		{
			static int64_t counter = 0;
			if (++counter % 100 == 0)
				std::cout << ".";
		}
		if (s == S_end) break;
		else if (S_end - s > 0 && S_end - s < S_step) s = S_end;
		else s += S_step;
	}
	if (!bError)
		std::cout << "\n" << typeid(S).name() << "/" << typeid(T).name() << "/" << AUTO_TEST_OP_STR << ": Test OK\n";
}


int main()
{
	TryOrExcept(TEST_CASE_OP(char, 127, char, 255, +));
	TryOrExcept(TEST_CASE_OP(char, 127, int, std::numeric_limits<int>::max(), +));
	TryOrExcept(TEST_CASE_OP(char, 127, int, 127, +));
	TryOrExcept(TEST_CASE_OP(char, 64, char, 63, +));
	TryOrExcept(TEST_CASE_OP(char, -64, char, -65, +));

	TryOrExcept(TEST_CASE_OP(signed int, 0, unsigned int, -1, +));

	TryOrExcept(TEST_CASE_FN2(signed short, std::numeric_limits<short>::max(), signed short, 2, signed short, pow));
	try
	{
		std::cout << "Trying std::pow(std::numeric_limits<int64_t>::max(), 2)...";
		overflowchecked<int64_t> probe = std::pow(std::numeric_limits<int64_t>::max(), 2);
		std::cout << "OK (?)\n";
	}
	catch (INTO_exception&)
	{
		std::cout << "Overflow\n";
	}
	

	AUTO_TEST<short, int>();

}