#include <fstream>
#include <iostream>
#include <iostream>
#include <fstream>
class someancestor {
friend DEBUGXRAY::DEBUGCLASS;
};
class outer : private someancestor
{
public:
	outer() = default;
	~outer() {
		class quick_make_a_class_here {
		public:
			~quick_make_a_class_here() {
				std::cout << "this will be called after dtor work\n";
			}
		
friend DEBUGXRAY::DEBUGCLASS;
} instance;
		std::cout << "do some other dtor work\n";
	}
	class inner : public someancestor {
	public:
		~inner() {
			std::cout << "inner class destoryed\n";
		}
	
friend DEBUGXRAY::DEBUGCLASS;
} _inner;

friend DEBUGXRAY::DEBUGCLASS;
};
int main()
{
  class dummy {
  public:
    dummy() = default;
  
friend DEBUGXRAY::DEBUGCLASS;
};
  outer outerinstance;
}

class Base {
public:
	Base() = default;
	~Base() = default;
	

friend DEBUGXRAY::DEBUGCLASS;
};
class Derived1 : public Base {
friend DEBUGXRAY::DEBUGCLASS;
};
class Dervied2 : private Base {
friend DEBUGXRAY::DEBUGCLASS;
};