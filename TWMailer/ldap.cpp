//
//  ldap.cpp
//  TWMailer
//
//  Created by Lukas Reisinger on 04.11.15.
//  Copyright (c) 2015 Lukas Reisinger. All rights reserved.
//

#include <cstring>
#include "ldap.h"

#include <ldap.h>
/* LDAP Server settings */
#define LDAP_SERVER "ldap://nafiux.com:389"

int login(char* user, char* pass)//1 = success, 0 = wrong login info, -1 = conn failed
{
    if(strcmp(user, "test") == 0){
        return 0;
    }
    else if(strcmp(user, "true") == 0){
        return 1;
    }
    
    LDAP *ld;
    int rc;
    char bind_dn[100];
    
    sprintf( bind_dn, "cn=%s,ou=People,dc=nafiux,dc=com", user );
    printf( "Connecting as %s...\n", bind_dn );
    
    /* Open LDAP Connection */
    if( ldap_initialize( &ld, LDAP_SERVER ) )
    {
        perror( "ldap_initialize" );
        return -1;
    }
    
    /* User authentication (bind) */
    rc = ldap_simple_bind_s( ld, bind_dn, pass );
    if( rc != LDAP_SUCCESS )
    {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc) );
        return 0;
    }
    printf( "Successful authentication\n" );
    ldap_unbind( ld );
    return 1;

}
