/*************************************************************************

   Program:    
   File:       InPDBZone.c
   
   Version:    V1.5
   Date:       25.02.14
   Function:   
   
   Copyright:  (c) SciTech Software 1993-2014
   Author:     Dr. Andrew C. R. Martin
   Phone:      +44 (0) 1372 275775
   EMail:      andrew@bioinf.org.uk
               
**************************************************************************

   This program is not in the public domain, but it may be copied
   according to the conditions laid out in the accompanying file
   COPYING.DOC

   The code may not be sold commercially or included as part of a 
   commercial product except as described in the file COPYING.DOC.

**************************************************************************

   Description:
   ============

**************************************************************************

   Usage:
   ======

**************************************************************************

   Revision History:
   =================
   V1.0  30.09.92 Original
   V1.1  16.06.93 Tidied for book. Mode now a char.
   V1.2  18.06.96 Added InPDBZone() from QTree program
   V1.3  19.09.96 Added InPDBZoneSpec()
   V1.4  24.02.14 Added BiopInPDBZone() By: CTP
   V1.5  25.02.14 Added error message for InPDBZone(). By: CTP

*************************************************************************/
/* Includes
*/
#include "SysDefs.h"
#include "pdb.h"

/************************************************************************/
/* Defines and macros
*/

/************************************************************************/
/* Globals
*/

/************************************************************************/
/* Prototypes
*/


/************************************************************************/
/*>BOOL InPDBZone(PDB *p, char chain, int resnum1, char insert1, 
                  int resnum2, char insert2)
   ----------------------------------------------------------
   Input:   PDB    *p         Pointer to a PDB record
            char   chain      Chain name
            int    resnum1    First residue
            char   insert1    First insert code
            int    resnum2    Second residue
            char   insert2    Second insert code
   Returns: BOOL              Is p in the range specified?

   Checks that atom stored in PDB pointer p is within the specified 
   residue range.

   N.B. This assumes ASCII coding.

   29.03.95 Original    By: ACRM
   08.02.96 Insert residues inside a zone were not handled correctly!
   18.06.96 Added to bioplib from QTree (was called InZone())
   24.02.14 Converted into wrapper for BiopInPDBZone() By: CTP
   25.02.14 Added error message. By: CTP
*/
BOOL InPDBZone(PDB *p, char chain, int resnum1, char insert1, 
               int resnum2, char insert2)
{
   char chain_a[2]   = " ",
        insert1_a[2] = " ",
        insert2_a[2] = " ";
        
#ifdef BIOPLIB_CHECK
   fprintf(stderr, 
           "This code uses InPDBZone() which is now deprecated!\n");
#endif

   chain_a[0]   = chain;
   insert1_a[0] = insert1;
   insert2_a[0] = insert2;
        
   return(BiopInPDBZone(p,chain_a,resnum1,insert1_a,resnum2, insert2_a));
}

/************************************************************************/
/*>BOOL BiopInPDBZone(PDB *p, char *chain, int resnum1, char *insert1, 
                    int resnum2, char *insert2)
   ---------------------------------------------------------------
   Input:   PDB    *p         Pointer to a PDB record
            char   *chain     Chain name
            int    resnum1    First residue
            char   *insert1   First insert code
            int    resnum2    Second residue
            char   *insert2   Second insert code
   Returns: BOOL              Is p in the range specified?

   Checks that atom stored in PDB pointer p is within the specified 
   residue range.

   N.B. This assumes ASCII coding.

   24.02.14 Based on InPDBZone() but takes chain and inserts as stings 
            instead of chars. By: CTP
   
*/
BOOL BiopInPDBZone(PDB *p, char *chain, int resnum1, char *insert1, 
                   int resnum2, char *insert2)
{
   if(CHAINMATCH(p->chain,chain))
   {
      
      /* If residue number is *within* the range, return TRUE           */
      if((p->resnum > resnum1) && (p->resnum < resnum2))
         return(TRUE);
      
      /* If the range has a single residue number, check both inserts   */
      if((p->resnum == resnum1) && (p->resnum == resnum2))
      {
         if(((int)p->insert[0] >= (int)insert1[0]) &&
            ((int)p->insert[0] <= (int)insert2[0]))
            return(TRUE);
      }
      
      /* If residue number matches ends of range check insert           */
      if(((p->resnum == resnum1) &&
          ((int)p->insert[0] >= (int)insert1[0])) ||
         ((p->resnum == resnum2) &&
          ((int)p->insert[0] <= (int)insert2[0])))
         return(TRUE);
   }
   
   return(FALSE);
}


