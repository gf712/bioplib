/************************************************************************/
/**

   \file       ErrStack.h
   
   \version    V1.0R
   \date       31.08.94
   \brief      Build and print an error stack for program failure.
   
   \copyright  (c) UCL / Dr. Andrew C. R. Martin 1994
   \author     Dr. Andrew C. R. Martin
   \par
               Institute of Structural & Molecular Biology,
               University College London,
               Gower Street,
               London.
               WC1E 6BT.
   \par
               andrew@bioinf.org.uk
               andrew.martin@ucl.ac.uk
               
**************************************************************************

   This code is NOT IN THE PUBLIC DOMAIN, but it may be copied
   according to the conditions laid out in the accompanying file
   COPYING.DOC.

   The code may be modified as required, but any modifications must be
   documented so that the person responsible can be identified.

   The code may not be sold commercially or included as part of a 
   commercial product except as described in the file COPYING.DOC.

**************************************************************************

   Description:
   ============


   This set of routines allows a stack of errors to be created. 
   When a program has a fatal error, the StoreError() routine is called
   to place the error on the stack. As the program un-winds, each
   routine which fails stores it's error. Finally, before the program
   actually exits, it calls ShowErrors() to display the error stack.

**************************************************************************

   Usage:
   ======

**************************************************************************

   Revision History:
   =================
-  V1.0  31.08.94 Original    By: ACRM

*************************************************************************/
#ifndef _ERRSTACK_H
#define _ERRSTACK_H
/************************************************************************/
/* Includes
*/
#include "SysDefs.h"

/************************************************************************/
/* Prototypes
*/
void StoreError(char *routine, char *error);
void ShowErrors(void *PrintRoutine(char *), BOOL Trace);

/************************************************************************/
#endif
