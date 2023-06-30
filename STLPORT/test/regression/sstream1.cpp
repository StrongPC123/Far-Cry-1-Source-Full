// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <sstream>
#include <cassert>

#ifdef MAIN 
#define sstream1_test main
#endif

int sstream1_test(int, char**)
{
    
    std::istringstream iss;
	int x[9];
	std::fill_n(x, 9, 999);
	iss.str(std::string("0-0+0 010-010+010 0x1-0x1+0x1"));
	iss >> std::hex;
	iss >> x[0] >> x[1] >> x[2] >> x[3] >> x[4] >> x[5] >> x[6] >> x[7] >> x[8];
	assert(x[0] == 0x0);
	assert(x[1] == -0x0);
	assert(x[2] == +0x0);
	assert(x[3] == 0x010);
	assert(x[4] == -0x010);
	assert(x[5] == +0x010);
	assert(x[6] == 0x1);
	assert(x[7] == -0x1);
	assert(x[8] == +0x1);
	
	iss.clear();
	iss.str(std::string("0-0+0 010-010+010 0x1-0x1+0x1"));
	iss.unsetf(std::ios_base::dec | std::ios_base::hex);
	std::fill_n(x, 9, 999);
	iss >> x[0] >> x[1] >> x[2] >> x[3] >> x[4] >> x[5] >> x[6] >> x[7] >> x[8];
	assert(x[0] == 0);
	assert(x[1] == -0);
	assert(x[2] == +0);
	assert(x[3] == 010);
	assert(x[4] == -010);
	assert(x[5] == +010);
	assert(x[6] == 0x1);
	assert(x[7] == -0x1);
	assert(x[8] == +0x1);
}

