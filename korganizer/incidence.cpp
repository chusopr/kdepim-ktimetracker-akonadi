// $Id$

#include "calformat.h"
#include "koprefs.h"

#include "incidence.h"
#include "incidence.moc"

Incidence::Incidence() :
  KORecurrence(this),
  KOAlarm(this)
{
  mReadOnly = false;

  recreate();

  setOrganizer(KOPrefs::instance()->mEmail);
  if (organizer().isEmpty())
    setOrganizer("x-none");
  
  mSummary = "";
  mDescription = "";

  mRelatedTo = 0;

  mExDates.setAutoDelete(true);
}

Incidence::~Incidence()
{
  Incidence *ev;
  for (ev=mRelations.first();ev;ev=mRelations.next()) {
    if (ev->relatedTo() == this) ev->setRelatedTo(0);
  }
  if (relatedTo()) relatedTo()->removeRelation(this);  
}

void Incidence::recreate()
{
  setCreated(QDateTime::currentDateTime());

  setVUID(CalFormat::createUniqueId());

  setRevision(0);

  setLastModified(QDateTime::currentDateTime());
}

void Incidence::setReadOnly(bool readonly)
{
  mReadOnly = readonly;
  setRecurReadOnly(mReadOnly);
  setAlarmReadOnly(mReadOnly);
}

void Incidence::setLastModified(const QDateTime &lm)
{
  // DON'T! emit eventUpdated because we call this from
  // CalObject::updateEvent().
  mLastModified = lm;
}

const QDateTime &Incidence::lastModified() const
{
  return mLastModified;
}

const QDateTime &Incidence::dtStart() const
{
  return mDtStart;
}

void Incidence::setCreated(QDateTime created)
{
  if (mReadOnly) return;
  mCreated = created;
}

QDateTime Incidence::created() const
{
  return mCreated;
}

void Incidence::setVUID(const QString &VUID)
{
  mVUID = VUID;
  emit eventUpdated(this);
}

const QString &Incidence::VUID() const
{
  return mVUID;
}

void Incidence::setRevision(int rev)
{
  if (mReadOnly) return;
  mRevision = rev;
  emit eventUpdated(this);
}

int Incidence::revision() const
{
  return mRevision;
}

void Incidence::setOrganizer(const QString &o)
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  mOrganizer = o;
  if (mOrganizer.left(7).upper() == "MAILTO:")
    mOrganizer = mOrganizer.remove(0,7);
  emit eventUpdated(this);  
}

const QString &Incidence::organizer() const
{
  return mOrganizer;
}

void Incidence::addAttendee(Attendee *a)
{
//  kdDebug() << "Incidence::addAttendee()" << endl;
  if (mReadOnly) return;
//  kdDebug() << "Incidence::addAttendee() weiter" << endl;
  if (a->name().left(7).upper() == "MAILTO:")
    a->setName(a->name().remove(0,7));

  mAttendees.append(a);
  emit eventUpdated(this);
}

void Incidence::removeAttendee(Attendee *a)
{
  if (mReadOnly) return;
  mAttendees.removeRef(a);
  emit eventUpdated(this);
}

void Incidence::removeAttendee(const char *n)
{
  Attendee *a;

  if (mReadOnly) return;
  for (a = mAttendees.first(); a; a = mAttendees.next())
    if (a->getName() == n) {
      mAttendees.remove();
      break;
    }
}
    
void Incidence::clearAttendees()
{
  if (mReadOnly) return;
  mAttendees.clear();
}

Attendee *Incidence::getAttendee(const char *n) const
{
  QListIterator<Attendee> qli(mAttendees);

  qli.toFirst();
  while (qli) {
    if (qli.current()->getName() == n)
      return qli.current();
    ++qli;
  }
  return 0L;
}

void Incidence::setDescription(const QString &description)
{
  if (mReadOnly) return;
  mDescription = description;
  emit eventUpdated(this);
}

/*
void Incidence::setDescription(const char *description)
{
  if (mReadOnly) return;
  Incidence::description = description;
  emit eventUpdated(this);
}
*/

const QString &Incidence::description() const
{
  return mDescription;
}


void Incidence::setSummary(const QString &summary)
{
  if (mReadOnly) return;
  mSummary = summary;
  emit eventUpdated(this);
}

/*
void Incidence::setSummary(const char *summary)
{
  if (mReadOnly) return;
  Incidence::summary = summary;
  emit eventUpdated(this);
}
*/

const QString &Incidence::summary() const
{
  return mSummary;
}

void Incidence::setCategories(const QStringList &categories)
{
  if (mReadOnly) return;
  mCategories = categories;
  emit eventUpdated(this);
}

// TODO: remove setCategories(QString) function
void Incidence::setCategories(const QString &catStr)
{
  if (mReadOnly) return;

  if (catStr.isEmpty()) return;

  mCategories = QStringList::split(",",catStr);

  QStringList::Iterator it;
  for(it = mCategories.begin();it != mCategories.end(); ++it) {
    *it = (*it).stripWhiteSpace();
  }

  emit eventUpdated(this);
}

const QStringList &Incidence::categories() const
{
  return mCategories;
}

QString Incidence::categoriesStr()
{
  return mCategories.join(",");
}

void Incidence::setRelatedToVUID(const QString &relatedToVUID)
{
  if (mReadOnly) return;
  mRelatedToVUID = relatedToVUID;
}

const QString &Incidence::relatedToVUID() const
{
  return mRelatedToVUID;
}

void Incidence::setRelatedTo(Incidence *relatedTo)
{
  if (mReadOnly) return;
  Incidence *oldRelatedTo = mRelatedTo;
  if(oldRelatedTo) {
    oldRelatedTo->removeRelation(this);
  }
  mRelatedTo = relatedTo;
  if (mRelatedTo) mRelatedTo->addRelation(this);
  emit eventUpdated(this);
}

Incidence *Incidence::relatedTo() const
{
  return mRelatedTo;
}

const QList<Incidence> &Incidence::relations() const
{
  return mRelations;
}

void Incidence::addRelation(Incidence *event)
{
  mRelations.append(event);
  emit eventUpdated(this);
}

void Incidence::removeRelation(Incidence *event)
{
  mRelations.removeRef(event);
//  if (event->getRelatedTo() == this) event->setRelatedTo(0);
  emit eventUpdated(this);
}

bool Incidence::recursOn(const QDate &qd) const
{
  if (recursOnPure(qd) && !isException(qd)) return true;
  else return false;
}

void Incidence::setExDates(const QDateList &exDates)
{
  if (mReadOnly) return;
  mExDates.clear();
  mExDates = exDates;

  setRecurExDatesCount(mExDates.count());

  emit eventUpdated(this);
}

void Incidence::setExDates(const char *dates)
{
  if (mReadOnly) return;
  mExDates.clear();
  QString tmpStr = dates;
  int index = 0;
  int index2 = 0;

  while ((index2 = tmpStr.find(',', index)) != -1) {
    QDate *tmpDate = new QDate;
    *tmpDate = strToDate(tmpStr.mid(index, (index2-index)));
    
    mExDates.append(tmpDate);
    index = index2 + 1;
  }
  QDate *tmpDate = new QDate;
  *tmpDate = strToDate(tmpStr.mid(index, (tmpStr.length()-index)));
  mExDates.inSort(tmpDate);

  setRecurExDatesCount(mExDates.count());

  emit eventUpdated(this);
}

void Incidence::addExDate(const QDate &date)
{
  if (mReadOnly) return;
  QDate *addDate = new QDate(date);
  mExDates.inSort(addDate);

  setRecurExDatesCount(mExDates.count());

  emit eventUpdated(this);
}

const QDateList &Incidence::exDates() const
{
  return mExDates;
}

bool Incidence::isException(const QDate &qd) const
{
  QDateList tmpList(FALSE); // we want a shallow copy

  tmpList = mExDates;

  QDate *datePtr;
  for (datePtr = tmpList.first(); datePtr;
       datePtr = tmpList.next()) {
    if (qd == *datePtr) {
      return TRUE;
    }
  }
  return FALSE;
}
