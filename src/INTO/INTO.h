#pragma once

//TODO implement __DEBUG_CHECK_INTEGER_OVERFLOW_USE_X86_64_ASM routines for mixed types
//TODO integrate __DEBUG_CHECK_INTEGER_OVERFLOW_USE_X86_64_ASM into operators
//TODO make static_assert overflow check work on non-MSVC compilers
//TODO implement other operators & operator members

#include <type_traits>
#include <string>
#include <stdexcept>
#include <limits>
#include <cstdint>

#ifdef __DEBUG_CHECK_INTEGER_OVERFLOW_NAMESPACE
namespace __DEBUG_CHECK_INTEGER_OVERFLOW_NAMESPACE {
#endif

template<typename T, typename U> using INTO_common_t = std::common_type_t<T, U>;

#ifdef __DEBUG_CHECK_INTEGER_OVERFLOW
#define CREATE_TYPE_ALIAS_WITHNAME(type,aliasprefix)	typedef overflowchecked<type> aliasprefix##o;
#else
#define CREATE_TYPE_ALIAS_WITHNAME(type,aliasprefix)	typedef type aliasprefix##o;
#endif
#define CREATE_TYPE_ALIAS(type)		CREATE_TYPE_ALIAS_WITHNAME(type,type)

constexpr bool OVERFLOWCHECK_ON_BY_DEFAULT = true;
constexpr bool SKIP_INITIALIZATION_CHECK = false;

template <typename T>
class overflowchecked;

class INTO_exception : public std::overflow_error {
public:
	INTO_exception(const char* message) :
		std::overflow_error(message) {}
};

namespace detail
{
	template<typename T, bool = std::is_signed<T>::value>
	struct MaximumEncloser_ { typedef intmax_t type; };
	template <typename T>
	struct MaximumEncloser_<T, false> { typedef uintmax_t type; };
}

template <typename T>
using MaximumEncloser_t = typename detail::MaximumEncloser_<T>::type;

template <typename T> [[noreturn]] inline void SignalOverflowError(const overflowchecked<T>* object, std::string errorMsg)
{
	throw INTO_exception(errorMsg.c_str());
}

template <typename T>
class overflowchecked {
private:
	T m_value;
	static bool s_bOverflowCheckActive;
	bool IsOverflowCheckActive() { return s_bOverflowCheckActive; }
	bool SkipInitializationCheck() { return SKIP_INITIALIZATION_CHECK; }
public:
	overflowchecked() = default;
	template <typename U> overflowchecked(U initval) {
		if (!SkipInitializationCheck() && IsOverflowCheckActive())
		{
			const auto _initval_extended = static_cast<MaximumEncloser_t<T>>(initval);
			constexpr auto _min_extended = static_cast<MaximumEncloser_t<T>>(std::numeric_limits<T>::min());
			constexpr auto _max_extended = static_cast<MaximumEncloser_t<T>>(std::numeric_limits<T>::max());
			if (initval != _initval_extended || _initval_extended < _min_extended || _initval_extended > _max_extended)
			{
				SignalOverflowError(this, std::string(typeid(T).name()) + " initialization overflow: " +
					std::to_string(_initval_extended) + " is not in range " + std::to_string(_min_extended) + ".." +
					std::to_string(_max_extended));
			}
		}
		m_value = static_cast<T>(initval);
	}
	operator T () { return m_value; }
	template <typename U, typename V> friend const overflowchecked<INTO_common_t<U, V>> operator+ (overflowchecked<U> lhs, overflowchecked<V> rhs);
	template <typename U, typename V> friend const overflowchecked<INTO_common_t<U, V>> operator- (overflowchecked<U> lhs, overflowchecked<V> rhs);
	template <typename U, typename V> friend const overflowchecked<INTO_common_t<U, V>> operator* (overflowchecked<U> lhs, overflowchecked<V> rhs);
	template <typename U, typename V> friend const overflowchecked<INTO_common_t<U, V>> operator/ (overflowchecked<U> lhs, overflowchecked<V> rhs);
};

#ifdef __DEBUG_CHECK_INTEGER_OVERFLOW_USE_X86_64_ASM
#ifdef _MSC_VER

#if UINTPTR_MAX == 0xffff'ffff'ffff'ffff    // 64-bit mode, assumably
#error Can't do __asm on MSVC/x64
#elif UINTPTR_MAX != 0xffff'ffff			// but not 32-bit mode
#error Strange ptr size
#else
inline bool add_with_oc(uint8_t a, uint8_t b, uint8_t& c)
{
	__asm {
		mov cl, [a]
		mov dl, [b]
		add cl, dl
		setc al
		mov edx, [c]
		mov byte ptr[edx], cl
	}
}
inline bool add_with_oc(int8_t a, int8_t b, int8_t& c)
{
	__asm {
		mov cl, [a]
		mov dl, [b]
		add cl, dl
		seto al
		mov edx, [c]
		mov byte ptr[edx], cl
	}
}
inline bool add_with_oc(uint16_t a, uint16_t b, uint16_t& c)
{
	__asm {
		mov cx, [a]
		mov dx, [b]
		add cx, dx
		setc al
		mov edx, [c]
		mov word ptr[edx], cx
	}
}
inline bool add_with_oc(int16_t a, int16_t b, int16_t& c)
{
	__asm {
		mov cx, [a]
		mov dx, [b]
		add cx, dx
		seto al
		mov edx, [c]
		mov word ptr[edx], cx
	}
}
inline bool add_with_oc(uint32_t a, uint32_t b, uint32_t& c)
{
	__asm {
		mov ecx, [a]
		mov edx, [b]
		add ecx, edx
		setc al
		mov edx, [c]
		mov[edx], ecx
	}
}
inline bool add_with_oc(int32_t a, int32_t b, int32_t& c)
{
	__asm {
		mov ecx, [a]
		mov edx, [b]
		add ecx, edx
		seto al
		mov edx, [c]
		mov[edx], ecx
	}
}
inline bool add_with_oc(uint64_t a, uint64_t b, uint64_t& c)
{
	__asm {
		push esi
		push edi
		mov eax, dword ptr[a]
		mov edx, dword ptr[a + 4]
		mov esi, dword ptr[b]
		mov edi, dword ptr[b + 4]
		add eax, esi
		adc edx, edi
		setc cl
		mov esi, [c]
		mov dword ptr[esi], eax
		mov dword ptr[esi + 4], edx
		pop edi
		pop esi
		mov al, cl
	}
}
inline bool add_with_oc(int64_t a, int64_t b, int64_t& c)
{
	__asm {
		push esi
		push edi
		mov eax, dword ptr[a]
		mov edx, dword ptr[a + 4]
		mov esi, dword ptr[b]
		mov edi, dword ptr[b + 4]
		add eax, esi
		adc edx, edi
		seto cl
		mov esi, [c]
		mov dword ptr[esi], eax
		mov dword ptr[esi + 4], edx
		pop edi
		pop esi
		mov al, cl
	}
}
#endif //UINTPTR_MAX == 0xffff'ffff / 32-bit mode
#else //!_MSC_VER
inline bool add_with_oc(uint8_t a, uint8_t b, uint8_t& c)
{
	bool retval;
	__asm__ volatile (
		"addb %%dl, %%cl    \n"
		"setc %%al          \n"
		: "=c" (c), "=a" (retval)
		: "c" (a), "d" (b)
		: );
	return retval;
}
inline bool add_with_oc(int8_t a, int8_t b, int8_t& c)
{
	bool retval;
	__asm__ volatile (
		"addb %%dl, %%cl    \n"
		"seto %%al          \n"
		: "=c" (c), "=a" (retval)
		: "c" (a), "d" (b)
		: );
	return retval;
}
inline bool add_with_oc(uint16_t a, uint16_t b, uint16_t& c)
{
	bool retval;
	__asm__ volatile (
		"addw %%dx, %%cx    \n"
		"setc %%al          \n"
		: "=c" (c), "=a" (retval)
		: "c" (a), "d" (b)
		: );
	return retval;
}
inline bool add_with_oc(int16_t a, int16_t b, int16_t& c)
{
	bool retval;
	__asm__ volatile (
		"addw %%dx, %%cx    \n"
		"seto %%al          \n"
		: "=c" (c), "=a" (retval)
		: "c" (a), "d" (b)
		: );
	return retval;
}
inline bool add_with_oc(uint32_t a, uint32_t b, uint32_t& c)
{
	bool retval;
	__asm__ volatile (
		"addl %%edx, %%ecx  \n"
		"setc %%al          \n"
		: "=c" (c), "=a" (retval)
		: "c" (a), "d" (b)
		: );
	return retval;
}
inline bool add_with_oc(int32_t a, int32_t b, int32_t& c)
{
	bool retval;
	__asm__ volatile (
		"addl %%edx, %%ecx  \n"
		"seto %%al          \n"
		: "=c" (c), "=a" (retval)
		: "c" (a), "d" (b)
		: );
	return retval;
}
inline bool add_with_oc(uint64_t a, uint64_t b, uint64_t& c)
{
	bool retval;
#if UINTPTR_MAX == 0xffff'ffff'ffff'ffff    // 64-bit mode, assumably
	{
		__asm__ volatile (
			"addq %%rdx, %%rcx  \n"
			"setc %%al          \n"
			: "=c" (c), "=a" (retval)
			: "c" (a), "d" (b)
			: );
	}
#elif UINTPTR_MAX == 0xffff'ffff    // 32-bit mode
	{
		__asm__ volatile (
			"addl %%esi, %%eax   \n"
			"adcl %%edi, %%edx   \n"
			"setc %%cl           \n"
			: "=A" (c), "=c" (retval)
			: "a" ((uint32_t)a), "d" ((uint32_t)(a >> 32)),
			  "S" ((uint32_t)b), "D" ((uint32_t)(b >> 32))
			: );
	}
#else
#error Strange ptr size
#endif
	return retval;
}

inline bool add_with_oc(int64_t a, int64_t b, int64_t& c)
{
	bool retval;
#if UINTPTR_MAX == 0xffff'ffff'ffff'ffff    // 64-bit mode, assumably
	{
		__asm__ volatile (
			"addq %%rdx, %%rcx  \n"
			"seto %%al          \n"
			: "=c" (c), "=a" (retval)
			: "c" (a), "d" (b)
			: );
	}
#elif UINTPTR_MAX == 0xffff'ffff    // 32-bit mode
	{
		__asm__ volatile (
			"addl %%esi, %%eax   \n"
			"adcl %%edi, %%edx   \n"
			"seto %%cl           \n"
			: "=A" (c), "=c" (retval)
			: "a" ((uint32_t)a), "d" ((uint32_t)(a >> 32)),
			  "S" ((uint32_t)b), "D" ((uint32_t)(b >> 32))
			: );
	}
#else
#error Strange ptr size
#endif
	return retval;
}

#endif //!_MSC_VER
#endif //__DEBUG_CHECK_INTEGER_OVERFLOW_USE_X86_ASM

template <typename T> bool overflowchecked<T>::s_bOverflowCheckActive = OVERFLOWCHECK_ON_BY_DEFAULT;

// The method for checking addition and subtraction overflow here exploits the fact that multiple overflows 
// cannot occur in these operations, thus, the distance between the exact algebraic sum/difference
// is never greater than the total range of the type, so, if an overflow occurs, the (truncated) result will 
// be on the wrong side of the addend/minuend (in the sense of standard ordering). 
// It is range-agnostic, but relies heavily on modulo wrap-around overflow behavior, which is defined only 
// in case of unsigned intergers, but is the de facto overflow behavior in both the signed & unsigned cases 
// for most C/C++ compilers and platforms nowadays. A static_assert() check for this is included though.
template <typename U, typename V> const overflowchecked<INTO_common_t<U, V>> operator+ (overflowchecked<U> lhs, overflowchecked<V> rhs)
{
	using common_type = INTO_common_t<U, V>;
#ifdef _MSC_VER
	static_assert(static_cast<U>(std::numeric_limits<U>::max() + 1) == std::numeric_limits<U>::min(), "type U does not exhibit wrap-around overflow behavior");
	static_assert(static_cast<V>(std::numeric_limits<V>::max() + 1) == std::numeric_limits<V>::min(), "type V does not exhibit wrap-around overflow behavior");
	static_assert(static_cast<common_type>(std::numeric_limits<common_type>::max() + 1) == std::numeric_limits<common_type>::min(), "common type for U+V does not exhibit wrap-around overflow behavior");
#endif
	const bool bBothCheckActive = lhs.IsOverflowCheckActive() && rhs.IsOverflowCheckActive();
	const auto nakedResult = static_cast<common_type>(lhs.m_value + rhs.m_value);
	const bool rhsNonNeg = rhs.m_value >= 0;	const bool rhsNegative = !rhsNonNeg;
	if (bBothCheckActive)
	{
		constexpr auto lowerBound = std::numeric_limits<common_type>::min();
		constexpr auto upperBound = std::numeric_limits<common_type>::max();
		if (rhsNonNeg && nakedResult < lhs.m_value)
		{
			SignalOverflowError(&lhs, (std::is_same<U, V>::value ?
				std::string(typeid(U).name()) :
				std::string(typeid(U).name()) + ", " + typeid(V).name() + " [common:" + typeid(common_type).name() + "]") +
				" op+ overflow: " + std::to_string(lhs.m_value) + "+" + std::to_string(rhs.m_value) + " > " + std::to_string(upperBound));
		}
		if (rhsNegative && nakedResult >= lhs.m_value)
		{
			SignalOverflowError(&lhs, (std::is_same<U, V>::value ?
				std::string(typeid(U).name()) :
				std::string(typeid(U).name()) + ", " + typeid(V).name() + " [common:" + typeid(common_type).name() + "]") +
				" op+ overflow: " + std::to_string(lhs.m_value) + "+" + std::to_string(rhs.m_value) + " < " + std::to_string(lowerBound));
		}
	}
	return overflowchecked<common_type>(nakedResult);
}

template <typename U, typename V> const overflowchecked<INTO_common_t<U, V>> operator- (overflowchecked<U> lhs, overflowchecked<V> rhs)
{
	using common_type = INTO_common_t<U, V>;
	static_assert(static_cast<U>(std::numeric_limits<U>::max() + 1) == std::numeric_limits<U>::min(), "type U does not exhibit wrap-around overflow behavior");
	static_assert(static_cast<V>(std::numeric_limits<V>::max() + 1) == std::numeric_limits<V>::min(), "type V does not exhibit wrap-around overflow behavior");
	static_assert(static_cast<common_type>(std::numeric_limits<common_type>::max() + 1) == std::numeric_limits<common_type>::min(), "common type for U+V does not exhibit wrap-around overflow behavior");
	const bool bBothCheckActive = lhs.IsOverflowCheckActive() && rhs.IsOverflowCheckActive();
	const auto nakedResult = static_cast<common_type>(lhs.m_value - rhs.m_value);
	const bool rhsNonNeg = rhs.m_value >= 0;	const bool rhsNegative = !rhsNonNeg;
	if (bBothCheckActive)
	{
		constexpr auto lowerBound = std::numeric_limits<common_type>::min();
		constexpr auto upperBound = std::numeric_limits<common_type>::max();
		if (rhsNegative && nakedResult <= lhs.m_value)
		{
			SignalOverflowError(&lhs, (std::is_same<U, V>::value ?
				std::string(typeid(U).name()) :
				std::string(typeid(U).name()) + ", " + typeid(V).name() + " [common:" + typeid(common_type).name() + "]") +
				" op- overflow: " + std::to_string(lhs.m_value) + "-" + std::to_string(rhs.m_value) + " > " + std::to_string(upperBound));
		}
		if (rhsNonNeg && nakedResult > lhs.m_value)
		{
			SignalOverflowError(&lhs, (std::is_same<U, V>::value ?
				std::string(typeid(U).name()) :
				std::string(typeid(U).name()) + ", " + typeid(V).name() + " [common:" + typeid(common_type).name() + "]") +
				" op- overflow: " + std::to_string(lhs.m_value) + "-" + std::to_string(rhs.m_value) + " < " + std::to_string(lowerBound));
		}
	}
	return overflowchecked<common_type>(nakedResult);
}

// The technique that's been used here is based on the irreversibility of a multiplication in the presence 
// of an overflow (with one exception: the case of signed INT_MIN * (-1) == INT_MIN can be reversed).
// This is not true though for addition/subtraction in the usual wrap-around overflow scenario, so it can't
// be used there. Despite its superficial simpleness, this method is usually slower thean than the one used 
// in the addition/subtraction case, caused mainly by the div/idiv instruction involved in the check requiring 
// an order of magnitude more CPU cycles to execute than ordinary arithmetic or comparison instructions.
// However, the ordering-based approach that has proven useful in the addition/subtraction case cannot be used 
// here: e.g. 32*10==64 holds for the usual 8-bit signed char type, obviously because of overflow, but the result 
// is on the right side of both the multipliers (in fact multiple overflows occurred here, and that's why the 
// result can be greater than both 32 and 10 in this case).
template <typename U, typename V> const overflowchecked<INTO_common_t<U, V>> operator* (overflowchecked<U> lhs, overflowchecked<V> rhs)
{
	using common_type = INTO_common_t<U, V>;
	const bool bBothCheckActive = lhs.IsOverflowCheckActive() && rhs.IsOverflowCheckActive();
	const auto nakedResult = static_cast<common_type>(lhs.m_value - rhs.m_value);

	if (bBothCheckActive)
	{
		constexpr auto lowerBound = std::numeric_limits<common_type>::min();
		constexpr auto upperBound = std::numeric_limits<common_type>::max();
		if ((lhs.m_value == -1 && rhs.m_value == nakedResult) ||
			(lhs.m_value == nakedResult && rhs.m_value == -1) ||
			(rhs.m_value != 0 && nakedResult / rhs.m_value != lhs.m_value))
		{
			SignalOverflowError(&lhs, (std::is_same<U, V>::value ?
				std::string(typeid(U).name()) :
				std::string(typeid(U).name()) + ", " + typeid(V).name() + " [common:" + typeid(common_type).name() + "]") +
				" op* overflow: " + std::to_string(lhs.m_value) + "*" + std::to_string(rhs.m_value) + " does not fit in range " +
				std::to_string(lowerBound) + ".." + std::to_string(upperBound));
		}
	}
	return overflowchecked<common_type>(nakedResult);
}
// An easy case: overflow can only occur in one case: if INT_MIN / (-1) < INT_MAX
template <typename U, typename V> const overflowchecked<INTO_common_t<U, V>> operator/ (overflowchecked<U> lhs, overflowchecked<V> rhs)
{
	using common_type = INTO_common_t<U, V>;
	const bool bBothCheckActive = lhs.IsOverflowCheckActive() && rhs.IsOverflowCheckActive();
	const auto nakedResult = static_cast<common_type>(lhs.m_value - rhs.m_value);

	if (bBothCheckActive)
	{
		constexpr auto lowerBound = std::numeric_limits<common_type>::min();
		constexpr auto upperBound = std::numeric_limits<common_type>::max();
		if ((lhs.m_value == -1 && rhs.m_value == nakedResult) ||
			(lhs.m_value == nakedResult && rhs.m_value == -1))
		{
			SignalOverflowError(&lhs, (std::is_same<U, V>::value ?
				std::string(typeid(U).name()) :
				std::string(typeid(U).name()) + ", " + typeid(V).name() + " [common:" + typeid(common_type).name() + "]") +
				" op/ overflow: " + std::to_string(lhs.m_value) + "/" + std::to_string(rhs.m_value) + " does not fit in range " +
				std::to_string(lowerBound) + ".." + std::to_string(upperBound));
		}
	}
	return overflowchecked<common_type>(nakedResult);
}

#ifdef __DEBUG_CHECK_INTEGER_OVERFLOW_NAMESPACE
} // namespace __DEBUG_CHECK_INTEGER_OVERFLOW_NAMESPACE
#endif

// namespace ends here, the followings are typedefs in global namespace (if enabled)

#ifdef __DEBUG_CHECK_INTEGER_OVERFLOW_ALIAS
CREATE_TYPE_ALIAS(unsigned);								// this one creates unsignedo
CREATE_TYPE_ALIAS(signed);									//		...signedo
CREATE_TYPE_ALIAS(char);
CREATE_TYPE_ALIAS(short);
CREATE_TYPE_ALIAS(int);
CREATE_TYPE_ALIAS(long);									//		...longo
CREATE_TYPE_ALIAS_WITHNAME(signed char,			schar);		//		...scharo
CREATE_TYPE_ALIAS_WITHNAME(unsigned char,		uchar);
CREATE_TYPE_ALIAS_WITHNAME(unsigned short,		ushort);
CREATE_TYPE_ALIAS_WITHNAME(unsigned int,		uint);
CREATE_TYPE_ALIAS_WITHNAME(long,				ulong);
CREATE_TYPE_ALIAS_WITHNAME(long long,			llong);
CREATE_TYPE_ALIAS_WITHNAME(unsigned long long,	ullong);	//		...ullongo
#endif //__DEBUG_CHECK_INTEGER_OVERFLOW_ALIAS

#undef CREATE_TYPE_ALIAS
#undef CREATE_TYPE_ALIAS_WITHNAME

