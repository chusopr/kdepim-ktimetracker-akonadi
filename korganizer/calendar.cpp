// Calendar class for KOrganizer
// (c) 1998 Preston Brown
// 	$Id$

#include "config.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>

#include <qclipboard.h>
#include <qdialog.h>
#include <qfile.h>

#include <kapp.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "vcaldrag.h"
#include "qdatelist.h"
#include "koprefs.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "koexceptions.h"
#include "calfilter.h"

#include "calendar.h"
#include "calendar.moc"

extern "C" {
  char *parse_holidays(const char *, int year, short force);
  struct holiday {
    char            *string;        /* name of holiday, 0=not a holiday */
    unsigned short  dup;            /* reference count */
  };
  extern struct holiday holiday[366];
};

class AddIncidenceVisitor : public IncidenceVisitor {
  public:
    AddIncidenceVisitor(Calendar *calendar) : mCalendar(calendar) {}
    
    bool visit(Event *e) { mCalendar->addEvent(e); return true; }
    bool visit(Todo *t) { mCalendar->addTodo(t); return true; }

  private:
    Calendar *mCalendar;
};

Calendar::Calendar()
  : QObject()
{
  mDndFormat = new VCalFormat(this);
  
  mFormat = 0;

#if 0
  switch (KOPrefs::instance()->mDefaultFormat) {
    case KOPrefs::FormatVCalendar: 
      mFormat = new VCalFormat(this);
      break;
    case KOPrefs::FormatICalendar:
      mFormat = new ICalFormat(this);
      break;
    default:
      mFormat = new ICalFormat(this);
      break;
  }
#endif
  
  mICalFormat = new ICalFormat(this);

  // Setup default filter, which does nothing
  mFilter = new CalFilter;
  mFilter->setEnabled(false);

  struct passwd *pwent;
  uid_t userId;
  QString tmpStr;

  mDialogsOn = true;

  // initialize random numbers.  This is a hack, and not
  // even that good of one at that.
//  srandom(time(0L));

  // user information...
  userId = getuid();
  pwent = getpwuid(userId);
  tmpStr = KOPrefs::instance()->mName;
  if (tmpStr.isEmpty()) {    
    if (strlen(pwent->pw_gecos) > 0)
      setOwner(pwent->pw_gecos);
    else
      setOwner(pwent->pw_name);
      KOPrefs::instance()->mName = getOwner();
  } else {
    setOwner(tmpStr);
  }

  tmpStr = KOPrefs::instance()->mEmail;
  if (tmpStr.isEmpty()) {
    tmpStr = pwent->pw_name;
    tmpStr += "@";

#ifdef HAVE_GETHOSTNAME
    char cbuf[80];
    if (gethostname(cbuf, 79)) {
      // error getting hostname
      tmpStr += "localhost";
    } else {
      hostent he;
      if (gethostbyname(cbuf)) {
	  he = *gethostbyname(cbuf);
	  tmpStr += he.h_name;
      } else {
	// error getting hostname
	tmpStr += "localhost";
      }
    }
#else
    tmpStr += "localhost";
#endif
    setEmail(tmpStr);
    KOPrefs::instance()->mEmail = tmpStr;
  } else {
    setEmail(tmpStr);
  }

  readHolidayFileName();

  tmpStr = KOPrefs::instance()->mTimeZone;
//  kdDebug() << "Calendar::Calendar(): TimeZone: " << tmpStr << endl;
  int dstSetting = KOPrefs::instance()->mDaylightSavings;
  extern long int timezone;
  struct tm *now;
  time_t curtime;
  curtime = time(0);
  now = localtime(&curtime);
  int hourOff = - ((timezone / 60) / 60);
  if (now->tm_isdst)
    hourOff += 1;
  QString tzStr;
  tzStr.sprintf("%.2d%.2d",
		hourOff, 
		abs((timezone / 60) % 60));

  // if no time zone was in the config file, write what we just discovered.
  if (tmpStr.isEmpty()) {
    KOPrefs::instance()->mTimeZone = tzStr;
  } else {
    tzStr = tmpStr;
  }
  
  // if daylight savings has changed since last load time, we need
  // to rewrite these settings to the config file.
  if ((now->tm_isdst && !dstSetting) ||
      (!now->tm_isdst && dstSetting)) {
    KOPrefs::instance()->mTimeZone = tzStr;
    KOPrefs::instance()->mDaylightSavings = now->tm_isdst;
  }
  
  setTimeZone(tzStr);

  KOPrefs::instance()->writeConfig();
}

Calendar::~Calendar() 
{
  delete mICalFormat;
  delete mDndFormat;
  delete mFormat;
}

ICalFormat *Calendar::iCalFormat()
{
  return mICalFormat;
}

VCalDrag *Calendar::createDrag(Event *selectedEv, QWidget *owner)
{
  return mDndFormat->createDrag(selectedEv,owner);
}

VCalDrag *Calendar::createDragTodo(Todo *selectedEv, QWidget *owner)
{
  return mDndFormat->createDragTodo(selectedEv,owner);
}

Event *Calendar::createDrop(QDropEvent *de)
{
  return mDndFormat->createDrop(de);
}

Todo *Calendar::createDropTodo(QDropEvent *de)
{
  kdDebug() << "Calendar::createDropTodo()" << endl;
  return mDndFormat->createDropTodo(de);
}

void Calendar::cutEvent(Event *selectedEv)
{
  if (copyEvent(selectedEv))
    deleteEvent(selectedEv);
}

bool Calendar::copyEvent(Event *selectedEv)
{
  return mDndFormat->copyEvent(selectedEv);
}

Event *Calendar::pasteEvent(const QDate *newDate,const QTime *newTime)
{
  return mDndFormat->pasteEvent(newDate,newTime);
}

const QString &Calendar::getOwner() const
{
  return mOwner;
}

void Calendar::setOwner(const QString &os)
{
  int i;
  // mOwner = os.ascii(); // to detach it
  mOwner = os; // #### Why? This should be okay?
  i = mOwner.find(',');
  if (i != -1)
    mOwner = mOwner.left(i);
}

void Calendar::setTimeZone(const QString & tz)
{
  bool neg = FALSE;
  int hours, minutes;
  QString tmpStr(tz);

  if (tmpStr.left(1) == "-")
    neg = TRUE;
  if (tmpStr.left(1) == "-" || tmpStr.left(1) == "+")
    tmpStr.remove(0, 1);
  hours = tmpStr.left(2).toInt();
  if (tmpStr.length() > 2) 
    minutes = tmpStr.right(2).toInt();
  else
    minutes = 0;
  mTimeZone = (60*hours+minutes);
  if (neg)
    mTimeZone = -mTimeZone;
}

QString Calendar::getTimeZoneStr() const 
{
  QString tmpStr;
  int hours = abs(mTimeZone / 60);
  int minutes = abs(mTimeZone % 60);
  bool neg = mTimeZone < 0;

  tmpStr.sprintf("%c%.2d%.2d",
		 (neg ? '-' : '+'),
		 hours, minutes);
  return tmpStr;
}

const QString &Calendar::getEmail()
{
  return mOwnerEmail;
}

void Calendar::setEmail(const QString &e)
{
  mOwnerEmail = e;
}

void Calendar::setTimeZone(int tz)
{
  mTimeZone = tz;
}

int Calendar::getTimeZone() const
{
  return mTimeZone;
}

void Calendar::showDialogs(bool d)
{
  mDialogsOn = d;
}

// don't ever call this unless a kapp exists!
void Calendar::updateConfig()
{
//  kdDebug() << "Calendar::updateConfig()" << endl;

  bool updateFlag = FALSE;
  
  // TODO: update Organizer in all events
#if 0
  mOwner = KOPrefs::instance()->mName;

  // update events to new organizer (email address) 
  // if it has changed...
  QString configEmail = KOPrefs::instance()->mEmail;

  if (mOwnerEmail != configEmail) {
    QString oldEmail = mOwnerEmail;
    //    oldEmail.detach();
    mOwnerEmail = configEmail;
    Event *firstEvent, *currEvent;
    bool atFirst = TRUE;

    firstEvent = last();
    // gotta skip over the first one, which is same as first. 
    // I know, bad coding.
    for (currEvent = prev(); currEvent; currEvent = prev()) {
//      kdDebug() << "in Calendar::updateConfig(), currEvent summary: " << currEvent->getSummary() << endl;
      if ((currEvent == firstEvent) && !atFirst) {
	break;
      }
      if (currEvent->getOrganizer() == oldEmail) {
	currEvent->setReadOnly(FALSE);
	currEvent->setOrganizer(mOwnerEmail);
        updateFlag = TRUE;
      }
      atFirst = FALSE;
    }
  }
#endif
  readHolidayFileName();

  setTimeZone(KOPrefs::instance()->mTimeZone.latin1());

  if (updateFlag)
    emit calUpdated((Event *) 0L);
}

QString Calendar::getHolidayForDate(const QDate &qd)
{
  static int lastYear = 0;

//  kdDebug() << "CalendarLocal::getHolidayForDate(): Holiday: " << holidays << endl;

  if (mHolidayfile.isEmpty()) return (QString(""));

  if ((lastYear == 0) || (qd.year() != lastYear)) {
      lastYear = qd.year() - 1900; // silly parse_year takes 2 digit year...
      parse_holidays(QFile::encodeName(mHolidayfile), lastYear, 0);
  }

  if (holiday[qd.dayOfYear()-1].string) {
    QString holidayname = QString::fromLocal8Bit(holiday[qd.dayOfYear()-1].string);
//    kdDebug() << "Holi name: " << holidayname << endl;
    return(holidayname);
  } else {
//    kdDebug() << "No holiday" << endl;
    return(QString(""));
  }
}

void Calendar::readHolidayFileName()
{
  QString holidays(KOPrefs::instance()->mHoliday);
  if (holidays == "(none)") mHolidayfile = "";
  holidays = holidays.prepend("holiday_");
  mHolidayfile = locate("appdata",holidays);

//  kdDebug() << "holifile: " << mHolidayfile << endl;
}

void Calendar::setFilter(CalFilter *filter)
{
  mFilter = filter;
}

CalFilter *Calendar::filter()
{
  return mFilter;
}

QList<Event> Calendar::getEventsForDate(const QDate &date,bool sorted)
{
  QList<Event> el = eventsForDate(date,sorted);
  mFilter->apply(&el);
  return el;
}

QList<Event> Calendar::getEventsForDate(const QDateTime &qdt)
{
  QList<Event> el = eventsForDate(qdt);
  mFilter->apply(&el);
  return el;
}

QList<Event> Calendar::getEvents(const QDate &start,const QDate &end,
                                    bool inclusive)
{
  QList<Event> el = events(start,end,inclusive);
  mFilter->apply(&el);
  return el;
}


void Calendar::addIncidence(Incidence *i)
{
  AddIncidenceVisitor v(this);

  i->accept(v);
}
