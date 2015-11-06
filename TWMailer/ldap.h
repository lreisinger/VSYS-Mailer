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

struct user_ldap {
    int sd;
    char username[9];
    int retries;
    char ip[16];
} ;



#endif /* defined(__TWMailer__ldap__) */

