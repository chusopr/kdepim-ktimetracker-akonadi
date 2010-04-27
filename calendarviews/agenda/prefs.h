/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef PREFS_H
#define PREFS_H


#include "prefs_base.h"
#include <akonadi/collection.h>
#include <kdatetime.h>

#include <QHash>

class QFont;
class QColor;
class QStringList;

namespace EventViews
{

class Prefs : public PrefsBase
{
  public:
    virtual ~Prefs();

    /** Get instance of Prefs. It is made sure that there is only one
    instance. */
    static Prefs *instance();

    /** Set preferences to default values */
    void usrSetDefaults();

    /** Read preferences from config file */
    void usrReadConfig();

    /** Write preferences to config file */
    void usrWriteConfig();

  protected:
    void setTimeZoneDefault();

    /** Fill empty mail fields with default values. */
    void fillMailDefaults();

  private:
    /** Constructor disabled for public. Use instance() to create a Prefs
    object. */
    Prefs();

    static Prefs *mInstance;

  public:
    // preferences data
    void setFullName( const QString & );
    QString fullName();
    void setEmail( const QString & );
    QString email();
    /// Returns all email addresses for the user.
    QStringList allEmails();
    /// Returns all email addresses together with the full username for the user.
    QStringList fullEmails();
    /// Return true if the given email belongs to the user
    bool thatIsMe( const QString &email );

    void setCategoryColor( const QString &cat, const QColor &color );
    QColor categoryColor( const QString &cat ) const;
    bool hasCategoryColor( const QString &cat ) const;

    void setResourceColor ( const QString &, const QColor & );
    QColor resourceColor( const QString & );

    void setTimeSpec( const KDateTime::Spec &spec );
    KDateTime::Spec timeSpec();

    QString mHtmlExportFile;

    // Groupware passwords
    QString mPublishPassword;
    QString mRetrievePassword;

    QStringList timeScaleTimezones();
    void setTimeScaleTimezones( const QStringList &list );

    QString defaultCalendar() const;
    void setDefaultCollection( const Akonadi::Collection& );
    Akonadi::Collection defaultCollection() const;

  private:
    QHash<QString,QColor> mCategoryColors;
    QColor mDefaultCategoryColor;

    QHash<QString,QColor> mResourceColors;
    QColor mDefaultResourceColor;

    QFont mDefaultMonthViewFont;
    QFont mDefaultAgendaTimeLabelsFont;

    KDateTime::Spec mTimeSpec;
    QStringList mTimeScaleTimeZones;

    QString mDefaultCalendar;
    Akonadi::Collection mDefaultCollection;

  public: // Do not use - except in PrefsDialogMain
    QString mName;
    QString mEmail;
};

} // namespace EventViews

#endif
