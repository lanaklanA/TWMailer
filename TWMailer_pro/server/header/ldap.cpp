///////////////////////////////////////////////////////////////////////////////
// C++/11 Headers Includes
#include <iostream>
#include <vector>
// #include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

///////////////////////////////////////////////////////////////////////////////
// C Headers Includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ldap.h>

///////////////////////////////////////////////////////////////////////////////
// Own Headers Includes
#include "ldap.h"

///////////////////////////////////////////////////////////////////////////////
// Initialize
LDAP *ldap_init()
{
   LDAP *ldapHandle;
   const int ldapVersion = LDAP_VERSION3;
   const char *ldapUri = "ldap://ldap.technikum-wien.at:389";

   // init ldap
   int rc = ldap_initialize(&ldapHandle, ldapUri);

   // check if function above worked
   if (rc != LDAP_SUCCESS) {
      fprintf(stderr, "ldap_init failed\n");
      return EXIT_SUCCESS;
   }
   printf("connected to LDAP server %s\n", ldapUri);

   // set option
   rc = ldap_set_option(
       ldapHandle,
       LDAP_OPT_PROTOCOL_VERSION, // OPTION
       &ldapVersion);             // IN-Value

   // check if function above worked
   if (rc != LDAP_OPT_SUCCESS)
   {
      fprintf(stderr, "ldap_set_option(PROTOCOL_VERSION): %s\n", ldap_err2string(rc));
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      // return EXIT_FAILURE;
   }

   // start tls
   rc = ldap_start_tls_s(
       ldapHandle,
       NULL,
       NULL);
   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr, "ldap_start_tls_s(): %s\n", ldap_err2string(rc));
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      // return EXIT_FAILURE;
   }

   return ldapHandle;
}

///////////////////////////////////////////////////////////////////////////////
// Bind
int login_and_bind(char *username, char *password, LDAP *ldapHandle)
{
   char ldapBindUser[256], rawLdapUser[128], ldapBindPassword[256];

   // Fill variables with valid values (username and password)
   strcpy(rawLdapUser, username);
   sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", rawLdapUser);
   strcpy(ldapBindPassword, password);
   printf("User set to: %s\n", ldapBindUser);

   // bind credential with ldap
   BerValue bindCredentials, *servercredp;
   bindCredentials.bv_val = (char *)ldapBindPassword;
   bindCredentials.bv_len = strlen(ldapBindPassword);
   int rc = ldap_sasl_bind_s(
       ldapHandle,
       ldapBindUser,
       LDAP_SASL_SIMPLE,
       &bindCredentials,
       NULL,
       NULL,
       &servercredp);

   // check if function above worked
   if (rc != LDAP_SUCCESS) {
      fprintf(stderr, "LDAP bind error: %s\n", ldap_err2string(rc));
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      return rc;
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Search for User
int search_user(char *filter, LDAP *ldapHandle)
{
   const char *ldapSearchBaseDomainComponent = "dc=technikum-wien,dc=at";
   const char *ldapSearchFilter = (const char *)filter;
   ber_int_t ldapSearchScope = LDAP_SCOPE_SUBTREE;
   const char *ldapSearchResultAttributes[] = {"uid", "cn", NULL};

   LDAPMessage *searchResult;
   int rc = ldap_search_ext_s(
       ldapHandle,
       ldapSearchBaseDomainComponent,
       ldapSearchScope,
       ldapSearchFilter,
       (char **)ldapSearchResultAttributes,
       0,
       NULL,
       NULL,
       NULL,
       500,
       &searchResult);

   // check if function above worked
   if (rc != LDAP_SUCCESS)   {
      fprintf(stderr, "LDAP search error: %s\n", ldap_err2string(rc));
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      return EXIT_FAILURE;
   }
   
   rc = ldap_count_entries(ldapHandle, searchResult);

   ldap_msgfree(searchResult);
   return rc;
}