/*
    This file is part of KOrganizer.
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <time.h>
#include <unistd.h>

#include <qdir.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <kstaticdeleter.h>

#include "koprefs.h"

KOPrefs *KOPrefs::mInstance = 0;
static KStaticDeleter<KOPrefs> insd;

KOPrefs::KOPrefs() :
  KOPrefsBase()
{
  mCategoryColors.setAutoDelete(true);

  mDefaultCategoryColor = QColor(151, 235, 121);

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize(mDefaultMonthViewFont.pointSize()-2);

  KPrefs::setCurrentGroup("General");

  addItemPath("Html Export File",mHtmlExportFile,
      QDir::homeDirPath() + "/" + i18n("Default export file", "calendar.html"));

  KPrefs::setCurrentGroup("Fonts");

  addItemFont("MonthView Font",mMonthViewFont, mDefaultMonthViewFont);

  KPrefs::setCurrentGroup("Colors");

  addItemColor("Event Color",mEventColor,mDefaultCategoryColor);
}


KOPrefs::~KOPrefs()
{
  kdDebug(5850) << "KOPrefs::~KOPrefs()" << endl;
  if (mInstance == this)
      mInstance = insd.setObject(0);
}


KOPrefs *KOPrefs::instance()
{
  if (!mInstance) {
      mInstance = insd.setObject(new KOPrefs());
      mInstance->readConfig();
  }

  return mInstance;
}

void KOPrefs::usrSetDefaults()
{
  // Default should be set a bit smarter, respecting username and locale
  // settings for example.

  KEMailSettings settings;
  mName = settings.getSetting(KEMailSettings::RealName);
  mEmail = settings.getSetting(KEMailSettings::RealName);
  fillMailDefaults();

  mMonthViewFont = mDefaultMonthViewFont;

  setTimeZoneIdDefault();

  KPimPrefs::usrSetDefaults();
}

void KOPrefs::fillMailDefaults()
{
  if (mName.isEmpty()) mName = i18n("Anonymous");
  if (mEmail.isEmpty()) mEmail = i18n("nobody@nowhere");
}

void KOPrefs::setTimeZoneIdDefault()
{
  QString zone;

  char zonefilebuf[100];
  int len = readlink("/etc/localtime",zonefilebuf,100);
  if (len > 0 && len < 100) {
    zonefilebuf[len] = '\0';
    zone = zonefilebuf;
    zone = zone.mid(zone.find("zoneinfo/") + 9);
  } else {
    tzset();
    zone = tzname[0];
  }

  kdDebug () << "----- time zone: " << zone << endl;

  mTimeZoneId = zone;
}

void KOPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();

  mCustomCategories << i18n("Appointment") << i18n("Business")
      << i18n("Meeting") << i18n("Phone Call") << i18n("Education")
      << i18n("Holiday") << i18n("Vacation") << i18n("Special Occasion")
      << i18n("Personal") << i18n("Travel") << i18n("Miscellaneous")
      << i18n("Birthday");

  QStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    setCategoryColor(*it,mDefaultCategoryColor);
  }
}


void KOPrefs::usrReadConfig()
{
  config()->setGroup("General");
  mCustomCategories = config()->readListEntry("Custom Categories");
  if (mCustomCategories.isEmpty()) setCategoryDefaults();

  config()->setGroup("Personal Settings");
  mName = config()->readEntry("user_name");
  mEmail = config()->readEntry("user_email");
  fillMailDefaults();

  // old category colors, ignore if they have the old default
  // should be removed a few versions after 3.2...
  config()->setGroup("Category Colors");
  QValueList<QColor> oldCategoryColors;
  QStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    QColor c = config()->readColorEntry(*it, &mDefaultCategoryColor);
    oldCategoryColors.append( (c == QColor(196,196,196)) ?
                              mDefaultCategoryColor : c);
  }

  // new category colors
  config()->setGroup("Category Colors2");
  QValueList<QColor>::Iterator it2;
  for (it = mCustomCategories.begin(), it2 = oldCategoryColors.begin();
       it != mCustomCategories.end(); ++it, ++it2 ) {
    setCategoryColor(*it,config()->readColorEntry(*it, &*it2));
  }

  if (mTimeZoneId.isEmpty()) {
    setTimeZoneIdDefault();
  }

  KPimPrefs::usrReadConfig();
}


void KOPrefs::usrWriteConfig()
{
  config()->setGroup("General");
  config()->writeEntry("Custom Categories",mCustomCategories);

  config()->setGroup("Personal Settings");
  config()->writeEntry("user_name",mName);
  config()->writeEntry("user_email",mEmail);

  config()->setGroup("Category Colors2");
  QDictIterator<QColor> it(mCategoryColors);
  while (it.current()) {
    config()->writeEntry(it.currentKey(),*(it.current()));
    ++it;
  }

  KPimPrefs::usrWriteConfig();
}

void KOPrefs::setCategoryColor(QString cat,const QColor & color)
{
  mCategoryColors.replace(cat,new QColor(color));
}

QColor *KOPrefs::categoryColor(QString cat)
{
  QColor *color = 0;

  if (!cat.isEmpty()) color = mCategoryColors[cat];

  if (color) return color;
  else return &mDefaultCategoryColor;
}

void KOPrefs::setFullName(const QString &name)
{
  mName = name;
}

void KOPrefs::setEmail(const QString &email)
{
  mEmail = email;
}

QString KOPrefs::fullName()
{
  if (mEmailControlCenter) {
    KEMailSettings settings;
    return settings.getSetting(KEMailSettings::RealName);
  } else {
    return mName;
  }
}

QString KOPrefs::email()
{
  if (mEmailControlCenter) {
    KEMailSettings settings;
    return settings.getSetting(KEMailSettings::EmailAddress);
  } else {
    return mEmail;
  }
}
