#include "RenderPCH.h"
#include "nvparse.h"


#include "rc1.0_combiners.h"

void CombinersStruct::Validate()
{
	if (2 == numConsts &&
		cc[0].reg.bits.name == cc[1].reg.bits.name)
		errors.set("global constant set twice");

	generals.Validate(numConsts, &cc[0]);

	final.Validate();
}

void CombinersStruct::Invoke()
{
	for (int i = 0; i < numConsts; i++)
		glCombinerParameterfvNV(cc[i].reg.bits.name, &(cc[i].v[0]));

	generals.Invoke();

	final.Invoke();
}

bool is_rc10(const char * s)
{
	return ! strncmp(s, "!!RC1.0", 7);
}


bool rc10_init_more()
{
	bool rcinit = false;
	if(rcinit == false)
	{
		if(!SUPPORTS_GL_NV_register_combiners && !SUPPORTS_GL_ATI_fragment_shader)
		{
			errors.set("unable to initialize GL_NV_register_combiners or GL_ATI_fragment_shader\n");
			return false;
		}
		else
		{
			rcinit = true;
		}
	}

  bool rc2init = true;
	
	errors.reset();
	line_number = 1;

	return true;
}
