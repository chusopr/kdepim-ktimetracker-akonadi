// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KOPREFS_H
#define _KOPREFS_H

#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>

class KConfig;

class KOPrefs
{
  public:
    ~KOPrefs();
  
    /** Get instance of KOPrefs. It is made sure that there is only one
    instance. */
    static KOPrefs *instance();
  
    /** Set preferences to default values */
    void setDefaults();
  
    /** Read preferences from config file */
    void readConfig();

    /** Write preferences to config file */
    void writeConfig();

  private:
    /** Constructor disabled for public. Use instance() to create a KOPrefs
    object. */
    KOPrefs();

    KConfig *mConfig;  // pointer to KConfig object

    static KOPrefs *mInstance;

  public:
    // preferences data
    QString mName;
    QString mEmail;
    QString mAdditional;
    QString mHoliday;
    bool    mAutoSave;
    bool    mConfirm;
  
    int     mTimeFormat;
    int     mDateFormat;
    QString mTimeZone;
    int     mStartTime;
    int     mAlarmTime;
    bool    mWeekstart;
    int     mDaylightSavings;

    QFont   mTimeBarFont;

    QColor  mHolidayColor;
    QColor  mHighlightColor;

    int     mDayBegins;
    int     mHourSize;
    bool    mDailyRecur;

    QString mPrinter;
    int     mPaperSize;
    int     mPaperOrientation;
    QString mPrintPreview;

    QStringList mCustomCategories;
};

#endif
