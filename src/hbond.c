/* #define ALLOW_AXN */
/************************************************************************/
/**

   \file       hbond.c
   
   \version    V1.7
   \date       07.07.14
   \brief      Report whether two residues are H-bonded using
               Baker & Hubbard criteria
   
   \copyright  (c) UCL / Dr. Andrew C. R. Martin 1996-2014
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


**************************************************************************

   Usage:
   ======
   NOTE, explicit hydrogens must be added to the PDB linked list before
   calling this routine.

   The only external entry points are IsHBonded() and ValidHBond()

**************************************************************************

   Revision History:
   =================
-  V1.0  26.01.96 Original    By: ACRM
-  V1.1  09.02.96 Added #ifdef'd code to allow AE1/AE2 AD1/AD2
-  V1.2  19.12.02 Fixed bug in walking over multiple H atoms
-  V1.3  18.08.05 Fixed bug relating to sidechains like GLN/ASN
-  V1.4  03.01.06 Proline backbone nitrogen cannot act as donor!
                  Also incorporated fix in ValidHBond() from 02.06.99
                  Inpharmatica version to handle NULL antecedent atoms
-  V1.5  17.01.06 Added IsMCDonorHBonded() and IsMCAcceptorHBonded()
-  V1.6  20.03.14 Updated message in Demo code. By: CTP
-  V1.7  07.07.14 Use bl prefix for functions By: CTP

*************************************************************************/
/* Includes
*/
#include <math.h>
#include "MathType.h"
#include "SysDefs.h"
#include "pdb.h"
#include "macros.h"
#include "general.h"
#include "angle.h"
#include "hbond.h"

/************************************************************************/
/* Defines and macros
*/
#define DADIST 3.5
#define DADISTSQ (DADIST*DADIST)
#define HADIST 2.5
#define HADISTSQ (HADIST*HADIST)


/************************************************************************/
/* Globals
*/


/************************************************************************/
/* Prototypes
*/
static BOOL FindBackboneAcceptor(PDB *res, PDB **AtomA, PDB **AtomP);
static BOOL FindBackboneDonor(PDB *res, PDB **AtomH, PDB **AtomD);
static BOOL FindSidechainAcceptor(PDB *res, PDB **AtomA, PDB **AtomP);
static BOOL FindSidechainDonor(PDB *res, PDB **AtomH, PDB **AtomD);


/************************************************************************/
/*>int blIsHBonded(PDB *res1, PDB *res2, int type)
   -----------------------------------------------
*//**

   \param[in]     *res1     First residue
   \param[in]     *res2     Second residue
   \param[in]     type      HBond type to search for
   \return                    HBond type found or 0 if none

   Determines whether 2 residues are H-bonded using the crude criteria
   of Baker & Hubbard, 1984 (Prog. Biophys. & Mol. Biol, 44, 97-179)

   N.B. Explicit hydrogens must be added to the PDB linked list before
   calling this routine!

   Searches may be made for any H-bond, or just for those involving
   backbone or sidechain. These requirements may be combined in any
   way for the two residues by ORing the appropriate flags in the type
   variable.


      The flags are:
      HBOND_BACK1 (res1 b/b)
      HBOND_BACK2 (res2 b/b)
      HBOND_SIDE1 (res1 s/c)
      HBOND_SIDE2 (res2 s/c)


      The most common flag combinations are already provided:
      HBOND_BB         (b/b -- b/b)
      HBOND_BS         (b/b -- s/c)
      HBOND_SS         (s/c -- s/c)
      HBOND_SB         (s/c -- b/b)
      HBOND_SIDECHAIN  (s/c -- any)
      HBOND_BACKBONE   (b/b -- any)
      HBOND_ANY        (any -- any)

-  25.01.96 Original    By: ACRM
-  07.07.14 Use bl prefix for functions By: CTP
*/
int blIsHBonded(PDB *res1, PDB *res2, int type)
{
   PDB *AtomH,              /* The hydrogen                             */
       *AtomD,              /* The hydrogen donor                       */
       *AtomA,              /* The acceptor                             */
       *AtomP;              /* The acceptor's antecedent                */

   /* Find H-bonds involving the backbone of res1                       */
   if(ISSET(type, HBOND_BACK1))
   {
      if(FindBackboneDonor(res1, &AtomH, &AtomD))
      {
         if(ISSET(type, HBOND_BACK2))
         {
            if(FindBackboneAcceptor(res2, &AtomA, &AtomP))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_BACK1|HBOND_BACK2);
            }
         }
         if(ISSET(type, HBOND_SIDE2))
         {
            /* Clear internal flags                                     */
            FindSidechainAcceptor(NULL, NULL, NULL);
            while(FindSidechainAcceptor(res2, &AtomA, &AtomP))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_BACK1|HBOND_SIDE2);
            }
         }
      }
      if(FindBackboneAcceptor(res1, &AtomA, &AtomP))
      {
         if(ISSET(type, HBOND_BACK2))
         {
            if(FindBackboneDonor(res2, &AtomH, &AtomD))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_BACK1|HBOND_BACK2);
            }
         }
         if(ISSET(type, HBOND_SIDE2))
         {
            /* Clear internal flags                                     */
            FindSidechainDonor(NULL, NULL, NULL);
            while(FindSidechainDonor(res2, &AtomH, &AtomD))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_BACK1|HBOND_SIDE2);
            }
         }
      }
   }
   
   /* Find H-bonds involving sidechain of res1                          */
   if(ISSET(type, HBOND_SIDE1))
   {
      /* Clear internal flags                                           */
      FindSidechainDonor(NULL, NULL, NULL);
      while(FindSidechainDonor(res1, &AtomH, &AtomD))
      {
         if(ISSET(type, HBOND_BACK2))
         {
            if(FindBackboneAcceptor(res2, &AtomA, &AtomP))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_SIDE1|HBOND_BACK2);
            }
         }
         if(ISSET(type, HBOND_SIDE2))
         {
            /* Clear internal flags                                     */
            FindSidechainAcceptor(NULL, NULL, NULL);
            while(FindSidechainAcceptor(res2, &AtomA, &AtomP))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_SIDE1|HBOND_SIDE2);
            }
         }
      }
      /* Clear internal flags                                           */
      FindSidechainAcceptor(NULL, NULL, NULL);
      while(FindSidechainAcceptor(res1, &AtomA, &AtomP))
      {
         if(ISSET(type, HBOND_BACK2))
         {
            if(FindBackboneDonor(res2, &AtomH, &AtomD))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_SIDE1|HBOND_BACK2);
            }
         }
         if(ISSET(type, HBOND_SIDE2))
         {
            /* Clear internal flags                                     */
            FindSidechainDonor(NULL, NULL, NULL);
            while(FindSidechainDonor(res2, &AtomH, &AtomD))
            {
               if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
                  return(HBOND_SIDE1|HBOND_SIDE2);
            }
         }
      }
   }

   return(0);
}


/************************************************************************/
/*>BOOL blValidHBond(PDB *AtomH, PDB *AtomD, PDB *AtomA, PDB *AtomP)
   -----------------------------------------------------------------
*//**

   \param[in]     *AtomH      The hydrogen
   \param[in]     *AtomD      The donor (to which the H is attached)
   \param[in]     *AtomA      The acceptor
   \param[in]     *AtomP      The antecedent to the acceptor
   \return                    Valid?

   Determines whether a set of atoms form a valid H-bond using the
   Baker and Hubbard criteria

-  25.01.96 Original    By: ACRM
-  02.06.99 Added NULL antecedent handling to allow calculation of
            HBonds with missing antecedents. Previously AtomP==NULL
            was handled as an invalid HBond.
-  07.07.14 Use bl prefix for functions By: CTP
*/
BOOL blValidHBond(PDB *AtomH, PDB *AtomD, PDB *AtomA, PDB *AtomP)
{
   REAL ang1, ang2;
   
   if(AtomD==NULL || AtomA==NULL)
   {
      return(FALSE);
   }
   
   if(AtomH==NULL)
   {
      /* If antecedent not defined then just check the distance         */
      if(AtomP==NULL)
      {
         if(DISTSQ(AtomD, AtomA) < DADISTSQ)
            return(TRUE);
         else
            return(FALSE);
      }

      ang1 = blangle(AtomP->x, AtomP->y, AtomP->z,
                     AtomA->x, AtomA->y, AtomA->z,
                     AtomD->x, AtomD->y, AtomD->z);

      if((DISTSQ(AtomD, AtomA) < DADISTSQ) &&
         ang1 >= PI/2.0                    &&
         ang1 <= PI)
         return(TRUE);
   }
   else
   {
      ang2 = blangle(AtomD->x, AtomD->y, AtomD->z,
                     AtomH->x, AtomH->y, AtomH->z,
                     AtomA->x, AtomA->y, AtomA->z);

      /* If the antecedent isn't defined then just set ang1 to within
         the allowed range
      */
      if(AtomP==NULL)
      {
         ang1 = 3.0*PI/4.0;
      }
      else
      {
         ang1 = blangle(AtomP->x, AtomP->y, AtomP->z,
                        AtomA->x, AtomA->y, AtomA->z,
                        AtomH->x, AtomH->y, AtomH->z);
      }

      if((DISTSQ(AtomH, AtomA) < HADISTSQ) &&
         ang1 >= PI/2.0                    &&
         ang1 <= PI                        &&
         ang2 >= PI/2.0                    &&
         ang2 <= PI)
         return(TRUE);
   }
   
   return(FALSE);
}


/************************************************************************/
/*>static BOOL FindBackboneAcceptor(PDB *res, PDB **AtomA, PDB **AtomP)
   --------------------------------------------------------------------
*//**

   \param[in]     *res       Pointer to residue of interest
   \param[out]    **AtomA    The acceptor atom
   \param[out]    **AtomP    The antecedent (previous) atom
   \return                    Success?

   Finds pointers to backbone acceptor atoms

-  25.01.96 Original    By: ACRM
-  07.07.14 Use bl prefix for functions By: CTP
*/
static BOOL FindBackboneAcceptor(PDB *res, PDB **AtomA, PDB **AtomP)
{
   PDB *p,
       *NextRes;

   if(res == NULL)
   {
      return(FALSE);
   }

   *AtomA = NULL;
   *AtomP = NULL;

   NextRes = blFindNextResidue(res);
   for(p=res; p!=NextRes; NEXT(p))
   {
      if(!strncmp(p->atnam,"O   ",4) ||
         !strncmp(p->atnam,"OXT ",4) ||
         !strncmp(p->atnam,"O1  ",4) ||
         !strncmp(p->atnam,"O2  ",4))
         *AtomA = p;
      if(!strncmp(p->atnam,"C   ",4))
         *AtomP = p;
   }

   if(*AtomA && *AtomP)
      return(TRUE);
   
   return(FALSE);
}


/************************************************************************/
/*>static BOOL FindBackboneDonor(PDB *res, PDB **AtomH, PDB **AtomD)
   -----------------------------------------------------------------
*//**

   \param[in]     *res       Pointer to residue of interest
   \param[out]    **AtomH    The hydrogen
   \param[out]    **AtomD    The 'donor' atom
   \return                    Success?

   Finds pointers to backbone donor atoms

-  25.01.96 Original    By: ACRM
-  03.01.06 Returns FALSE if this is a proline
-  07.07.14 Use bl prefix for functions By: CTP
*/
static BOOL FindBackboneDonor(PDB *res, PDB **AtomH, PDB **AtomD)
{
   PDB *p,
       *NextRes;

   if((res == NULL) || (!strncmp(res->resnam, "PRO", 3)))
   {
      return(FALSE);
   }

   *AtomH = NULL;
   *AtomD = NULL;

   NextRes = blFindNextResidue(res);
   for(p=res; p!=NextRes; NEXT(p))
   {
      if(!strncmp(p->atnam,"H   ",4))
         *AtomH = p;
      if(!strncmp(p->atnam,"N   ",4))
         *AtomD = p;
   }

   if(*AtomD)
      return(TRUE);
   
   return(FALSE);
}


/************************************************************************/
/*>static BOOL FindSidechainAcceptor(PDB *res, PDB **AtomA, PDB **AtomP)
   ---------------------------------------------------------------------
*//**

   \param[in]     *res       Pointer to residue of interest
   \param[out]    **AtomA    The acceptor atom
   \param[out]    **AtomP    The antecedent (previous) atom
   \return                    Success?

   Finds pointers to sidechain acceptor atoms. Each call will return
   a new set of atoms till all are found.

   Call with all parameters set to NULL before each new residue

-  25.01.96 Original    By: ACRM
-  09.02.96 Added #ifdef'd code to allow AE1/AE2 AD1/AD2
-  18.08.05 Fixed bug relating to sidechains like GLN/ASN - when finding
            the antecedent for the NE2 and ND2 atoms respectively, it
            would find OE1/OD1 rather than CD/CG. (See pprev code)
-  07.07.14 Use bl prefix for functions By: CTP
*/
static BOOL FindSidechainAcceptor(PDB *res, PDB **AtomA, PDB **AtomP)
{
   static PDB  *p     = NULL,
               *prev  = NULL,
               *pprev = NULL,
               *NextRes;
   static BOOL First = TRUE;
   
   if(res == NULL)
   {
      /* Clear statics                                                  */
      p      = NULL;
      prev   = NULL;
      /* ACRM+++ 18.08.05 */
      pprev  = NULL;
      First  = TRUE;
      
      return(FALSE);
   }
   if(First)
   {
      First   = FALSE;
      p       = res;
      NextRes = blFindNextResidue(res);
   }

   for( ; p!=NextRes; NEXT(p))
   {
      if((p->atnam[0] == 'O' && strncmp(p->atnam, "O   ", 4)
                             && strncmp(p->atnam, "O1  ", 4)
                             && strncmp(p->atnam, "O2  ", 4)
                             && strncmp(p->atnam, "OXT ", 4)) ||
#ifdef ALLOW_AXN
         (p->atnam[0] == 'A') ||
#endif
         (p->atnam[0] == 'N' && strncmp(p->atnam, "N   ", 4)))
      {
         *AtomA = p;
         *AtomP = prev;

/* ACRM+++ 18.08.05 */
         if((*AtomP) && 
            ((*AtomP)->atnam_raw[2] == (*AtomA)->atnam_raw[2]))
         {
            *AtomP = pprev;
         }
         pprev  = prev;
/* ACRM=== */

         prev   = p;
         NEXT(p);

         return(TRUE);
      }
      /* ACRM+++ 18.08.05 */
      pprev  = prev;

      prev = p;
   }

   return(FALSE);
}


/************************************************************************/
/*>static BOOL FindSidechainDonor(PDB *res, PDB **AtomH, PDB **AtomD)
   ------------------------------------------------------------------
*//**

   \param[in]     *res       Pointer to residue of interest
   \param[out]    **AtomH    The hydrogen
   \param[out]    **AtomD    The 'donor' atom
   \return                    Success?

   Finds pointers to sidechain donor atoms. Each call will return
   a new set of atoms till all are found.

   Call with all parameters set to NULL before each new residue

-  25.01.96 Original    By: ACRM
-  09.02.96 Added #ifdef'd code to allow AE1/AE2 AD1/AD2
-  19.12.02 Fixed bug in walking over multiple hydrogens. This fixes
            a problem when the last ATOM residue is a donor and is
            followed by only water (or mercury!) HETATMs
            Also fixed a bug where a lone residue occurs which appears
            to have just a hydrogen.
-  07.07.14 Use bl prefix for functions By: CTP
*/
static BOOL FindSidechainDonor(PDB *res, PDB **AtomH, PDB **AtomD)
{
   static PDB  *p    = NULL,
               *prev = NULL,
               *NextRes;
   static BOOL First = TRUE;
   
   if(res == NULL)
   {
      /* Clear statics                                                  */
      p     = NULL;
      prev  = NULL;
      First = TRUE;
      
      return(FALSE);
   }
   if(First)
   {
      First   = FALSE;
      p       = res;
      NextRes = blFindNextResidue(res);
   }

   for( ; p!=NextRes; NEXT(p))
   {
#ifdef ALLOW_AXN
      if(p->atnam[0] == 'A')
      {
         *AtomD = p;
         *AtomH = NULL;
         prev   = p;
         NEXT(p);

         return(TRUE);
      }
#endif
      if(p->atnam[0] == 'H' && strncmp(p->atnam, "H   ", 4))
      {
         *AtomH = p;
         *AtomD = prev;

         if(*AtomD == NULL)
            continue;
         
         /* Step over any multiple hydrogens                            */
         while((p!=NULL) && (p!=NextRes) && (p->atnam[0] == 'H'))
         {
            prev = p;
            NEXT(p);
         }

         /* If the donor is an oxygen or a nitrogen on a lysine, the 
            preceding torsion is rotatable, so we don't know where the 
            H will be 
         */
         if( (*AtomD)->atnam[0] == 'O' || 
            ((*AtomD)->atnam[0] == 'N' && 
             !strncmp((*AtomD)->resnam,"LYS",3)))
         {
            *AtomH = NULL;
         }
         
         return(TRUE);
      }
      prev = p;
   }

   return(FALSE);
}

/************************************************************************/
/*>int blIsMCDonorHBonded(PDB *res1, PDB *res2, int type)
   ------------------------------------------------------
*//**

   \param[in]     *res1     First residue
   \param[in]     *res2     Second residue
   \param[in]     type      HBond type to search for
   \return                    HBond type found or 0 if none

   Determines whether 2 residues are H-bonded using the crude criteria
   of Baker & Hubbard, 1984 (Prog. Biophys. & Mol. Biol, 44, 97-179)

   N.B. Explicit hydrogens must be added to the PDB linked list before
   calling this routine!

   Searches for HBonds, in which the first residue is a mainchain donor
   and the second residue is an acceptor. Type should be
   HBOND_BACK2 or HBOND_SIDE2 depending whether the second residue
   (the acceptor) is backbond or sidechain

-  17.01.06 Original modified from IsHbonded()    By: ACRM
-  07.07.14 Use bl prefix for functions By: CTP
*/
int blIsMCDonorHBonded(PDB *res1, PDB *res2, int type)
{
   PDB *AtomH,              /* The hydrogen                             */
       *AtomD,              /* The hydrogen donor                       */
       *AtomA,              /* The acceptor                             */
       *AtomP;              /* The acceptor's antecedent                */

   /* Find H-bonds involving the backbone of res1                       */
   if(FindBackboneDonor(res1, &AtomH, &AtomD))
   {
      if(ISSET(type, HBOND_BACK2))
      {
         if(FindBackboneAcceptor(res2, &AtomA, &AtomP))
         {
            if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
               return(HBOND_BACK2);
         }
      }
      if(ISSET(type, HBOND_SIDE2))
      {
         /* Clear internal flags                                     */
         FindSidechainAcceptor(NULL, NULL, NULL);
         while(FindSidechainAcceptor(res2, &AtomA, &AtomP))
         {
            if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
               return(HBOND_SIDE2);
         }
      }
   }

   return(0);
}

/************************************************************************/
/*>int blIsMCAcceptorHBonded(PDB *res1, PDB *res2, int type)
   ---------------------------------------------------------
*//**

   \param[in]     *res1     First residue
   \param[in]     *res2     Second residue
   \param[in]     type      HBond type to search for
   \return                    HBond type found or 0 if none

   Determines whether 2 residues are H-bonded using the crude criteria
   of Baker & Hubbard, 1984 (Prog. Biophys. & Mol. Biol, 44, 97-179)

   N.B. Explicit hydrogens must be added to the PDB linked list before
   calling this routine!

   Searches for HBonds, in which the first residue is a mainchain donor
   and the second residue is an acceptor. Type should be
   HBOND_BACK2 or HBOND_SIDE2 depending whether the second residue
   (the acceptor) is backbond or sidechain

-  17.01.06 Original    By: ACRM
-  07.07.14 Use bl prefix for functions By: CTP
*/
int blIsMCAcceptorHBonded(PDB *res1, PDB *res2, int type)
{
   PDB *AtomH,              /* The hydrogen                             */
       *AtomD,              /* The hydrogen donor                       */
       *AtomA,              /* The acceptor                             */
       *AtomP;              /* The acceptor's antecedent                */

   /* Find H-bonds involving the backbone of res1                       */
   if(FindBackboneAcceptor(res1, &AtomA, &AtomP))
   {
      if(ISSET(type, HBOND_BACK2))
      {
         if(FindBackboneDonor(res2, &AtomH, &AtomD))
         {
            if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
               return(HBOND_BACK2);
         }
      }
      if(ISSET(type, HBOND_SIDE2))
      {
         /* Clear internal flags                                     */
         FindSidechainDonor(NULL, NULL, NULL);
         while(FindSidechainDonor(res2, &AtomH, &AtomD))
         {
            if(blValidHBond(AtomH, AtomD, AtomA, AtomP))
               return(HBOND_BACK1|HBOND_SIDE2);
         }
      }
   }

   return(0);
}





      
   
   
/*************************************************************************
**                                                                      **
**                           DEMO CODE                                  **
**                                                                      **
*************************************************************************/
#ifdef DEMO_CODE

#include <stdio.h>

int main(int argc, char **argv)
{
   FILE *fp, 
        *pgp;
   PDB  *pdb,
        *p, *q;
   int  natom, 
        nhyd;
   BOOL noenv;
   
   if(argc != 2 || argv[1][0] == '-')
   {
      fprintf(stderr,"Usage: hbond <file.pdb>\n");
      fprintf(stderr,"Generates a list of all HBonds in the PDB file\n");
      return(0);
   }
   
   /* Open the PDB file                                                 */
   if((fp = fopen(argv[1],"r")))
   {
      /* Open the PGP file                                              */
      if((pgp = OpenFile("Explicit.pgp","DATADIR","r",&noenv)))
      {
         /* Read the PDB file                                           */
         if((pdb  = blReadPDB(fp, &natom)))
         {
            /* Add hydrogens                                            */
            if((nhyd = blHAddPDB(pgp, pdb)))
            {
               /* Loop through each residue                             */
               for(p=pdb; p!=NULL; p=blFindNextResidue(p))
               {
                  /* Loop through each following residue                */
                  for(q=blFindNextResidue(p); q!=NULL; q=blFindNextResidue(q))
                  {
                     /* Print message if there is any HBond             */
                     if(blIsHBonded(p, q, HBOND_ANY))
                        printf("%s%d%s HBonded to %s%d%s\n",
                               p->chain, p->resnum, p->insert,
                               q->chain, q->resnum, q->insert);
                  }
               }
            }
            else
            {
               fprintf(stderr,"No hydrogens added to PDB file\n");
            }
         }
         else
         {
            fprintf(stderr,"No atoms read from PDB file\n");
         }
      }
      else
      {
         fprintf(stderr,"Unable to open PGP file: Explicit.pgp\n");
      }
   }
   else
   {
      fprintf(stderr,"Unable to open PDB file: %s\n");
   }
   
   return(0);
}
#endif
