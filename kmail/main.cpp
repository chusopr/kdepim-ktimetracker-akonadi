// KMail startup and initialize code
// Author: Stefan Taferner <taferner@alpin.or.at>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <kuniqueapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <dcopclient.h>
#include <kcrash.h>

#include "kmkernel.h" //control center

#undef Status // stupid X headers
#include "kmailIface_stub.h" // to call control center of master kmail

#include <kaboutdata.h>
#include <kdebug.h>

#include "kmversion.h"


// OLD about text.  This is horrbly outdated.
/*const char* aboutText =
    "KMail [" KMAIL_VERSION "] by\n\n"
    "Stefan Taferner <taferner@kde.org>,\n"
    "Markus W�bben <markus.wuebben@kde.org>\n\n"
    "based on the work of:\n"
    "Lynx <lynx@topaz.hknet.com>,\n"
    "Stephan Meyer <Stephan.Meyer@pobox.com>,\n"
    "and the above authors.\n\n"
    "This program is covered by the GPL.\n\n"
    "Please send bugreports to taferner@kde.org";
*/

static KCmdLineOptions kmoptions[] =
{
  { "s", 0 , 0 },
  { "subject <subject>",	I18N_NOOP("Set subject of msg."), 0 },
  { "c", 0 , 0 },
  { "cc <address>",		I18N_NOOP("Send CC: to 'address'."), 0 },
  { "b", 0 , 0 },
  { "bcc <address>",		I18N_NOOP("Send BCC: to 'addres'."), 0 },
  { "h", 0 , 0 },
  { "header <header>",		I18N_NOOP("Add 'header' to msg."), 0 },
  { "msg <file>",		I18N_NOOP("Read msg-body from 'file'."), 0 },
  { "body <text>",              I18N_NOOP("Set body of msg."), 0 },
  { "attach <url>",     I18N_NOOP("Add an attachment to the mail"), 0 },
  { "check",			I18N_NOOP("Check for new mail only."), 0 },
  { "composer",			I18N_NOOP("Open only composer window."), 0 },
  { "+[address]",		I18N_NOOP("Send msg to 'address'."), 0 },
//  { "+[file]",                  I18N_NOOP("Show message from file 'file'."), 0 },
  { 0, 0, 0}
};

//-----------------------------------------------------------------------------

extern "C" {

static void setSignalHandler(void (*handler)(int));

// Crash recovery signal handler
static void signalHandler(int sigId)
{
  setSignalHandler(SIG_DFL);
  fprintf(stderr, "*** KMail got signal %d (Exiting)\n", sigId);
  // try to cleanup all windows
  kernel->dumpDeadLetters();
  ::exit(-1); //
}

// Crash recovery signal handler
static void crashHandler(int sigId)
{
  setSignalHandler(SIG_DFL);
  fprintf(stderr, "*** KMail got signal %d (Crashing)\n", sigId);
  // try to cleanup all windows
  kernel->dumpDeadLetters();
  // Return to DrKonqi.
}
//-----------------------------------------------------------------------------


static void setSignalHandler(void (*handler)(int))
{
  signal(SIGKILL, handler);
  signal(SIGTERM, handler);
  signal(SIGHUP,  handler);
  KCrash::setEmergencySaveFunction(crashHandler);
}

}
//-----------------------------------------------------------------------------

class KMailApplication : public KUniqueApplication
{
public:
  KMailApplication() : KUniqueApplication() { };

  virtual int newInstance();
};

int KMailApplication::newInstance()
{
  QString to, cc, bcc, subj, body;
  KURL messageFile = QString::null;
  KURL attachURL = QString::null;
  bool mailto = false;
  bool checkMail = false;
  //bool viewOnly = false;

  // process args:
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->getOption("subject"))
  {
     mailto = true;
     subj = args->getOption("subject");
  }

  if (args->getOption("cc"))
  {
     mailto = true;
     cc = args->getOption("cc");
  }

  if (args->getOption("bcc"))
  {
     mailto = true;
     bcc = args->getOption("bcc");
  }

  if (args->getOption("msg"))
  {
     mailto = true;
     messageFile = QString::fromLocal8Bit(args->getOption("msg"));
  }

  if (args->getOption("body"))
  {
     mailto = true;
     body = args->getOption("body");
  }
  
  if (args->getOption("attach"))
  {
     mailto = true;
     attachURL = QString::fromLocal8Bit(args->getOption("attach"));
  }

  if (args->isSet("composer"))
    mailto = true;

  if (args->isSet("check"))
    checkMail = true;

  for(int i= 0; i < args->count(); i++)
  {
    if (!to.isEmpty())
      to += ", ";
     if (strncasecmp(args->arg(i),"mailto:",7)==0)
       to += args->arg(i);
     else
       to += args->arg(i);
     mailto = true;
  }

  args->clear();

  kernel->action (mailto, checkMail, to, cc, bcc, subj, body, messageFile, attachURL) ;
  return 0;
}

int main(int argc, char *argv[])
{
  // WABA: KMail is a KUniqueApplication. Unfortunately this makes debugging
  // a bit harder: You should pass --nofork as commandline argument when using
  // a debugger. In gdb you can do this by typing "set args --nofork" before
  // typing "run".

  KAboutData about("kmail", I18N_NOOP("KMail"),
                   KMAIL_VERSION,
		   I18N_NOOP("A KDE E-Mail client."),
		   KAboutData::License_GPL,
                   I18N_NOOP("(c) 1997-2001, The KMail developers"),
		   0,
		   "http://kmail.kde.org");
  about.addAuthor( "Don Sanders", I18N_NOOP("Current maintainer"), "don@sanders.org" );
  about.addAuthor( "Waldo Bastian", 0, "bastian@kde.org" );
  about.addAuthor( "Andreas Gungl", 0, "a.gungl@gmx.de" );
  about.addAuthor( "Michael Haeckel", 0, "michael@haeckel.net" );
  about.addAuthor( "Lars Knoll", 0, "knoll@kde.org" );
  about.addAuthor( "J. Nick Koston", 0, "bdraco@darkorb.net" );
  about.addAuthor( "Stephan Kulow", 0, "coolo@kde.org" );
  about.addAuthor( "Marc Mutz", 0, "mutz@kde.org" );
  about.addAuthor( "Daniel Naber", 0, "daniel.naber@t-online.de" );
  about.addAuthor( "Sven Radej", 0, "radej@kde.org" );
  about.addAuthor( "Espen Sand", 0, "espen@kde.org" );
  about.addAuthor( "George Staikos", 0, "staikos@kde.org" );
  about.addAuthor( "Stefan Taferner ", 0, "taferner@kde.org" );
  about.addAuthor( "Mario Weilguni", 0, "mweilguni@sime.com" );
  about.addAuthor( "Robert D. Williams", 0, "rwilliams@kde.org" );
  about.addAuthor( "Markus Wuebben", 0, "markus.wuebben@kde.org" );

  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions( kmoptions ); // Add kmail options

  if (!KMailApplication::start())
     return 0;

  KMailApplication app;
  KGlobal::locale()->insertCatalogue("libkdenetwork");

  kapp->dcopClient()->suspend(); // Don't handle DCOP requests yet

  //local, do the init
  KMKernel kmailKernel;
  kmailKernel.init();
  kapp->dcopClient()->setDefaultObject( kmailKernel.objId() );

  // and session management
  kmailKernel.doSessionManagement();

  // any dead letters?
  kmailKernel.recoverDeadLetters();

  setSignalHandler(signalHandler);

  kapp->dcopClient()->resume(); // Ok. We are ready for DCOP requests.
  // Go!
  int ret = kapp->exec();

  // clean up
  kmailKernel.cleanup();
  return ret;
}

