LDAP *ldap_init();
void login_and_bind(char *username, char *password, LDAP *ldapHandle);
int search_user(char *filter, LDAP *ldapHandle);
