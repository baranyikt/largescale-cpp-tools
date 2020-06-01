# largescale-C++\-tools
### Tools made with the purpose of helping to maintain large-scale C++ codebases
- EXPIRES -- for marking code parts that are planned to be used temporarily only
- DEBUGFRIEND -- makes one class be friend of all others in a large project
- INTO -- defines temporary replacement types for integers with  well-defined (and reporting) overflow behavior both in signed and unsigned case (named after the original  8086 instruction made just for that)
- FLOATO -- small tool for turning on/off floating point exceptions for the whole project
- OPNEW_REPLACER -- finds all _new_ and _delete_ calls in a project and replaces them with given statements -- useful for introducing project-wide memory manager, without the need of overriding (global) operator new and operator delete

## EXPIRES
Some codes are intended to be temporary solutions for a certain problem. Often enough, these codes are forgotten and persist unintentionally in the code base, surviving much longer time than the originally planned lifetime. \_\_EXPIRES__ macro is designed to be the cure for that. It uses the built-in\_\_DATE__ preprocessor directive to get the compilation date and compares it with the date argument it's given. If it's over the given date, it stops compilation with an error.

For example:
```c++
__EXPIRES__("20191010");			// using YYYYMMDD format (customizable)
```
compiles fine until Oct 9, 2019 but will fail to compile on Oct 10 and onwards. 

Of course, it's got no runtime overhead, after all, it's just a static_assert() call.

Header-only, you can find everything you need in expires/expires.h

## DEBUGFRIEND
Temporarily added codes for debug dumping & value verification often hit the pitfall of OOP encapsulation, i.e. private members cannot be dumped from outside, which is good 99% of time, but in some cases causes headaches while debugging -- especially in larger projects. DEBUGFRIEND tries to fill this gap by adding `friend DEBUGXRAY::DEBUGCLASS;` to all class definitions in the project, for example turns this class definition
```c++
class Olive {
public:
	Olive() = default;
    ~Olive();
private:
	bool _isPitted;
};
```
into this
```c++
class Olive {
public:
	Olive() = default;
    ~Olive();
private:
	bool _isPitted;
    friend DEBUGXRAY::DEBUGCLASS;
};
```
and then you can extend DEBUGCLASS to get what you're curious about:
```c++
namespace DEBUGXRAY {
class DEBUGCLASS {
public:
	// ...
    static bool IsOlivePitted(const Olive& o) { return o._isPitted; }
    // ...
};
} // namespace
```
and use that anywhere in the code:
```c++
bool seedInsideOlive = !(DEBUGXRAY::DEBUGCLASS::IsOlivePitted(olive));
```

[classdecl_modifier.cpp: expects one inputfile, one outputfile, analyzes inputfile, searches for class definitions and extends them -- needs libclang for parsing C++ source; debugxray.h: skeleton definition file for DEBUGXRAY::DEBUGCLASS]

## INTO
INTO is a lightweight header-only library that defines a set of standard integer type wrappers with overloaded arithmetic operators that take care of signed and unsigned integer overflows. It also provides typedefs to be able to switch back and forth between overflow checked and built-in versions. 
It got it's name after the original 8086/8088 assembly instruction INTO (opcode 0xCE) that calls interrupt 4 if overflow bit is set in [E]FLAGS. 

Simple usage example:
```c++
unsignedo x = 32767;				// or overflowchecked<unsigned> x, if __DEBUG_CHECK_INTEGER_OVERFLOW_ALIAS is OFF
x += 1;						// this throws std::overflow_error depending on whether __DEBUG_CHECK_INTEGER_OVERFLOW is ON or OFF
```
Typedef aliases look something like this:
```c++
#ifdef __DEBUG_CHECK_INTEGER_OVERFLOW
typedef overflowchecked<unsigned> unsignedo;
#else
typedef unsigned unsignedo;
#endif
```
It also checks for variable initialization and that's how it can trap overflows concerning non-operator cases, like calling pow() or other cmath functions:
```c++
into i = 32768;					// this alone would cause std::overflow_error if it was shorto, rather then into
into result = pow(i,3);				// pow(32768,3) == 35184372088832 > 2147483648, this will be and std::overflow_error
```
