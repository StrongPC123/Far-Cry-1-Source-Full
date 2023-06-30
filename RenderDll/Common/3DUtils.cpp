
#include "RenderPCH.h"

#pragma warning(push)
#pragma warning(disable:4244) // conversion from 'double' to 'float', possible loss of data
#pragma warning(disable:4305) // truncation from 'double' to 'float'

void utlMtx2Euler(int ord, float m[3][3], float rot[3]);
static void utlMtx2Quat(float m[3][3], float quat[4]);


//  ========== utlDecompMatrix ==========
//
//  SYNOPSIS
//      Decompose a matrix to it's components, translate,
//      rotate ( a quaternion) and scale.
//
static void utlDecompMatrix( const float *mat, DECOMP_MAT *dmat, char *rotOrder)
{
    int         i, j,
    order;
    static float       Sxy, Sxz,
    rot[3], quat[4],
    det,
        m[3][3];

    dmat->translate[0] = mat[3*4+0];
    dmat->translate[1] = mat[3*4+1];
    dmat->translate[2] = mat[3*4+2];

    m[0][0] = mat[0*4+0];
    m[0][1] = mat[0*4+1];
    m[0][2] = mat[0*4+2];

    dmat->scale[0] = sqrt_tpl( m[0][0]*m[0][0] + m[0][1]*m[0][1] + m[0][2]*m[0][2]);

    /* Normalize second row */
    m[0][0] /= dmat->scale[0];
    m[0][1] /= dmat->scale[0];
    m[0][2] /= dmat->scale[0];

    /* Determine xy shear */
    Sxy = mat[0*4+0] * mat[1*4+0] + 
    mat[0*4+1] * mat[1*4+1] +
    mat[0*4+2] * mat[1*4+2];

    m[1][0] = mat[1*4+0] - Sxy * mat[0*4+0];
    m[1][1] = mat[1*4+1] - Sxy * mat[0*4+1];
    m[1][2] = mat[1*4+2] - Sxy * mat[0*4+2];

    dmat->scale[1] = sqrt_tpl( m[1][0]*m[1][0] + m[1][1]*m[1][1] + m[1][2]*m[1][2]);

    /* Normalize second row */
    m[1][0] /= dmat->scale[1];
    m[1][1] /= dmat->scale[1];
    m[1][2] /= dmat->scale[1];

    /* Determine xz shear */
    Sxz = mat[0*4+0] * mat[2*4+0] + 
    mat[0*4+1] * mat[2*4+1] +
    mat[0*4+2] * mat[2*4+2];

    m[2][0] = mat[2*4+0] - Sxz * mat[0*4+0];
    m[2][1] = mat[2*4+1] - Sxz * mat[0*4+1];
    m[2][2] = mat[2*4+2] - Sxz * mat[0*4+2];

    dmat->scale[2] = sqrt_tpl( m[2][0]*m[2][0] + m[2][1]*m[2][1] + m[2][2]*m[2][2]);

    /* Normalize third row */
    m[2][0] /= dmat->scale[2];
    m[2][1] /= dmat->scale[2];
    m[2][2] /= dmat->scale[2];

    det = (m[0][0]*m[1][1]*m[2][2]) + (m[0][1]*m[1][2]*m[2][0]) + (m[0][2]*m[1][0]*m[2][1]) -
    (m[0][2]*m[1][1]*m[2][0]) - (m[0][0]*m[1][2]*m[2][1]) - (m[0][1]*m[1][0]*m[2][2]);

    /* If the determinant of the rotation matrix is negative, */
    /* negate the matrix and scale factors.                   */
    
    if ( det < 0.0) {
    for ( i = 0; i < 3; i++) {
      for ( j = 0; j < 3; j++) 
        m[i][j] *= -1.0;
      dmat->scale[i] *= -1.0;
    }
    }

    // Copy the 3x3 rotation matrix into the decomposition 
    // structure.
    //
    memcpy( dmat->rotMatrix, m, sizeof( float)*9);

    /*rot[1] = asin( -m[0][2]);
    if ( fabsf( cos( rot[1])) > 0.0001) {
        rot[0] = asin( m[1][2]/cos( rot[1]));
        rot[2] = asin( m[0][1]/cos( rot[1]));
    } else {
        rot[0] = acos( m[1][1]);
        rot[2] = 0.0;
    }*/

    switch( rotOrder[2]) {
    case XROT:
      if ( rotOrder[1] == YROT) 
        order = UTL_ROT_XYZ;
      else
        order = UTL_ROT_XZY;
            break;

    case YROT:
      if ( rotOrder[1] == XROT) 
        order = UTL_ROT_YXZ;
      else
        order = UTL_ROT_YZX;
            break;

    case ZROT:
      if ( rotOrder[1] == XROT) 
        order = UTL_ROT_ZXY;
      else
        order = UTL_ROT_ZYX;
            break;

    default:
      order = UTL_ROT_XYZ;
      break;
    }

    utlMtx2Euler( order, m, rot);
    dmat->rotation[0] = rot[0];
    dmat->rotation[1] = rot[1];
    dmat->rotation[2] = rot[2];

    utlMtx2Quat(m,quat);
    dmat->quaternion[0] = quat[0];
    dmat->quaternion[1] = quat[1];
    dmat->quaternion[2] = quat[2];
    dmat->quaternion[3] = quat[3];
}


/*
 *  ========== CapQuat2Euler ==========
 *
 *  SYNOPSIS
 *      Convert a quaternion to Euler angles.
 *
 *  PARAMETERS
 *      int                     The order of rotations
 *      float   mat[3][3]       rotation matrix  
 *      float   rot[3]          xyz-rotation values
 *
 *  DESCRIPTION
 *      This routine converts a mateix to Euler angles.
 *      There are a few caveats:
 *          The rotation order for the returned angles is always zyx.
 *      The derivation of this algorithm is taken from Ken Shoemake's
 *      paper:
 *          SIGGRAPH 1985, Vol. 19, # 3, pp. 253-254
 *
 *  RETURN VALUE
 *      None.
 */

#ifdef WIN32
#define M_PI_2 3.14159/2.0
#endif

void utlMtx2Euler(int ord, float m[3][3], float rot[3])
{
  /*
   *  Ken Shoemake's recommended algorithm is to convert the
   *  quaternion to a matrix and the matrix to Euler angles.
   *  We do this, of course, without generating unused matrix
   *  elements.
   */
    float        zr, sxr, cxr,
    yr, syr, cyr,
    xr, szr, czr;
    static float epsilon = 1.0e-5f;

    switch ( ord) {

        case UTL_ROT_ZYX:
            syr = -m[0][2];
            cyr = sqrt_tpl(1 - syr * syr);

            if (cyr < epsilon) {
                /* Insufficient accuracy, assume that yr = PI/2 && zr = 0 */
                xr = cry_atan2f(-m[2][1], m[1][1]);
                yr = (syr > 0) ? M_PI_2 : -M_PI_2;      /* +/- 90 deg */
                zr = 0.0;
            } else {
                xr = cry_atan2f(m[1][2], m[2][2]);
                yr = cry_atan2f(syr, cyr);
                zr = cry_atan2f(m[0][1], m[0][0]);
            }
            break;

        case UTL_ROT_YZX:
            szr = m[0][1];
            czr = sqrt_tpl(1 - szr * szr);
            if (czr < epsilon) {
                /* Insufficient accuracy, assume that zr = +/- PI/2 && yr = 0 */
                xr = cry_atan2f(m[1][2], m[2][2]);
                yr = 0.0;
                zr = (szr > 0) ? M_PI_2 : -M_PI_2;
            } else {
                xr = cry_atan2f(-m[2][1], m[1][1]);
                yr = cry_atan2f(-m[0][2], m[0][0]);
                zr = cry_atan2f(szr,  czr);
            }
            break;

        case UTL_ROT_ZXY:
            sxr = m[1][2];
            cxr = sqrt_tpl(1 - sxr * sxr);

            if (cxr < epsilon) {
                /* Insufficient accuracy, assume that xr = PI/2 && zr = 0 */
                xr = (sxr > 0) ? M_PI_2 : -M_PI_2;
                yr = cry_atan2f(m[2][0], m[0][0]);
                zr = 0.0;
            } else {
                xr = cry_atan2f( sxr, cxr);
                yr = cry_atan2f(-m[0][2], m[2][2]);
                zr = cry_atan2f(-m[1][0], m[1][1]);
            }
            break;

        case UTL_ROT_XZY:
            szr = -m[1][0];
            czr = sqrt_tpl(1 - szr * szr);
            if (czr < epsilon) {
                /* Insufficient accuracy, assume that zr = PI / 2 && xr = 0 */
                xr = 0.0;
                yr = cry_atan2f(-m[0][2], m[2][2]);
                zr = (szr > 0) ? M_PI_2 : -M_PI_2;
            } else {
                xr = cry_atan2f(m[0][2], m[1][1]);
                yr = cry_atan2f(m[2][0], m[0][0]);
                zr = cry_atan2f(szr, czr);
            }
            break;

        case UTL_ROT_YXZ:
            sxr = -m[2][1];
            cxr = sqrt_tpl(1 - sxr * sxr);

            if (cxr < epsilon) {
                /* Insufficient accuracy, assume that xr = PI/2 && yr = 0 */
                xr = (sxr > 0) ? M_PI_2 : -M_PI_2;
                yr = 0.0;
                zr = cry_atan2f(-m[1][0], m[0][0]);
            } else {
                xr = cry_atan2f(sxr, cxr);
                yr = cry_atan2f(m[2][0], m[2][2]);
                zr = cry_atan2f(m[0][1], m[1][1]);
            }
            break;

        case UTL_ROT_XYZ:
            syr = m[2][0];
            cyr = sqrt_tpl(1 - syr * syr);
            if (cyr < epsilon) {
                /* Insufficient accuracy, assume that yr = PI / 2 && xr = 0 */
                xr = 0.0;
                yr = (syr > 0) ? M_PI_2 : -M_PI_2;
                zr = cry_atan2f(m[0][1], m[1][1]);
            } else {
                xr = cry_atan2f(-m[2][1], m[2][2]);
                yr = cry_atan2f( syr, cyr);
                zr = cry_atan2f(-m[1][1], m[0][0]);
            }
            break;
    }

    rot[0] = xr;
    rot[1] = yr;
    rot[2] = zr;
}

/*
 *  ========= utlMtx2Quat ====================
 * 
 *  SYNOPSIS
 *  Returns the w,x,y,z coordinates of the quaternion
 *  given the rotation matrix.
 */
static void utlMtx2Quat(float m[3][3], float quat[4])
{
    // m stores the 3x3 rotation matrix.
    // Convert it to quaternion.
    float trace = m[0][0] + m[1][1] + m[2][2];
    float s;
    if (trace > 0.0) {
        s = sqrt_tpl(trace + 1.0);
        quat[0] = s*0.5;
        s = 0.5/s;
        quat[1] = (m[1][2] - m[2][1])*s;
        quat[2] = (m[2][0] - m[0][2])*s;
        quat[3] = (m[0][1] - m[1][0])*s;

    }
    else {
        int i = 0; // i represents index of quaternion, so 0=scalar, 1=xaxis, etc.
        int nxt[3] = {1,2,0}; // next index for each component.
        if (m[1][1] > m[0][0]) i = 1;
        if (m[2][2] > m[i][i]) i = 2;
        int j = nxt[i]; int k = nxt[j];
        s = sqrt_tpl( (m[i][i] - (m[j][j] + m[k][k])) + 1.0);
        float q[4];
        q[i+1] = s*0.5;
        s=0.5/s;
        q[0] = (m[j][k] - m[k][j])*s;
        q[j+1] = (m[i][j]+m[j][i])*s;
        q[k+1] = (m[i][k]+m[k][i])*s;
        quat[0] = q[0];
        quat[1] = q[1];
        quat[2] = q[2];
        quat[3] = q[3];
    }
}

/*
 *  ========== DtMatrixGetTranslation ==========
 *
 *  SYNOPSIS
 *  Return the x,y,z translation components of the
 *  given matrix. The priority order is assumed to be ---.
 */

int  DtMatrixGetTranslation( float *matrix, float *xTrans, float *yTrans, float *zTrans)
{
    DECOMP_MAT dmat;
    if (matrix)
    {
        utlDecompMatrix( matrix, &dmat, "xyz" );
        *xTrans = dmat.translate[0];
        *yTrans = dmat.translate[1];
        *zTrans = dmat.translate[2];
    }
    else
    {
        *xTrans = *yTrans = *zTrans = 0.0;
    }
    return(1);

}  /* DtMatrixGetTranslation */

/*
 *  ========== DtMatrixGetQuaternion ==========
 *
 *  SYNOPSIS
 *  Return the quaternion (scalar, xAxis, yAxis, zAxis)
 *  defining the orientation represented in the given matrix.
 */
int  DtMatrixGetQuaternion(float *matrix, float *scalar, float *xAxis, float *yAxis, float *zAxis)
{
    DECOMP_MAT dmat;
    if (matrix)
    {
    utlDecompMatrix( matrix, &dmat, "xyz" );
    *scalar = dmat.quaternion[0];
    *xAxis = dmat.quaternion[1];
    *yAxis = dmat.quaternion[2];
    *zAxis = dmat.quaternion[3];
    } 
    else
    {
    *scalar = 1.0; *xAxis = *yAxis = *zAxis = 0.0;
    }
    return(1);
}  /* DtMatrixGetQuaternion */

/*
 *  ========== DtMatrixGetRotation ==========
 *
 *  SYNOPSIS
 *  Return the x,y,z rotation components of the
 *  given matrix. The priority order is assumed to be ---.
 */

int  DtMatrixGetRotation(float *matrix, float *xRotation, float *yRotation, float *zRotation)
{

    DECOMP_MAT dmat;
    if (matrix)
    {
        utlDecompMatrix( matrix, &dmat, "xyz" );
        *xRotation = dmat.rotation[0];
        *yRotation = dmat.rotation[1];
        *zRotation = dmat.rotation[2];
    }
    else
    {
        *xRotation = *yRotation = *zRotation = 0.0;
    }
    return(1);

}  /* DtMatrixGetRotation */


/*
 *  ========== DtMatrixGetScale ==========
 *
 *  SYNOPSIS
 *  Return the x,y,z scale components of the given
 *  matrix. The priority order is assumed to be ---.
 */

int  DtMatrixGetScale(float *matrix, float *xScale, float *yScale, float *zScale)
{

    DECOMP_MAT dmat;
    
    if (matrix)
    {
        utlDecompMatrix( matrix, &dmat, "xyz" );
        *xScale = dmat.scale[0];
        *yScale = dmat.scale[1];
        *zScale = dmat.scale[2];
    }
    else
    {
        *xScale = *yScale = *zScale = 1.0;
    }
    return(1);

}  /* DtMatrixGetScale */
/*
 *  ========== DtMatrixGetTransforms ==========
 *
 *  SYNOPSIS
 *  Return the x,y,z translation, scale quaternion and
 *  Euler angles in "xyz" order of the given
 *  matrix. 
 */

int DtMatrixGetTransforms(float *matrix, float *translate, 
                          float *scale, float *quaternion, float *rotation)
{
    DECOMP_MAT dmat;

    if (matrix)
    {
        utlDecompMatrix( matrix, &dmat, "xyz" );

  if (translate) {
      translate[0] = dmat.translate[0];
      translate[1] = dmat.translate[1];
      translate[2] = dmat.translate[2];
  }
  if (scale) {
      scale[0] = dmat.scale[0];
      scale[1] = dmat.scale[1];
      scale[2] = dmat.scale[2];
  }
  if (quaternion) {
      quaternion[0] = dmat.quaternion[0];
      quaternion[1] = dmat.quaternion[1];
      quaternion[2] = dmat.quaternion[2];
      quaternion[3] = dmat.quaternion[3];
  }
  if (rotation) {
      rotation[0] = dmat.rotation[0];
      rotation[1] = dmat.rotation[1];
      rotation[2] = dmat.rotation[2];
  }
  return(1);
  }
  return(0);
}

//==============================================================================

float gSinTable[1024] = {
  0.000000,0.001534,0.003068,0.004602,0.006136,0.007670,0.009204,0.010738,
  0.012272,0.013805,0.015339,0.016873,0.018407,0.019940,0.021474,0.023008,
  0.024541,0.026075,0.027608,0.029142,0.030675,0.032208,0.033741,0.035274,
  0.036807,0.038340,0.039873,0.041406,0.042938,0.044471,0.046003,0.047535,
  0.049068,0.050600,0.052132,0.053664,0.055195,0.056727,0.058258,0.059790,
  0.061321,0.062852,0.064383,0.065913,0.067444,0.068974,0.070505,0.072035,
  0.073565,0.075094,0.076624,0.078153,0.079682,0.081211,0.082740,0.084269,
  0.085797,0.087326,0.088854,0.090381,0.091909,0.093436,0.094963,0.096490,
  0.098017,0.099544,0.101070,0.102596,0.104122,0.105647,0.107172,0.108697,
  0.110222,0.111747,0.113271,0.114795,0.116319,0.117842,0.119365,0.120888,
  0.122411,0.123933,0.125455,0.126977,0.128498,0.130019,0.131540,0.133061,
  0.134581,0.136101,0.137620,0.139139,0.140658,0.142177,0.143695,0.145213,
  0.146730,0.148248,0.149765,0.151281,0.152797,0.154313,0.155828,0.157343,
  0.158858,0.160372,0.161886,0.163400,0.164913,0.166426,0.167938,0.169450,
  0.170962,0.172473,0.173984,0.175494,0.177004,0.178514,0.180023,0.181532,
  0.183040,0.184548,0.186055,0.187562,0.189069,0.190575,0.192080,0.193586,
  0.195090,0.196595,0.198098,0.199602,0.201105,0.202607,0.204109,0.205610,
  0.207111,0.208612,0.210112,0.211611,0.213110,0.214609,0.216107,0.217604,
  0.219101,0.220598,0.222094,0.223589,0.225084,0.226578,0.228072,0.229565,
  0.231058,0.232550,0.234042,0.235533,0.237024,0.238514,0.240003,0.241492,
  0.242980,0.244468,0.245955,0.247442,0.248928,0.250413,0.251898,0.253382,
  0.254866,0.256349,0.257831,0.259313,0.260794,0.262275,0.263755,0.265234,
  0.266713,0.268191,0.269668,0.271145,0.272621,0.274097,0.275572,0.277046,
  0.278520,0.279993,0.281465,0.282937,0.284408,0.285878,0.287347,0.288816,
  0.290285,0.291752,0.293219,0.294685,0.296151,0.297616,0.299080,0.300543,
  0.302006,0.303468,0.304929,0.306390,0.307850,0.309309,0.310767,0.312225,
  0.313682,0.315138,0.316593,0.318048,0.319502,0.320955,0.322408,0.323859,
  0.325310,0.326760,0.328210,0.329658,0.331106,0.332553,0.334000,0.335445,
  0.336890,0.338334,0.339777,0.341219,0.342661,0.344101,0.345541,0.346980,
  0.348419,0.349856,0.351293,0.352729,0.354164,0.355598,0.357031,0.358463,
  0.359895,0.361326,0.362756,0.364185,0.365613,0.367040,0.368467,0.369892,
  0.371317,0.372741,0.374164,0.375586,0.377007,0.378428,0.379847,0.381266,
  0.382683,0.384100,0.385516,0.386931,0.388345,0.389758,0.391170,0.392582,
  0.393992,0.395401,0.396810,0.398218,0.399624,0.401030,0.402435,0.403838,
  0.405241,0.406643,0.408044,0.409444,0.410843,0.412241,0.413638,0.415034,
  0.416430,0.417824,0.419217,0.420609,0.422000,0.423390,0.424780,0.426168,
  0.427555,0.428941,0.430326,0.431711,0.433094,0.434476,0.435857,0.437237,
  0.438616,0.439994,0.441371,0.442747,0.444122,0.445496,0.446869,0.448241,
  0.449611,0.450981,0.452350,0.453717,0.455084,0.456449,0.457813,0.459177,
  0.460539,0.461900,0.463260,0.464619,0.465976,0.467333,0.468689,0.470043,
  0.471397,0.472749,0.474100,0.475450,0.476799,0.478147,0.479494,0.480839,
  0.482184,0.483527,0.484869,0.486210,0.487550,0.488889,0.490226,0.491563,
  0.492898,0.494232,0.495565,0.496897,0.498228,0.499557,0.500885,0.502212,
  0.503538,0.504863,0.506187,0.507509,0.508830,0.510150,0.511469,0.512786,
  0.514103,0.515418,0.516732,0.518045,0.519356,0.520666,0.521975,0.523283,
  0.524590,0.525895,0.527199,0.528502,0.529804,0.531104,0.532403,0.533701,
  0.534998,0.536293,0.537587,0.538880,0.540171,0.541462,0.542751,0.544039,
  0.545325,0.546610,0.547894,0.549177,0.550458,0.551738,0.553017,0.554294,
  0.555570,0.556845,0.558119,0.559391,0.560662,0.561931,0.563199,0.564466,
  0.565732,0.566996,0.568259,0.569521,0.570781,0.572040,0.573297,0.574553,
  0.575808,0.577062,0.578314,0.579565,0.580814,0.582062,0.583309,0.584554,
  0.585798,0.587040,0.588282,0.589521,0.590760,0.591997,0.593232,0.594466,
  0.595699,0.596931,0.598161,0.599389,0.600616,0.601842,0.603067,0.604290,
  0.605511,0.606731,0.607950,0.609167,0.610383,0.611597,0.612810,0.614022,
  0.615232,0.616440,0.617647,0.618853,0.620057,0.621260,0.622461,0.623661,
  0.624859,0.626056,0.627252,0.628446,0.629638,0.630829,0.632019,0.633207,
  0.634393,0.635578,0.636762,0.637944,0.639124,0.640303,0.641481,0.642657,
  0.643832,0.645005,0.646176,0.647346,0.648514,0.649681,0.650847,0.652011,
  0.653173,0.654334,0.655493,0.656651,0.657807,0.658961,0.660114,0.661266,
  0.662416,0.663564,0.664711,0.665856,0.667000,0.668142,0.669283,0.670422,
  0.671559,0.672695,0.673829,0.674962,0.676093,0.677222,0.678350,0.679476,
  0.680601,0.681724,0.682846,0.683965,0.685084,0.686200,0.687315,0.688429,
  0.689541,0.690651,0.691759,0.692866,0.693971,0.695075,0.696177,0.697278,
  0.698376,0.699473,0.700569,0.701663,0.702755,0.703845,0.704934,0.706021,
  0.707107,0.708191,0.709273,0.710353,0.711432,0.712509,0.713585,0.714659,
  0.715731,0.716801,0.717870,0.718937,0.720003,0.721066,0.722128,0.723188,
  0.724247,0.725304,0.726359,0.727413,0.728464,0.729514,0.730563,0.731609,
  0.732654,0.733697,0.734739,0.735779,0.736817,0.737853,0.738887,0.739920,
  0.740951,0.741980,0.743008,0.744034,0.745058,0.746080,0.747101,0.748119,
  0.749136,0.750152,0.751165,0.752177,0.753187,0.754195,0.755201,0.756206,
  0.757209,0.758210,0.759209,0.760207,0.761202,0.762196,0.763188,0.764179,
  0.765167,0.766154,0.767139,0.768122,0.769103,0.770083,0.771061,0.772036,
  0.773010,0.773983,0.774953,0.775922,0.776888,0.777853,0.778817,0.779778,
  0.780737,0.781695,0.782651,0.783605,0.784557,0.785507,0.786455,0.787402,
  0.788346,0.789289,0.790230,0.791169,0.792107,0.793042,0.793975,0.794907,
  0.795837,0.796765,0.797691,0.798615,0.799537,0.800458,0.801376,0.802293,
  0.803208,0.804120,0.805031,0.805940,0.806848,0.807753,0.808656,0.809558,
  0.810457,0.811355,0.812251,0.813144,0.814036,0.814926,0.815814,0.816701,
  0.817585,0.818467,0.819348,0.820226,0.821103,0.821977,0.822850,0.823721,
  0.824589,0.825456,0.826321,0.827184,0.828045,0.828904,0.829761,0.830616,
  0.831470,0.832321,0.833170,0.834018,0.834863,0.835706,0.836548,0.837387,
  0.838225,0.839060,0.839894,0.840725,0.841555,0.842383,0.843208,0.844032,
  0.844854,0.845673,0.846491,0.847307,0.848120,0.848932,0.849742,0.850549,
  0.851355,0.852159,0.852961,0.853760,0.854558,0.855354,0.856147,0.856939,
  0.857729,0.858516,0.859302,0.860085,0.860867,0.861646,0.862424,0.863199,
  0.863973,0.864744,0.865514,0.866281,0.867046,0.867809,0.868571,0.869330,
  0.870087,0.870842,0.871595,0.872346,0.873095,0.873842,0.874587,0.875329,
  0.876070,0.876809,0.877545,0.878280,0.879012,0.879743,0.880471,0.881197,
  0.881921,0.882643,0.883363,0.884081,0.884797,0.885511,0.886223,0.886932,
  0.887640,0.888345,0.889048,0.889750,0.890449,0.891146,0.891841,0.892534,
  0.893224,0.893913,0.894599,0.895284,0.895966,0.896646,0.897325,0.898001,
  0.898674,0.899346,0.900016,0.900683,0.901349,0.902012,0.902673,0.903332,
  0.903989,0.904644,0.905297,0.905947,0.906596,0.907242,0.907886,0.908528,
  0.909168,0.909806,0.910441,0.911075,0.911706,0.912335,0.912962,0.913587,
  0.914210,0.914830,0.915449,0.916065,0.916679,0.917291,0.917901,0.918508,
  0.919114,0.919717,0.920318,0.920917,0.921514,0.922109,0.922701,0.923291,
  0.923880,0.924465,0.925049,0.925631,0.926210,0.926787,0.927363,0.927935,
  0.928506,0.929075,0.929641,0.930205,0.930767,0.931327,0.931884,0.932440,
  0.932993,0.933544,0.934093,0.934639,0.935184,0.935726,0.936266,0.936803,
  0.937339,0.937872,0.938404,0.938932,0.939459,0.939984,0.940506,0.941026,
  0.941544,0.942060,0.942573,0.943084,0.943593,0.944100,0.944605,0.945107,
  0.945607,0.946105,0.946601,0.947094,0.947586,0.948075,0.948561,0.949046,
  0.949528,0.950008,0.950486,0.950962,0.951435,0.951906,0.952375,0.952842,
  0.953306,0.953768,0.954228,0.954686,0.955141,0.955594,0.956045,0.956494,
  0.956940,0.957385,0.957826,0.958266,0.958703,0.959139,0.959572,0.960002,
  0.960431,0.960857,0.961280,0.961702,0.962121,0.962538,0.962953,0.963366,
  0.963776,0.964184,0.964590,0.964993,0.965394,0.965793,0.966190,0.966584,
  0.966976,0.967366,0.967754,0.968139,0.968522,0.968903,0.969281,0.969657,
  0.970031,0.970403,0.970772,0.971139,0.971504,0.971866,0.972226,0.972584,
  0.972940,0.973293,0.973644,0.973993,0.974339,0.974684,0.975025,0.975365,
  0.975702,0.976037,0.976370,0.976700,0.977028,0.977354,0.977677,0.977999,
  0.978317,0.978634,0.978948,0.979260,0.979570,0.979877,0.980182,0.980485,
  0.980785,0.981083,0.981379,0.981673,0.981964,0.982253,0.982539,0.982824,
  0.983105,0.983385,0.983662,0.983937,0.984210,0.984480,0.984749,0.985014,
  0.985278,0.985539,0.985798,0.986054,0.986308,0.986560,0.986809,0.987057,
  0.987301,0.987544,0.987784,0.988022,0.988258,0.988491,0.988722,0.988950,
  0.989177,0.989400,0.989622,0.989841,0.990058,0.990273,0.990485,0.990695,
  0.990903,0.991108,0.991311,0.991511,0.991710,0.991906,0.992099,0.992291,
  0.992480,0.992666,0.992850,0.993032,0.993212,0.993389,0.993564,0.993737,
  0.993907,0.994075,0.994240,0.994404,0.994565,0.994723,0.994879,0.995033,
  0.995185,0.995334,0.995481,0.995625,0.995767,0.995907,0.996045,0.996180,
  0.996313,0.996443,0.996571,0.996697,0.996820,0.996941,0.997060,0.997176,
  0.997290,0.997402,0.997511,0.997618,0.997723,0.997825,0.997925,0.998023,
  0.998118,0.998211,0.998302,0.998390,0.998476,0.998559,0.998640,0.998719,
  0.998795,0.998870,0.998941,0.999011,0.999078,0.999142,0.999205,0.999265,
  0.999322,0.999378,0.999431,0.999481,0.999529,0.999575,0.999619,0.999660,
  0.999699,0.999735,0.999769,0.999801,0.999831,0.999858,0.999882,0.999905,
  0.999925,0.999942,0.999958,0.999971,0.999981,0.999989,0.999995,0.999999
};

typedef union FastSqrtUnion
{
  float f;
  unsigned int i;
} FastSqrtUnion;

unsigned int gFastSqrtTable[0x10000];  // declare table of square roots 

void  build_sqrt_table()
{
  unsigned int i;
  FastSqrtUnion s;

  for (i = 0; i <= 0x7FFF; i++)
  {

    // Build a float with the bit pattern i as mantissa
    //  and an exponent of 0, stored as 127

    s.i = (i << 8) | (0x7F << 23);
    s.f = (float)sqrt_tpl(s.f);

    // Take the square root then strip the first 7 bits of
    //  the mantissa into the table

    gFastSqrtTable[i + 0x8000] = (s.i & 0x7FFFFF);

    // Repeat the process, this time with an exponent of 1, 
    //  stored as 128

    s.i = (i << 8) | (0x80 << 23);
    s.f = (float)sqrt_tpl(s.f);

    gFastSqrtTable[i] = (s.i & 0x7FFFFF);
  }
}

#include <float.h>


/*
* Initialize tables, etc for fast math functions.
*/
void init_math(void)
{
  static bool initialized = false;

  if (!initialized)
  {
    build_sqrt_table();
    initialized = true;
  }
}

#pragma warning(pop)
