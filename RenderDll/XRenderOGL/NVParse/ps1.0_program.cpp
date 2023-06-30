#include "RenderPCH.h"
#include "nvparse.h"

#include "ps1.0_program.h"


//using namespace std;
using namespace ps10;


#define DBG_MESG(msg, line)  	errors.set(msg, line)
//#define DBG_MESG(msg, line)

namespace
{
	struct set_constants
	{
		void operator() (constdef c)
		{
			if(c.reg[0] != 'c' && c.reg.size() != 2)
				DBG_MESG("def line must use constant registers", 0);
			int reg = c.reg[1] - '0';
			GLenum stage = GL_COMBINER0_NV + (reg / 2);
			GLenum cclr  = GL_CONSTANT_COLOR0_NV + (reg % 2);

			GLfloat cval[4];
			cval[0] = c.r;
			cval[0] = c.g;
			cval[0] = c.b;
			cval[0] = c.a;
			glCombinerStageParameterfvNV(stage, cclr, cval);
		}
	};

	GLenum get_tex_target()
	{
		if(glIsEnabled(GL_TEXTURE_CUBE_MAP_ARB))
			return GL_TEXTURE_CUBE_MAP_ARB;
		if(glIsEnabled(GL_TEXTURE_3D))
			return GL_TEXTURE_3D;
		if(glIsEnabled(GL_TEXTURE_RECTANGLE_NV))
			return GL_TEXTURE_RECTANGLE_NV;
		if(glIsEnabled(GL_TEXTURE_2D))
			return GL_TEXTURE_2D;
		if(glIsEnabled(GL_TEXTURE_1D))
			return GL_TEXTURE_1D;

		//otherwise make the op none...
		return GL_NONE;
	}


	struct set_texture_shaders
	{
    set_texture_shaders(std::vector<constdef> * cdef)
		{
			for(stage = 0; stage < 4; stage++)
			{
				glActiveTextureARB(GL_TEXTURE0_ARB + stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
			}
			stage = 0;
			c = cdef;
		}

    void operator() (std::vector<string> & instr)
		{
			if(stage > 3)
				return;
			glActiveTextureARB(GL_TEXTURE0_ARB + stage);

			string op = instr[0];
			if(op == "tex")
			{
				if(instr.size() != 2)
					fprintf(stderr,"incorrect \"tex\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, get_tex_target());
			}
			else if(op == "texbem")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texbem\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_OFFSET_TEXTURE_2D_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texbem\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texbeml")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texbeml\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_OFFSET_TEXTURE_2D_SCALE_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texbeml\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texcoord")
			{
				if(instr.size() != 2)
					fprintf(stderr,"incorrect \"texcoord\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_PASS_THROUGH_NV);
			}
			else if(op == "texkill")
			{
				if(instr.size() != 2)
					fprintf(stderr,"incorrect \"texkill\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
			}
			else if(op == "texm3x2pad")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x2pad\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_NV);
				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x2pad\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x2tex")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x2tex\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV);
				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x2tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3pad")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3pad\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_NV);
				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3pad\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3tex")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;

				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV);

				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3spec")
			{
				if(instr.size() != 4 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3spec\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;

				if(! c)
					return;
				constdef cd;
				for(int i = c->size()-1; i >= 0; i--)
				{
					cd = (*c)[i];
					if(cd.reg == "c0")
						break;
				}

				if(cd.reg != string("c0") || instr[3] != string("c0"))
					return;
				GLfloat eye[4];
				eye[0] = cd.r;
				eye[1] = cd.g;
				eye[2] = cd.b;
				eye[3] = cd.a;

				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV);
				glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_CONST_EYE_NV, eye);

				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3vspec")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3vspec\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;

				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV);

				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texreg2ar")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texreg2ar\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DEPENDENT_AR_TEXTURE_2D_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texreg2ar\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texreg2gb")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texreg2gb\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texreg2gb\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			stage++;
		}

    std::map<string, int> reg2stage;
		int stage;
    std::vector<constdef> * c;
	};
	
  GLenum reg_enum(string s)
	{
		if(s == "c0")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c1")
			return GL_CONSTANT_COLOR1_NV;
		else if(s == "c2")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c3")
			return GL_CONSTANT_COLOR1_NV;
		else if(s == "c4")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c5")
			return GL_CONSTANT_COLOR1_NV;
		else if(s == "c6")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c7")
			return GL_CONSTANT_COLOR1_NV;
		else if(s == "t0")
			return GL_TEXTURE0_ARB;
		else if(s == "t1")
			return GL_TEXTURE1_ARB;
		else if(s == "t2")
			return GL_TEXTURE2_ARB;
		else if(s == "t3")
			return GL_TEXTURE3_ARB;
		else if(s == "v0")
			return GL_PRIMARY_COLOR_NV;
		else if(s == "v1")
			return GL_SECONDARY_COLOR_NV;
		else if(s == "r0")
			return GL_SPARE0_NV;
		else if(s == "r1")
			return GL_SPARE1_NV;
		else // ??
			return GL_DISCARD_NV;
	}

	struct src
	{
    src(string s)
		{
			init(s);
		}


    void init(string s)
		{
			arg = s;
			comp = GL_RGB;
			map = GL_SIGNED_IDENTITY_NV;

			int offset;
      if((offset = s.find(".a")) != string::npos)
			{
				comp = GL_ALPHA;
				s.erase(offset, offset+2);
			}

			bool negate = false;

			if(s[0] == '1')
			{
				s.erase(0, 1);
				while(s[0] == ' ')
					s.erase(0,1);
				if(s[0] == '-')
					s.erase(0,1);
				while(s[0] == ' ')
					s.erase(0,1);
				map = GL_UNSIGNED_INVERT_NV;
			}
			else if(s[0] == '-')
			{
				s.erase(0, 1);
				while(s[0] == ' ')
					s.erase(0,1);
				negate = true;
				map = GL_UNSIGNED_INVERT_NV;
			}

			bool half_bias = false;
			bool expand = false;
      if(s.find("_bias") != string::npos)
			{
				s.erase(s.find("_bias"), 5);
				half_bias = true;
			}
      else if(s.find("_bx2") != string::npos)
			{
				s.erase(s.find("_bx2"), 4);
				expand = true;
			}
			
			if(expand)
			{
				if(negate)
					map = GL_EXPAND_NEGATE_NV;
				else
					map = GL_EXPAND_NORMAL_NV;
			}
			else if(half_bias)
			{
				if(negate)
					map = GL_HALF_BIAS_NEGATE_NV;
				else
					map = GL_HALF_BIAS_NORMAL_NV;
			}
			reg = reg_enum(s);
		}

		string arg;
		GLenum reg;
		GLenum map;
		GLenum comp;
	};


	struct set_register_combiners
	{
		set_register_combiners()
		{
			combiner = 0;
		}

    void operator() (std::vector<string> & instr)
		{
			string op;
			GLenum scale;
			bool op_sat = false;
			op = instr[0];
			int offset;
      if((offset = op.find("_x2")) != string::npos)
			{
				scale = GL_SCALE_BY_TWO_NV;
				op.erase(op.begin()+offset, op.begin()+offset+3);
			}
      else if((offset = op.find("_x4")) != string::npos)
			{
				scale = GL_SCALE_BY_FOUR_NV;
				op.erase(op.begin()+offset, op.begin()+offset+3);
			}
      else if((offset = op.find("_d2")) != string::npos)
			{
				scale = GL_SCALE_BY_ONE_HALF_NV;
				op.erase(op.begin()+offset, op.begin()+offset+3);
			}

      if((offset = op.find("_sat")) != string::npos)
			{
				// need to actually use this...
				op_sat = true;
				op.erase(op.begin()+offset, op.begin()+offset+4);
			}
			
      string dst = instr[1];
			int mask = GL_RGBA;
      if((offset = dst.find(".rgba")) != string::npos)
			{
				dst.erase(offset, offset + 5);
			}
			else if((offset = dst.find(".rgb")) != string::npos)
			{
				dst.erase(offset, offset + 4);
				mask = GL_RGB;
			}
      else if((offset = dst.find(".a")) != string::npos)
			{
				dst.erase(offset, offset + 2);
				mask = GL_ALPHA;
			}

			GLenum dreg = reg_enum(dst);
			GLenum C = GL_COMBINER0_NV + combiner;

			if(op == "add" || op == "sub")
			{
				src a(instr[2]);
				src b(instr[3]);
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, b.reg, b.map, b.comp);
					if(op == "add")
						glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					else
						glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, GL_EXPAND_NORMAL_NV, GL_RGB);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, b.reg, b.map, GL_ALPHA);
					if(op == "add")
						glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					else
						glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO, GL_EXPAND_NORMAL_NV, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "cnd")
			{
				src a(instr[3]);
				src b(instr[4]);
				if(instr[2] != string("r0.a"))
				{} // bad
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, b.reg, b.map, b.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_TRUE);
				}
				else
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, b.reg, b.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_TRUE);
				}
				else
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "dp3")
			{
				src a(instr[2]);
				src b(instr[3]);
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, b.reg, b.map, b.comp);
					glCombinerOutputNV(C, GL_RGB, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
						GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					// ooh.. what to do here?
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					// todo -- make next ref to dst.a actually ref dst.b since combiners can't write dp3 to the alpha channel
				}
				else
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "lrp")
			{
				src a(instr[2]);
				src b(instr[3]);
				src c(instr[4]);
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, GL_UNSIGNED_IDENTITY_NV, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, b.reg, b.map, b.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, a.reg, GL_UNSIGNED_INVERT_NV, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, c.reg, c.map, c.comp);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, b.reg, b.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, a.reg, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, c.reg, c.map, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "mad")
			{
				src a(instr[2]);
				src b(instr[3]);
				src c(instr[4]);
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, b.reg, b.map, b.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, c.reg, c.map, c.comp);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, b.reg, b.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, c.reg, c.map, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "mov")
			{
				src a(instr[2]);
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerOutputNV(C, GL_RGB, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "mul")
			{
				src a(instr[2]);
				src b(instr[3]);
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, b.reg, b.map, b.comp);
					glCombinerOutputNV(C, GL_RGB, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, b.reg, b.map, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
		}

		int combiner;
	};


}


void ps10::invoke(std::vector<constdef> * c,
                  std::list<std::vector<string> > * a,
                  std::list<std::vector<string> > * b)
{
	glEnable(GL_PER_STAGE_CONSTANTS_NV); // should we require apps to do this?
	if(c)
		std::for_each(c->begin(), c->end(), set_constants());
	if(a)
		std::for_each(a->begin(), a->end(), set_texture_shaders(c));
	if(b)
		std::for_each(b->begin(), b->end(), set_register_combiners());
}

// simple identification - just look for magic substring
//  -- easy to break...
bool is_ps10(const char * s)
{
	if(strstr(s, "ps.1.0"))
		return true;
	if(strstr(s, "Ps.1.0"))
		return true;
	return false;
}

bool ps10::init_extensions()
{
	// register combiners	
	static bool rcinit = false;
	if(rcinit == false)
	{
		if(!SUPPORTS_GL_NV_register_combiners)
		{
			errors.set("unable to initialize GL_NV_register_combiners\n");
			return false;
		}
		else
		{
			rcinit = true;
		}
	}

	// register combiners 2
	static bool rc2init = false;
	if(rc2init == false)
	{
		if( !SUPPORTS_GL_NV_register_combiners2)
		{
			errors.set("unable to initialize GL_NV_register_combiners2\n");
			return false;
		}
		else
		{
			rc2init = true;
		}
	}
	
	static bool tsinit = 0;	
	if (tsinit == false )
	{
		if(!SUPPORTS_GL_NV_texture_shader || !SUPPORTS_GL_ARB_multitexture)
		{
			errors.set("unable to initialize GL_NV_texture_shader\n");
			return false;
		}
		else
		{
			tsinit = true;
		}
	}
		
	return true;
}