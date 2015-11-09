//
//  ldap.h
//  TWMailer
//
//  Created by Lukas Reisinger on 04.11.15.
//  Copyright (c) 2015 Lukas Reisinger. All rights reserved.
//

#ifndef __TWMailer__ldap__
#define __TWMailer__ldap__

#include <stdio.h>
#include <vector>

extern std::vector<struct user_ldap*> loggedIn;

struct user_ldap {
    int sd;
    char username[9];
    int retries;
    char ip[16];
    int timestamp_lasttry;
} ;

int login(char* user, char* pass);

#endif /* defined(__TWMailer__ldap__) */

