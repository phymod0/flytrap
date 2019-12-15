#include <functional>
#include <iostream>
#include <string>


class Base
{
      public:
	std::function<void(void)> printer;
};


class Derived : public Base
{
      public:
	int magic;
	Derived()
	{
		constexpr int lol = 4321;
		magic = lol;
		printer = [&]() {
			std::cout << this->magic;
			std::cout << std::endl;
			++this->magic;
		};
	};
};


void functionTakingBase(const Base& b)
{
	b.printer();
	b.printer();
	b.printer();
}


int main()
{
	Derived d;
	functionTakingBase(d);
	std::cout << d.magic << std::endl;

	std::string str1 = "Hello world!\n";
	std::string str2 = std::move(str1);

	std::cout << str2;

	return 0;
}
