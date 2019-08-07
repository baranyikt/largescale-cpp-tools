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
[coming soon] INTO consists of a) a set of standard integer type wrappers with overloaded arithmetic operators that take care of signed and unsigned integer overflows, and b) typedefs optionally referring to them or the original built-in ones like this:
```c++
#ifdef __DEBUG_CHECK_INTEGER_OVERFLOW
typedef overflowchecked<unsigned> unsignedo;
#else
typedef unsigned unsignedo;
#endif
```
while the classes with overloaded operators look something like this (simplified template syntax code):
```c++
class overflowchecked<unsigned> {
private:
	unsigned _wrapped;
public:
	// ctors and conversion operators
	friend overflowchecked<unsigned> operator+(overflowchecked<unsigned> lhs, overflowchecked<unsigned> rhs) {
		if (std::numeric_limits<unsigned>::max() - lhs._wrapped < rhs._wrapped)
			throw std::overflow_error("unsigned integer overflow");
    return overflowchecked<unsigned>(lhs._wrapped + rhs._wrapped);
    }
};
```

It is named after original 8086/8088 assembly instruction INTO (opcode 0xCE) which calls interrupt 4 if it finds overflow bit is set in FLAGS register.
