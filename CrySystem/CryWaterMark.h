// Header for adding a watermark to an exe, which can then be set
// by the external CryWaterMark program. To use, simply write:
//
// WATERMARKDATA(__blah);
// 
// anywhere in the global scope in the program

#define NUMMARKWORDS 10
#define WATERMARKDATA(name) int name[] = { 0xDEBEFECA, 0xFABECEDA, 0xADABAFBE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// (the name is such that you can have multiple watermarks in one exe, don't use
// names like "watermark" just incase you accidentally give out an exe with
// debug information).
 