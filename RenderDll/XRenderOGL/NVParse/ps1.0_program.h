#ifndef _PS10_PROGRAM_H
#define _PS10_PROGRAM_H

namespace ps10
{

	struct constdef
	{
		string reg;
		float r,g,b,a;
	};


	void invoke(std::vector<constdef> * c,
		        std::list<std::vector<string> > * a,
				std::list<std::vector<string> > * b);
	
	bool init_extensions();
}

#endif
