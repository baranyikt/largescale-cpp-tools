#include <fstream>
#include <iostream>
#include <iostream>
#include <fstream>
class someancestor {};
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
		} instance;
		std::cout << "do some other dtor work\n";
	}
	class inner : public someancestor {
	public:
		~inner() {
			std::cout << "inner class destoryed\n";
		}
	} _inner;
};
int main()
{
  class dummy {
  public:
    dummy() = default;
  };
  outer outerinstance;
}

class Base {
public:
	Base() = default;
	~Base() = default;
	
};
class Derived1 : public Base {};
class Dervied2 : private Base {};