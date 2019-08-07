#pragma once

#include <limits>

namespace CodeExpiresFeature { namespace details {

// For the following two literals all that matters is the place and number 
// of Y/M/D chars: they describe where to search for the corresponding parts of 
// dates as well as some further formatting info:
// - whether year has 2 (YY), or 
// - 4 digits (YYYY),
// - whether month indicated by abbreviated english name (MMM),
// - by a zero-filled number (MM),
// - or by a space-padded number (mm),
// - and the same for days (DD or dd).

constexpr char DATE_FORMAT__COMPILER[] = "MMM dd YYYY";			// date format the compiler uses in __DATE__ precomp. directive
constexpr char DATE_FORMAT__ARGUMENTS[] = "YYYYMMDD";			// date format that will be used in arguments to __EXPIRES__ calls

static_assert(sizeof(__DATE__) == sizeof(DATE_FORMAT__COMPILER), "CodeExpiresFeature::details::DATE_FORMAT__COMPILER should reflect date format used by __DATE__ precompiler macro, but they've got different lengths");

// would be nicer with constexpr std::initializer_list<long> YEAR_CONSTRAINTS, but not everyone has N3471, even though it's dating back to 2012
constexpr long DATES_ARE_NEVER_BEFORE_YEAR_SAFETYCHECK = 2000;
constexpr long DATES_ARE_NEVER_AFTER_YEAR_SAFETYCHECK = 2100;

constexpr long ASSUMED_CENTURY_OF_TWODIGIT_YEARS = 2000;			// MM-DD-YY: 05-30-19 will be 05-30-2019 with this setting
constexpr long YEAR_MULTIPLIER = 10'000;							// These help generating from (year, month, day) tuple a number 
constexpr long MONTH_MULTIPLIER = 100;								//		which can be compared using operator< preserving later-than 
constexpr long DAY_MULTIPLIER = 1;									//		relation -- you can leave them this way

constexpr long ce_strlen(const char* str)
{
	return
		!str ?
		0 :
		*str == '\0' ?
		0 :
		1 + ce_strlen(str + 1);
}

constexpr long FindFirst_s(const char* findIn, long len, char findWhat, long offs = 0)
{
	return
		offs >= len || findIn[offs] == '\0' ? -1
		: findIn[offs] == findWhat ? offs
		: FindFirst_s(findIn, len, findWhat, offs + 1);
}

constexpr long FindLast_s(const char* findIn, long len, char findWhat, long offs = 0)
{
	return
		offs >= len ? -1
		: findIn[len - offs - 1] == findWhat ? len - offs - 1
		: FindLast_s(findIn, len, findWhat, offs + 1);
}

template <size_t arrayLen>
constexpr long GetYearStartIdx(const char(&dateformat)[arrayLen])
{
	return FindFirst_s(dateformat, arrayLen, 'Y');
}

template <size_t arrayLen>
constexpr bool IsYearTwoDigitsLong(const char(&dateformat)[arrayLen])
{
	return FindLast_s(dateformat, arrayLen, 'Y') - FindFirst_s(dateformat, arrayLen, 'Y') == 1;
}

template <size_t arrayLen>
constexpr long GetMonthStartIdx(const char(&dateformat)[arrayLen])
{
	return
		FindFirst_s(dateformat, arrayLen, 'M') != -1 ?
		FindFirst_s(dateformat, arrayLen, 'M') :
		FindFirst_s(dateformat, arrayLen, 'm');
}

template <size_t arrayLen>
constexpr bool IsMonthNumeric(const char(&dateformat)[arrayLen])
{
	return 
		FindLast_s(dateformat, arrayLen, 'M') - FindFirst_s(dateformat, arrayLen, 'M') == 1 ||
		FindLast_s(dateformat, arrayLen, 'm') - FindFirst_s(dateformat, arrayLen, 'm') == 1;
}

template <size_t arrayLen>
constexpr bool IsMonthZeroPadded(const char(&dateformat)[arrayLen])
{
	// which presents will be non-negative, while the other is negative (-1)
	return
		IsMonthNumeric(dateformat) ?
		FindFirst_s(dateformat, arrayLen, 'M') > FindFirst_s(dateformat, arrayLen, 'm') :
		false;
}

template <size_t arrayLen>
constexpr long GetDayStartIdx(const char(&dateformat)[arrayLen])
{
	return 
		FindFirst_s(dateformat, arrayLen, 'D') != -1 ?
		FindFirst_s(dateformat, arrayLen, 'D') :
		FindFirst_s(dateformat, arrayLen, 'd');
}

template <size_t arrayLen>
constexpr bool IsDayZeroPadded(const char(&dateformat)[arrayLen])
{
	// which presents will be non-negative, while the other is negative (-1)
	return
		FindFirst_s(dateformat, arrayLen, 'D') > FindFirst_s(dateformat, arrayLen, 'd');
}

constexpr long ExtractNum(const char* fromWhat, long offset, bool zeroPadded)
{
	return
		zeroPadded ?
		(fromWhat[offset] - '0') * 10 + (fromWhat[offset + 1] - '0') :
		(fromWhat[offset] != ' ' ? fromWhat[offset] : 0) * 10 + (fromWhat[offset + 1] - '0');

}

constexpr long ExtractYear(const char* fromWhat, long yearStart, bool twodigits)
{
	return twodigits ? 
		ASSUMED_CENTURY_OF_TWODIGIT_YEARS + (fromWhat[yearStart] - '0') * 10 + (fromWhat[yearStart + 1] - '0') :
		(fromWhat[yearStart] - '0') * 1000 + (fromWhat[yearStart + 1] - '0') * 100 +
		(fromWhat[yearStart + 2] - '0') * 10 + (fromWhat[yearStart + 3] - '0');
}

// Thanks to lolando and Krzysztof Szewczyk at [https://stackoverflow.com/questions/19760221/c-get-the-month-as-number-at-compile-time] for conversion of abbrev. month names to numbers
constexpr long ExtractMonth(const char* fromWhat, long monthStart, bool numeric, bool zeroPadded)
{
	return 
		numeric ? 
		ExtractNum(fromWhat, monthStart, zeroPadded) :
		fromWhat[monthStart + 2] == 'n' ? (fromWhat[monthStart + 1] == 'a' ? 1 : 6) :
		fromWhat[monthStart + 2] == 'b' ? 2 :
		fromWhat[monthStart + 2] == 'r' ? (fromWhat[monthStart + 0] == 'M' ? 3 : 4) :
		fromWhat[monthStart + 2] == 'y' ? 5 :
		fromWhat[monthStart + 2] == 'l' ? 7 :
		fromWhat[monthStart + 2] == 'g' ? 8 :
		fromWhat[monthStart + 2] == 'p' ? 9 :
		fromWhat[monthStart + 2] == 't' ? 10 :
		fromWhat[monthStart + 2] == 'v' ? 11 :
		12;
}

constexpr long ExtractDay(const char* fromWhat, long dayStart, bool zeroPadded)
{
	return ExtractNum(fromWhat, dayStart, zeroPadded);
}

constexpr bool IsSuspicious(long year, long month, long day)
{
	return
		year < DATES_ARE_NEVER_BEFORE_YEAR_SAFETYCHECK || year > DATES_ARE_NEVER_AFTER_YEAR_SAFETYCHECK ||
		month < 1 || month > 12 ||
		day < 1 || day > 31;
}

constexpr long CompileDateNumber(long year, long month, long day, long setSuspiciosTo)
{
	return
		IsSuspicious(year, month, day) ? 
		setSuspiciosTo :
		YEAR_MULTIPLIER * year + MONTH_MULTIPLIER * month + DAY_MULTIPLIER * day;
}

template <size_t arrayLen>
constexpr bool IsInvalidFormat(const char(&dateformat)[arrayLen])
{
	// of course this can be fooled by e.g. "M M-DD-Y  Y" but it is mainly against accidental format errors
	// and by the way, this format string would work perfectly fine anyway
	return
		arrayLen < 6																						||	// shortest possible valid format
		arrayLen > 32768																					||	// suspiciously long and could lead to signed overflow
		FindFirst_s(dateformat, arrayLen, 'Y') < 0) 														||	// must have YY part
		((FindFirst_s(dateformat, arrayLen, 'M') >= 0) != (FindFirst_s(dateformat, arrayLen, 'm') >= 0))	||	// must have MM xor mm part
		((FindFirst_s(dateformat, arrayLen, 'D') >= 0) != (FindFirst_s(dateformat, arrayLen, 'd') >= 0))	||	// must have DD xor dd part
		FindFirst_s(dateformat, arrayLen, 'y') >= 0) 														||	// small y not allowed
		((FindLast_s(dateformat, arrayLen, 'Y') - FindFirst_s(dateformat, arrayLen, 'Y') != 1) &&
		 (FindLast_s(dateformat, arrayLen, 'Y') - FindFirst_s(dateformat, arrayLen, 'Y') != 3))				||	// year part (YY) must be of size 2 or 4
		((FindLast_s(dateformat, arrayLen, 'M') - FindFirst_s(dateformat, arrayLen, 'M') != 1) &&
		 (FindLast_s(dateformat, arrayLen, 'M') - FindFirst_s(dateformat, arrayLen, 'M') != 2) &&
		 (FindLast_s(dateformat, arrayLen, 'm') - FindFirst_s(dateformat, arrayLen, 'm') != 1) &&
		 (FindLast_s(dateformat, arrayLen, 'm') - FindFirst_s(dateformat, arrayLen, 'm') != 2))				||	// month part (MM or mm) must be of size 2 or 3
		((FindLast_s(dateformat, arrayLen, 'D') - FindFirst_s(dateformat, arrayLen, 'D') != 1) &&
		 (FindLast_s(dateformat, arrayLen, 'd') - FindFirst_s(dateformat, arrayLen, 'd') != 1))				;	// day part (DD or dd) must be of size 2
}

// some precalc values -- may speed up compilation and ease debugging
constexpr long	DATE_FORMAT_ARG_YEARSTART = GetYearStartIdx(DATE_FORMAT__ARGUMENTS);			constexpr long	DATE_FORMAT_COMPILER_YEARSTART = GetYearStartIdx(DATE_FORMAT__COMPILER);
constexpr long	DATE_FORMAT_ARG_MONTHSTART = GetMonthStartIdx(DATE_FORMAT__ARGUMENTS);			constexpr long	DATE_FORMAT_COMPILER_MONTHSTART = GetMonthStartIdx(DATE_FORMAT__COMPILER);
constexpr long	DATE_FORMAT_ARG_DAYSTART = GetDayStartIdx(DATE_FORMAT__ARGUMENTS);				constexpr long	DATE_FORMAT_COMPILER_DAYSTART = GetDayStartIdx(DATE_FORMAT__COMPILER);
constexpr bool	DATE_FORMAT_ARG_Y2DIGITS = IsYearTwoDigitsLong(DATE_FORMAT__ARGUMENTS);			constexpr bool	DATE_FORMAT_COMPILER_Y2DIGITS = IsYearTwoDigitsLong(DATE_FORMAT__COMPILER);
constexpr bool	DATE_FORMAT_ARG_MONTHNUMERIC = IsMonthNumeric(DATE_FORMAT__ARGUMENTS);			constexpr bool	DATE_FORMAT_COMPILER_MONTHNUMERIC = IsMonthNumeric(DATE_FORMAT__COMPILER);
constexpr bool	DATE_FORMAT_ARG_MONTH0PADDED = IsMonthZeroPadded(DATE_FORMAT__ARGUMENTS);		constexpr bool	DATE_FORMAT_COMPILER_MONTH0PADDED = IsMonthZeroPadded(DATE_FORMAT__COMPILER);
constexpr bool	DATE_FORMAT_ARG_DAY0PADDED = IsDayZeroPadded(DATE_FORMAT__ARGUMENTS);			constexpr bool	DATE_FORMAT_COMPILER_DAY0PADDED = IsDayZeroPadded(DATE_FORMAT__COMPILER);
constexpr bool	DATE_FORMAT_ARG_VALID = !IsInvalidFormat(DATE_FORMAT__ARGUMENTS);				constexpr bool	DATE_FORMAT_COMPILER_VALID = !IsInvalidFormat(DATE_FORMAT__COMPILER);

constexpr long	COMPILE_YEAR = ExtractYear(__DATE__, DATE_FORMAT_COMPILER_YEARSTART, DATE_FORMAT_COMPILER_Y2DIGITS);
constexpr long	COMPILE_MONTH = ExtractMonth(__DATE__, DATE_FORMAT_COMPILER_MONTHSTART, DATE_FORMAT_COMPILER_MONTHNUMERIC, DATE_FORMAT_COMPILER_MONTH0PADDED);
constexpr long	COMPILE_DAY = ExtractDay(__DATE__, DATE_FORMAT_COMPILER_DAYSTART, DATE_FORMAT_COMPILER_DAY0PADDED);

constexpr auto	COMPILE_DATE_SUSPICIOUS_VAL = std::numeric_limits<long>::max();
constexpr auto	ARG_DATE_SUSPICIOUS_VAL = std::numeric_limits<long>::min();

constexpr long	COMPILE_DATE = CompileDateNumber(COMPILE_YEAR, COMPILE_MONTH, COMPILE_DAY, COMPILE_DATE_SUSPICIOUS_VAL);

constexpr long ArgDate(const char* argument)
{
	return CompileDateNumber(
		ExtractYear(argument, DATE_FORMAT_ARG_YEARSTART, DATE_FORMAT_ARG_Y2DIGITS),
		ExtractMonth(argument, DATE_FORMAT_ARG_MONTHSTART, DATE_FORMAT_ARG_MONTHNUMERIC, DATE_FORMAT_ARG_MONTH0PADDED),
		ExtractDay(argument, DATE_FORMAT_ARG_DAYSTART, DATE_FORMAT_ARG_DAY0PADDED),
		ARG_DATE_SUSPICIOUS_VAL
	);
}

constexpr bool CheckArg(const char* argument)
{
	return ce_strlen(argument) == sizeof(DATE_FORMAT__ARGUMENTS) - 1;	// rhs includes terminating \0
}

} // namespace details

constexpr bool ExpiresConditionCheck(const char* expiresOnDate)
{
	return
		details::DATE_FORMAT_ARG_VALID && details::DATE_FORMAT_COMPILER_VALID &&
		details::CheckArg(expiresOnDate) &&
		details::COMPILE_DATE < details::ArgDate(expiresOnDate);
}

}	// namespace CodeExpiresFeature

#define __EXPIRES__(expiresOnDate)		static_assert(CodeExpiresFeature::ExpiresConditionCheck(expiresOnDate), "code expired");
