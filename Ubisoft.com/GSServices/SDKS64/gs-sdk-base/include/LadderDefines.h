//****************************************************************************
//*   Author:  Guillaume Plante
 /*!  \file   LadderDefines.h
  *   \brief  Definitions for the ladder query service
  *
  *   This file defines global values and structures for the ladder query service.
  */
//****************************************************************************


#ifndef __LADDERDEFINES_H__
#define __LADDERDEFINES_H__

#include "define.h"
#include "GSTypes.h"

#define     LADDER_QUERY_VERSION        1		//!< Version of the ladder query service.

#define     LADDER_NAME_LENGTH          1025	//!< Length of a string representing the ladder name.
#define     LADDER_FIELDNAME_LENGTH     129		//!< Length of a string representing a field name in the ladder.
#define     LADDER_FIELDCAPTION_LENGTH  33		//!< Length of a string representing a ladder field name.
#define     LADDER_FIELDVALUE_LENGTH    129		//!< Length of a string representing a ladder field value.

/*! 
	\enum E_FILTER_OPERATOR
	\brief Filter operator enumeration.

    This enumeration contains the valid operators that are used in the LADDER_FILTER structure.
*/
typedef enum
{
	FILTER_EQUAL,
	FILTER_ABOVE,
	FILTER_BELOW
} E_FILTER_OPERATOR;

/*! 
	\struct LADDER_FILTER
	\brief Ladder filter structure.

    This structure defines a ladder filter that can be used with the fonction LadderQuery_AddFilterConstraint() .
	It is used to filter results based on a numeric value using the operators EQUAL,ABOVE or BELOW defined in the
	E_FILTER_OPERATOR enumeration.
*/
typedef struct
{
	GSchar szFieldName[LADDER_FIELDNAME_LENGTH];	//!< Name of the field that need to be compared against reference value.
	E_FILTER_OPERATOR eOperator;					//!< Logical operator to do comparison.
	GSint iValue;									//!< Reference value used to filter.
} LADDER_FILTER;


/*!
\struct LADDER_ROW
\brief Ladder row representation

This structure defines the representation of a row of ladder values. It is used
by the LobbyRcv_MatchFinalResult callback.
*/
struct LADDER_ROW
{
    GSchar player[NAMELENGTH];
    GSushort valueCount;
    GSint * values;
};

#endif //__LADDERDEFINES_H__
