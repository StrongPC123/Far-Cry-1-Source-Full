#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "state_to_nvparse_text.h"

#include <string>

//using namespace std;

// register combiners
namespace
{
	struct portion_state
	{
		GLint portion;
		GLint input[4];
		GLint mapping[4];
		GLint compusg[4];
		GLint output[3];
		GLint opselect[3];
		GLint bias;
		GLint scale;
	};


	bool nvrc1_supported;
	bool nvrc2_enabled;
	GLfloat c0[4], c1[4];

	bool same_color( const GLfloat * rgba1, const GLfloat * rgba2)
	{
		if( rgba1[0] == rgba2[0] && 
			rgba1[1] == rgba2[1] && 
			rgba1[2] == rgba2[2] && 
			rgba1[3] == rgba2[3])
			return true;
		return false;
	}

	portion_state get_portion(GLenum combiner, GLenum portion)
	{
		GLenum v[4] = { GL_VARIABLE_A_NV, GL_VARIABLE_B_NV, GL_VARIABLE_C_NV, GL_VARIABLE_D_NV };
		portion_state p;
		p.portion = portion;

		for(int i=0; i < 4; i++)
		{
			glGetCombinerInputParameterivNV(combiner, portion, v[i], GL_COMBINER_INPUT_NV, & p.input[i]);
			glGetCombinerInputParameterivNV(combiner, portion, v[i], GL_COMBINER_MAPPING_NV, & p.mapping[i]);
			glGetCombinerInputParameterivNV(combiner, portion, v[i], GL_COMBINER_COMPONENT_USAGE_NV, & p.compusg[i]);
		}

		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_AB_OUTPUT_NV, & p.output[0]);
		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_CD_OUTPUT_NV, & p.output[1]);
		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_SUM_OUTPUT_NV, & p.output[2]);

		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_AB_DOT_PRODUCT_NV, & p.opselect[0]);
		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_CD_DOT_PRODUCT_NV, & p.opselect[1]);
		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_MUX_SUM_NV, & p.opselect[2]);

		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_BIAS_NV, & p.bias);
		glGetCombinerOutputParameterivNV(combiner, portion, GL_COMBINER_SCALE_NV, & p.scale);

		return p;
	}

	string register_name(GLenum reg)
	{
		switch (reg)
		{
		case GL_CONSTANT_COLOR0_NV:
			return "const0";
		case GL_CONSTANT_COLOR1_NV:
			return "const1";
		case GL_FOG_COLOR:
			return "fog";
		case GL_PRIMARY_COLOR_NV:
			return "col0";
		case GL_SECONDARY_COLOR_NV:
			return "col1";
		case GL_TEXTURE0_ARB:
			return "tex0";
		case GL_TEXTURE1_ARB:
			return "tex1";
		case GL_TEXTURE2_ARB:
			return "tex2";
		case GL_TEXTURE3_ARB:
			return "tex3";
		case GL_SPARE0_NV:
			return "spare0";
		case GL_SPARE1_NV:
			return "spare1";
		case GL_ZERO:
			return "zero";
		case GL_DISCARD_NV:
			return "discard";
		case GL_E_TIMES_F_NV:
			return "final_product";
		case GL_SPARE0_PLUS_SECONDARY_COLOR_NV:
			return "color_sum";
		default:
			return "unknown_register";
		}
	}

	void get_mapping(GLenum mapping, string & mapping_prefix, string & mapping_suffix)
	{
		switch (mapping)
		{
		case GL_SIGNED_IDENTITY_NV:
			mapping_prefix = "";
			mapping_suffix = "";
			break;
		case GL_SIGNED_NEGATE_NV:
			mapping_prefix = "-";
			mapping_suffix = "";
			break;
		case GL_EXPAND_NORMAL_NV:
			mapping_prefix = "expand(";
			mapping_suffix = ")";
			break;
		case GL_EXPAND_NEGATE_NV:
			mapping_prefix = "-expand(";
			mapping_suffix = ")";
			break;
		case GL_HALF_BIAS_NORMAL_NV:
			mapping_prefix = "half_bias(";
			mapping_suffix = ")";
			break;
		case GL_HALF_BIAS_NEGATE_NV:
			mapping_prefix = "-half_bias(";
			mapping_suffix = ")";
			break;
		case GL_UNSIGNED_IDENTITY_NV:
			mapping_prefix = "unsigned(";
			mapping_suffix = ")";
			break;
		case GL_UNSIGNED_INVERT_NV:
			mapping_prefix = "unsigned_invert(";
			mapping_suffix = ")";
			break;
		default:
			mapping_prefix = "unknown_mapping(";
			mapping_suffix = ")";
			break;
		}
	}

	string operand_string(GLenum reg, GLenum mapping, GLenum compusg, GLenum portion)
	{
		string rname = register_name(reg);
		string cu;
		if(compusg != portion)
		{
			if(compusg == GL_RGB)
				cu = ".rgb";
			if(compusg == GL_ALPHA)
				cu = ".a";
			if(compusg == GL_BLUE)
				cu = ".b";
		}
		string mapping_prefix, mapping_suffix;
		get_mapping(mapping, mapping_prefix, mapping_suffix);
		string op = mapping_prefix + rname + cu + mapping_suffix;
		return op;
	}

	string general_combiner_portion(GLenum combiner, GLenum portion)
	{
		portion_state p = get_portion(combiner, portion);
		string s;
	
		// if unused portion return empty string
		if( p.output[0] == GL_DISCARD_NV &&
			p.output[1] == GL_DISCARD_NV &&
			p.output[2] == GL_DISCARD_NV)
			return s;

		if(portion == GL_RGB) s += "  rgb {\n";
		else                  s += "  alpha {\n";

		// do the AB and CD expressions
		for(int i=0; i < 2; i++)
		{
			if(p.output[i] != GL_DISCARD_NV || p.output[2] != GL_DISCARD_NV)
			{
				s += "    ";
				s += register_name(p.output[i]);
				s += " = ";
				// handle the arg * 1 simplification:
				if( (p.input[2*i  ] == GL_ZERO && p.mapping[2*i ] == GL_UNSIGNED_INVERT_NV) &&
					(p.input[2*i+1] != GL_ZERO) )
				{
					s += operand_string(p.input[2*i+1], p.mapping[2*i+1], p.compusg[2*i+1], portion);
				}
				else if( (p.input[2*i+1] == GL_ZERO && p.mapping[2*i+1] == GL_UNSIGNED_INVERT_NV) &&
					(p.input[2*i  ] != GL_ZERO) )
				{
					s += operand_string(p.input[2*i], p.mapping[2*i], p.compusg[2*i], portion);
				}
				else
				{
					s += operand_string(p.input[2*i], p.mapping[2*i], p.compusg[2*i], portion);
					if(p.opselect[i] == GL_TRUE)
						s += " . ";
					else
						s += " * ";
					s += operand_string(p.input[2*i+1], p.mapping[2*i+1], p.compusg[2*i+1], portion);
				}
				s += ";\n";
			}
		}
		if(p.output[2] != GL_DISCARD_NV)
		{
			s += "    ";
			s += register_name(p.output[2]);
			s += " = ";
			if(p.opselect[2] == GL_TRUE)
				s += "mux();\n";
			else
				s += "sum();\n";
		}

		if(p.bias == GL_NONE && p.scale == GL_SCALE_BY_ONE_HALF_NV)
			s += "    scale_by_one_half();\n";
		else if(p.bias == GL_NONE && p.scale == GL_SCALE_BY_TWO_NV)
			s += "    scale_by_one_two();\n";
		else if(p.bias == GL_NONE && p.scale == GL_SCALE_BY_FOUR_NV)
			s += "    scale_by_one_four();\n";
		else if(p.bias == GL_BIAS_BY_NEGATIVE_ONE_HALF_NV && p.scale == GL_NONE)
			s += "    bias_by_negative_one_half();\n";
		else if(p.bias == GL_BIAS_BY_NEGATIVE_ONE_HALF_NV && p.scale == GL_SCALE_BY_TWO_NV)
			s += "    bias_by_negative_one_half_scale_by_two();\n";


		s += "  }\n";
		return s;
	}

	string general_combiner(GLenum combiner)
	{
		string s;
		s += general_combiner_portion(combiner, GL_RGB);
		s += general_combiner_portion(combiner, GL_ALPHA);
		// if unused combiner return empty string
		if(s.size() == 0)
			return "";
		string cc;
		if(nvrc2_enabled)
		{
			GLfloat sc0[4], sc1[4];
			glGetCombinerStageParameterfvNV(combiner, GL_CONSTANT_COLOR0_NV, sc0);
			glGetCombinerStageParameterfvNV(combiner, GL_CONSTANT_COLOR1_NV, sc1);
			if(! same_color(c0, sc0))
			{
				char buf[80];
				sprintf(buf, "  const0 = (%f, %f, %f, %f);\n", sc0[0], sc0[1], sc0[2], sc0[3]);
				cc += buf;
			}
			if(! same_color(c1, sc1))
			{
				char buf[80];
				sprintf(buf, "  const1 = (%f, %f, %f, %f);\n", sc1[0], sc1[1], sc1[2], sc1[3]);
				cc += buf;
			}
		}

		return "{\n" + cc + s + "}\n";
	}

	struct final_combiner_state
	{
		GLint input[7];
		GLint mapping[7];
		GLint compusg[7];
		bool clamp_color_sum;
	};

	final_combiner_state get_final_combiner_state()
	{
		GLenum v[7] = { GL_VARIABLE_A_NV, GL_VARIABLE_B_NV, GL_VARIABLE_C_NV,
						GL_VARIABLE_D_NV, GL_VARIABLE_E_NV, GL_VARIABLE_F_NV,
						GL_VARIABLE_G_NV };
		final_combiner_state fc;

		for(int i=0; i < 7; i++)
		{
			glGetFinalCombinerInputParameterivNV(v[i], GL_COMBINER_INPUT_NV, & fc.input[i]);
			glGetFinalCombinerInputParameterivNV(v[i], GL_COMBINER_MAPPING_NV, & fc.mapping[i]);
			glGetFinalCombinerInputParameterivNV(v[i], GL_COMBINER_COMPONENT_USAGE_NV, & fc.compusg[i]);
		}
		GLboolean clamp_color_sum = GL_FALSE;
		glGetBooleanv(GL_COLOR_SUM_CLAMP_NV, & clamp_color_sum);
		fc.clamp_color_sum = (clamp_color_sum == GL_TRUE);
		return fc;
	}

	bool is_zero(const final_combiner_state & fc, int i)
	{
		if(fc.input[i] == GL_ZERO && fc.mapping[i] == GL_UNSIGNED_IDENTITY_NV)
			return true;
		return false;
	}

	
	string final_operand_string(GLenum reg, GLenum mapping, GLenum compusg, GLenum portion)
	{
		string rname = register_name(reg);
		string cu;
		if(compusg != portion)
		{
			if(compusg == GL_RGB)
				cu = ".rgb";
			if(compusg == GL_ALPHA)
				cu = ".a";
			if(compusg == GL_BLUE)
				cu = ".b";
		}
		string mapping_prefix, mapping_suffix;
		if(mapping == GL_UNSIGNED_INVERT_NV)
		{
			mapping_prefix = "unsigned_invert(";
			mapping_suffix = ")";
		}
		string op = mapping_prefix + rname + cu + mapping_suffix;
		return op;
	}

	string final_combiner()
	{
		final_combiner_state fc = get_final_combiner_state();
		string final_product;

		if( fc.input[0] == GL_E_TIMES_F_NV || 
			fc.input[1] == GL_E_TIMES_F_NV || 
			fc.input[2] == GL_E_TIMES_F_NV || 
			fc.input[3] == GL_E_TIMES_F_NV   )
		{
			final_product += "final_product = ";
			final_product += final_operand_string(fc.input[4], fc.mapping[4], fc.compusg[4], GL_RGB);
			final_product += " * ";
			final_product += final_operand_string(fc.input[5], fc.mapping[5], fc.compusg[5], GL_RGB);
			final_product += ";\n";
		}
		string clamp_color_sum;
		if(fc.clamp_color_sum &&
		   (fc.input[1] == GL_SPARE0_PLUS_SECONDARY_COLOR_NV || 
			fc.input[2] == GL_SPARE0_PLUS_SECONDARY_COLOR_NV || 
			fc.input[3] == GL_SPARE0_PLUS_SECONDARY_COLOR_NV   )
		  )
			clamp_color_sum = "clamp_color_sum();\n";

		string out_rgb;
		if(is_zero(fc, 0) && ! is_zero(fc, 3)) // output == d;
		{
			out_rgb = "out.rgb = ";
			out_rgb += final_operand_string(fc.input[3], fc.mapping[3], fc.compusg[3], GL_RGB);
			out_rgb += ";\n";
		}
		else if(! is_zero(fc, 0) && ! is_zero(fc, 1) && ! is_zero(fc, 2)) // lerp
		{
			out_rgb = "out.rgb = lerp(";
			out_rgb += final_operand_string(fc.input[0], fc.mapping[0], fc.compusg[0], GL_RGB);
			out_rgb += ", ";
			out_rgb += final_operand_string(fc.input[1], fc.mapping[1], fc.compusg[1], GL_RGB);
			out_rgb += ", ";
			out_rgb += final_operand_string(fc.input[2], fc.mapping[2], fc.compusg[2], GL_RGB);
			out_rgb += ")";
			if(! is_zero(fc,3))
			{
				out_rgb += " + ";
				out_rgb += final_operand_string(fc.input[3], fc.mapping[3], fc.compusg[3], GL_RGB);
			}
			out_rgb += ";\n";
		}
		else if(! is_zero(fc, 0) && ! is_zero(fc, 1)) // mul or mad
		{
			out_rgb = "out.rgb = ";
			out_rgb += final_operand_string(fc.input[0], fc.mapping[0], fc.compusg[0], GL_RGB);
			out_rgb += " * ";
			out_rgb += final_operand_string(fc.input[1], fc.mapping[1], fc.compusg[1], GL_RGB);
			if(! is_zero(fc,3))
			{
				out_rgb += " + ";
				out_rgb += final_operand_string(fc.input[3], fc.mapping[3], fc.compusg[3], GL_RGB);
			}
			out_rgb += ";\n";
		}
		else if(! is_zero(fc, 0) && is_zero(fc, 1) && ! is_zero(fc,2)) // odd case of mul or mad
		{
			// invert the mapping for A since the combiner setup is (1-A)C
			// but we can only express AB with nvparse
			if(fc.mapping[0] == GL_UNSIGNED_IDENTITY_NV)
				fc.mapping[0] = GL_UNSIGNED_INVERT_NV;
			else
				fc.mapping[0] = GL_UNSIGNED_IDENTITY_NV;

			out_rgb = "out.rgb = ";
			out_rgb += final_operand_string(fc.input[0], fc.mapping[0], fc.compusg[0], GL_RGB);
			out_rgb += " * ";
			out_rgb += final_operand_string(fc.input[2], fc.mapping[2], fc.compusg[2], GL_RGB);
			if(! is_zero(fc,3))
			{
				out_rgb += " + ";
				out_rgb += final_operand_string(fc.input[3], fc.mapping[3], fc.compusg[3], GL_RGB);
			}
			out_rgb += ";\n";
		}

		string out_a;
		if( ! is_zero(fc, 6) ) 
		{
			out_a = "out.a = ";
			out_a += final_operand_string(fc.input[6], fc.mapping[6], fc.compusg[6], GL_ALPHA);
			out_a += ";\n";
		}

		return final_product + clamp_color_sum + out_rgb + out_a;
	}

}


extern "C" char * state_to_rc10 ()
{
	nvrc1_supported = false; // required
	nvrc2_enabled = false;   // optional

	if(SUPPORTS_GL_NV_register_combiners)
		nvrc1_supported = true;
	if(SUPPORTS_GL_NV_register_combiners2)
	{
		if(glIsEnabled(GL_PER_STAGE_CONSTANTS_NV))
			nvrc2_enabled = true;
	}

	if(nvrc1_supported == false)
		return 0;
	
	GLenum combiner[8] = {GL_COMBINER0_NV, GL_COMBINER1_NV, GL_COMBINER2_NV, GL_COMBINER3_NV,
						  GL_COMBINER4_NV, GL_COMBINER5_NV, GL_COMBINER6_NV, GL_COMBINER7_NV };
	string state;

	state += "!!RC1.0\n";

	glGetFloatv(GL_CONSTANT_COLOR0_NV, c0);
	glGetFloatv(GL_CONSTANT_COLOR1_NV, c1);
	
	char buf[80];
	sprintf(buf, "const0 = (%f, %f, %f, %f);\n", c0[0], c0[1], c0[2], c0[3]);
	state += buf;
	sprintf(buf, "const1 = (%f, %f, %f, %f);\n", c1[0], c1[1], c1[2], c1[3]);
	state += buf;

	GLint num_combiners;
	glGetIntegerv(GL_NUM_GENERAL_COMBINERS_NV, &num_combiners);
	for(int i = 0; i < num_combiners; i++)
		state += general_combiner(combiner[i]);

	state += final_combiner();

	return strdup(state.c_str());
}


