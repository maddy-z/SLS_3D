#include <cstdio>
#include <cstdlib>

#include <iostream>

void ShowBinary(unsigned int num)
{
	unsigned int v;

	for (int i = 31; i >= 0; --i) {
		v = (num >> i) & 0x1;

		if ( v ) { std::cout << "1"; }
		else { std::cout << "0"; }
	}

	std::cout << std::endl;
}

unsigned int Binary2Gray(unsigned int b)
{
	unsigned int ans = (b >> 31);
	
	for (int i = 30; i >= 0 ; --i) 
	{
		ans <<= 1;
		ans |= ((b >> i) ^ (b >> (i+1))) & 0x1;
	}

	return ans;
}

unsigned int Gray2Binary(unsigned int g)
{
	unsigned int ans = (g >> 31);

	for (int i = 30; i >= 0; --i) {
		ans <<= 1;
		ans |= ((g >> i) ^ (ans >> 1)) & 0x1;
	}

	return ans;
}

// 
// Main Function
// 

int main(int argc, char ** argv)
{
	for (int i = 0; i < 16; ++i) 
	{
		std::cout << "Origin:\t" ; ShowBinary(i);
		std::cout << "B2G:\t" ; ShowBinary(Binary2Gray(i));
		std::cout << "G2B:\t"; ShowBinary(Gray2Binary(Binary2Gray(i)));
	}

	ShowBinary(0xffffffff);
	ShowBinary(Binary2Gray(0xffffffff));
	ShowBinary(Gray2Binary(Binary2Gray(0xffffffff)));
	
	std::cout << std::endl;
	
	system("pause");
	exit(EXIT_SUCCESS);

}