#include "getpdbchainlabels_suite.h"

/* Globals */
static char chainlabels[6][8] = {"A","B","B","C","C","D"};
PDB *pdb, *p;
int i, nchains;
char **chains;

/* Setup And Teardown */
void getpdbchainlabels_setup(void)
{
   /* Set PDB list chains */
   INIT(pdb,PDB);
   strcpy(pdb->chain,chainlabels[0]);
   
   for(p = pdb, i = 1; i < 6; i++)
   {
      ALLOCNEXT(p,PDB);
      strcpy(p->chain,chainlabels[i]);
   }
}

void getpdbchainlabels_teardown(void)
{
   FREELIST(pdb,PDB);
}

/* Core tests */
START_TEST(test_01)
{
   chains = blGetPDBChainLabels(pdb,&nchains);
   
   ck_assert(nchains == 4);
   ck_assert(CHAINMATCH(chains[0],"A"));
   ck_assert(CHAINMATCH(chains[1],"B"));
   ck_assert(CHAINMATCH(chains[2],"C"));
   ck_assert(CHAINMATCH(chains[3],"D"));
}
END_TEST

START_TEST(test_02)
{
   chains = blGetPDBChainLabels(NULL,&nchains);
   
   ck_assert(nchains == 0);
}
END_TEST


/* Create Suite */
Suite *getpdbchainlabels_suite(void)
{
   Suite *s = suite_create("GetPDBChainLabels");
   
   /* blGetPDBChainLabels() */
   TCase *tc_core = tcase_create("Core");
   tcase_add_checked_fixture(tc_core, getpdbchainlabels_setup, 
                             getpdbchainlabels_teardown);
   tcase_add_test(tc_core, test_01);
   tcase_add_test(tc_core, test_02);
   suite_add_tcase(s, tc_core);

   return(s);
}
