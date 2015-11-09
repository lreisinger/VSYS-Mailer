//
//  ldap.cpp
//  TWMailer
//
//  Created by Lukas Reisinger on 04.11.15.
//  Copyright (c) 2015 Lukas Reisinger. All rights reserved.
//


#include <cstring>
#include "ldap.h"
#include <stdlib.h>

#include <ldap.h>
/* LDAP Server settings */
#define LDAP_HOST "ldap.technikum-wien.at"
#define LDAP_PORT 389
#define SEARCHBASE "dc=technikum-wien,dc=at"
#define SCOPE LDAP_SCOPE_SUBTREE

int login(char* user, char* pass)//1 = success, 0 = wrong pass, -1 = wrong user, -2 = conn failed, -3 ldap search error
{
    
    //test usernames, pw egal
    if(strcmp(user, "wrong") == 0){
        return 0;
    }else if(strcmp(user, "Dominik1") == 0){
        return 1;
    }else if(strcmp(user, "lukas0x3") == 0){
        return 1;
    }
    
    
    
    //Step0:init ldap connection
    //Step1:anonym anmelden
    //step2:nach user suchen und dn herausfinden
    //step3:anmeldung mit dn und passwort
    
#pragma region step0
    
    LDAP *ld;
    int rc = 0;
    
    
    LDAPMessage *result, *e;	/* LDAP result handle */
    BerElement *ber;		/* array of attributes */
    char *attribute;
    char **vals;
    
    
    
    /* Open LDAP Connection */
    if ((ld=ldap_init(LDAP_HOST, LDAP_PORT)) == NULL)
    {
        perror("ldap_init failed");
        return -2;
    }
#pragma endregion step0
    
    
    
#pragma region step1
    
    /* User authentication (bind) */
    rc = ldap_simple_bind_s( ld, NULL, NULL );
    if( rc != LDAP_SUCCESS )
    {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc) );
        return 0;
    }

#pragma endregion step1
    
    
    
#pragma region step2
    
    
    char bind_dn[100];
    sprintf( bind_dn, "cn=%s,ou=People,dc=nafiux,dc=com", user );
    printf( "Connecting as %s...\n", bind_dn );
    
    char *attribs[3];		/* attribute array for search */
    
    attribs[0]=strdup("uid");		/* return uid and dn of entries */
    attribs[1]=strdup("dn");
    attribs[2]=NULL;		/* array must be NULL terminated */
    
    
    char filter[15];
    strcpy(filter, "(uid=");
    strcat(filter, user);
    strcat(filter, ")");
    
    rc = ldap_search_s(ld, SEARCHBASE, SCOPE, filter, attribs, 0, &result);
    
    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr,"LDAP search error: %s\n",ldap_err2string(rc));
        return -3;
    }
    
    printf("Total results: %d\n", ldap_count_entries(ld, result));
    
    bool found = false;
    for (e = ldap_first_entry(ld, result); e != NULL; e = ldap_next_entry(ld,e))
    {
        printf("DN: %s\n", ldap_get_dn(ld,e));
        
        /* Now print the attributes and values of each found entry */
        
        for (attribute = ldap_first_attribute(ld,e,&ber); attribute!=NULL; attribute = ldap_next_attribute(ld,e,ber))
        {
            if ((vals=ldap_get_values(ld,e,attribute)) != NULL)
            {
                for (int i=0;vals[i]!=NULL;i++)
                {
                    printf("%s: %s\n",attribute,vals[i]);
                    if(strcmp(attribute, "dn")){
                        strcpy(bind_dn, vals[i]);
                        found = true;
                    }
                    if(found)
                        break;
                }
                /* free memory used to store the values of the attribute */
                ldap_value_free(vals);
            }
            /* free memory used to store the attribute */
            ldap_memfree(attribute);
            
            if(found)
                break;
        }
        /* free memory used to store the value structure */
        if (ber != NULL) ber_free(ber,0);
        
        printf("\n");
        
        if(found)
            break;
    }
    
    /* free memory used for result */
    ldap_msgfree(result);
    free(attribs[0]);
    free(attribs[1]);
    printf("LDAP search suceeded\n");

    if(!found){
        return -1;
    }
#pragma endregion step2
    
    
    
#pragma region step3
    
    /* User authentication (bind) */
    rc = ldap_simple_bind_s( ld, bind_dn, pass );
    if( rc != LDAP_SUCCESS )
    {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc) );
        ldap_unbind( ld );
        return 0;
    }
    else{
        printf( "Successful authentication\n" );
        ldap_unbind( ld );
        return 1;
    }
#pragma endregion step3
    
    
}
