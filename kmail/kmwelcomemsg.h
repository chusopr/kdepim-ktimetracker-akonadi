#ifndef KMWELCOMEMSG_H
#define KMWELCOMEMSG_H

#include "kmglobal.h"
#include <klocale.h>
#include <kapp.h>

//
//  This file contains the welcome message for kmail when the inbox
//  is created
//

static const char *KM_WelcomeMsg =

I18N_NOOP(
"Welcome to KMail!\n"
"\n"
"KMail is an email client for the K Desktop Environment. "
"It is designed to be fully compatible with Internet mailing standards "
"including MIME, SMTP and POP3.\n"
"\n"
"* KMail has many powerful features which are described in the documentation. "
"You can access this documentation under the Help->Contents menu.\n"
"\n"
"* You can find news and updates at the KMail homepage: "
"http://kmail.kde.org\n"
"\n"
"* You can report bugs with the Help->Report Bug menu\n"
"\n"
"Some of the new features in this release of KMail include:\n"
"   + Message threading support\n"
"   + New configuration panel\n"
"   + Background POP3 checking\n"
"   + Multiple identities for sending\n"
"   + Enhanced PGP/GPG support\n"
"   + Numerous minor enhancements and bugfixes\n"
"\n"
"Please take a moment to fill in the KMail configuration panel at "
"Settings->Configuration. You need to at least create a primary identity "
"and setup a mail spool/POP3 account.\n\n"
"We hope that you will enjoy KMail.\n"
"\n"
"Thank you,\n"
"   The KMail Team.\n"
"\n"
      );

#endif
