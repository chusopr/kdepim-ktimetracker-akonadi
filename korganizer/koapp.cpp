// $Id$

#include <stdio.h>
#include <stdlib.h>

#include <kstddirs.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kwin.h>

#include "calendarlocal.h"
#include "korganizer.h"

#include "koapp.h"
#include "koapp.moc"


KOrganizerApp::KOrganizerApp() : KUniqueApplication()
{
}

KOrganizerApp::~KOrganizerApp()
{
}

void KOrganizerApp::displayImminent(const QString &file,int numdays)
{
  Calendar *cal = new CalendarLocal;

  QDate currDate(QDate::currentDate());
  Event *currEvent;

  if (!cal->load(file)) {
    printf(i18n("Could not load calendar '%1'.\n").arg(file).local8Bit());
    exit(0);
  }

  for (int i = 1; i <= numdays; i++) {
    printf("%s\n",KGlobal::locale()->formatDate(currDate).latin1());

    QList<Event> tmpList(cal->getEventsForDate(currDate, TRUE));
    printf("---------------------------------------------------------------\n");
    if (tmpList.count() > 0) {
      for (currEvent = tmpList.first(); currEvent; currEvent = tmpList.next()) {
        printf("%s",(const char *)currEvent->summary().local8Bit());
        if (!currEvent->doesFloat()) {
          printf(" (%s - %s)",(const char *)currEvent->dtStartStr().local8Bit(),
                 (const char *)currEvent->dtEndStr().local8Bit());
        }
        printf("\n");
      }
    } else {
      printf("(no events)\n");
    }

    printf("---------------------------------------------------------------\n");
    QList<Todo> tmpList2 = cal->getTodosForDate(currDate);
    Todo *currTodo;
    if (tmpList.count() > 0) {
      for (currTodo = tmpList2.first(); currTodo; currTodo = tmpList2.next()) {
        printf("%s",(const char *)currTodo->summary().local8Bit());
        if (!currTodo->doesFloat()) {
          printf(" (%s)",(const char *)currTodo->dtDueStr().local8Bit());
        }
        printf("\n");
      }
    } else {
      printf("(no todos)\n");
    }

    printf("\n");
    currDate = currDate.addDays(1);
  }
}


void KOrganizerApp::startAlarmDaemon()
{
  kdDebug() << "Starting alarm daemon" << endl;

  // Start alarmdaemon. It is a KUniqueApplication, that means it is
  // automatically made sure that there is only one instance of the alarm daemon
  // running.
  QString execStr = locate("exe","alarmd");
  system(execStr.latin1());

  // Force alarm daemon to load active calendar
  if (!dcopClient()->send("alarmd","ad","reloadCal()","")) {
    kdDebug() << "KOrganizerApp::startAlarmDaemon(): dcop send failed" << endl;
  }

  kdDebug() << "Starting alarm daemon done" << endl;
}


int KOrganizerApp::newInstance()
{
  kdDebug() << "KOApp::newInstance()" << endl;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // process command line options
  int numDays = 0;
  if (args->isSet("list")) {
    numDays = 1;
  } else if (args->isSet("show")) {
    numDays = args->getOption("show").toInt();
  } else {
    startAlarmDaemon();
  }

  // If filenames was given as argument load this as calendars, one per window.
  if (args->count() > 0) {
    int i;
    for(i=0;i<args->count();++i) {
      processCalendar(args->arg(i),numDays);
    }
  } else {
    KGlobal::config()->setGroup("General");
    QString file = KGlobal::config()->readEntry("Active Calendar");

    processCalendar(file,numDays,true);
  }
  
  kdDebug() << "KOApp::newInstance() done" << endl;
  return 0;
}

void KOrganizerApp::processCalendar(const QString & file,int numDays,
                                    bool active)
{
  if (numDays > 0) {
    displayImminent(file,numDays);
  } else {
    if (isRestored()) {
      RESTORE(KOrganizer)
    } else {
      KURL url;
      url.setPath(file);
      KOrganizer *korg=KOrganizer::findInstance(url);
      if (0 == korg) {
        korg = new KOrganizer("KOrganizer MainWindow");
        if (!file.isEmpty()) {
          korg->openURL(url);
          korg->setActive(active);
        }
        korg->show();
      } else
          KWin::setActiveWindow(korg->winId());
    }
  }
}
