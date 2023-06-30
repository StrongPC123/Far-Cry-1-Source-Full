#include "RenderPCH.h"
#include "nvparse.h"


nvparse_errors::nvparse_errors()
{
	num_errors = 0;
	reset();
}

nvparse_errors::~nvparse_errors()
{
	reset();
}


void nvparse_errors::reset()
{
	for(int i=0; i < num_errors; i++)
	    free(elist[i]);//FIXME detail_nmap something is writing 0x2 to elist[1] blah!
	for(int j=0; j <= NVPARSE_MAX_ERRORS; j++)
		elist[j] = 0;
	num_errors = 0;
}

extern char *gShObjectNotFound;

void nvparse_errors::set(const char * e)
{
	if(num_errors < NVPARSE_MAX_ERRORS)
  {
    char buf[512];
    if (gShObjectNotFound)
    {
      sprintf(buf, "(%s) %s", gShObjectNotFound, e);
      elist[num_errors++] = Cry_strdup(buf);
    }
    else
    {
		  elist[num_errors++] = Cry_strdup(e);
    }
  }
}

void nvparse_errors::set(const char * e, int line_number)
{
	char buff[256];
	sprintf(buff, "error on line %d: %s", line_number, e);
	if(num_errors < NVPARSE_MAX_ERRORS)
		elist[num_errors++] = Cry_strdup(buff);
}

char * const * const nvparse_errors::get_errors()
{
	return elist;
}
