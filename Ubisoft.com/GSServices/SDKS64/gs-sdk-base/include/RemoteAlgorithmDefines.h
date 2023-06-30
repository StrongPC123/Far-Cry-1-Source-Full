#ifndef __REMOTEALGORITHMDEFINES_H__
#define __REMOTEALGORITHMDEFINES_H__

/*!
\struct
\brief INPUT/OUTPUT Supported Data Types

This structure defines the data types that are supported as inputs and outputs
by remote algorithms.
*/
enum RAE_DATATYPE {
    RAE_INTEGER    //!< Any integer value (char, short, int, etc)
};

/*!
\struct RAE_VALUE
\brief INPUT/OUTPUT Element

This structure defines the value elements that are to be used as inputs and
outputs to the executed algorithm. This allows to create arrays of multiple
different type of data (once the structure handles more than just integers).
*/
struct RAE_VALUE
{
    RAE_DATATYPE dataType;      //!< The type of data this node contains
    union
    {
        GSint intValue;         //!< Field for an integer value
    };
};
        
#endif // __REMOTEALGORITHMDEFINES_H__
