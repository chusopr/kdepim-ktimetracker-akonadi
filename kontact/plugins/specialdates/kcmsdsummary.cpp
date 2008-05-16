/*
  This file is part of Kontact.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004-2006 Allen Winter <winter@kde.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kcmsdsummary.h"

#include <kaboutdata.h>
#include <kacceleratormanager.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdemacros.h>
#include <klocale.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

extern "C"
{
  KDE_EXPORT KCModule *create_sdsummary( QWidget *parent, const char * )
  {
    KComponentData inst( "kcmsdsummary" );
    return new KCMSDSummary( inst, parent );
  }
}

KCMSDSummary::KCMSDSummary( const KComponentData &inst, QWidget *parent )
  : KCModule( inst, parent )
{
  setupUi( this );

  mDaysButtonGroup = new QButtonGroup( this );
  mDaysButtonGroup->addButton( mDateTodayButton, 0 );
  mDaysButtonGroup->addButton( mDateMonthButton, 1 );
  mDaysButtonGroup->addButton( mDateRangeButton, 2 );
  mShowFromCalButtonGroup = new QButtonGroup( this );
  mShowFromCalButtonGroup->setExclusive( false );
  mShowFromCalButtonGroup->addButton( mShowBirthdaysFromCalBox );
  mShowFromCalButtonGroup->addButton( mShowAnniversariesFromCalBox );
  mShowFromCalButtonGroup->addButton( mShowHolidaysFromCalBox );
  mShowFromCalButtonGroup->addButton( mShowSpecialsFromCalBox );
  mShowFromKABButtonGroup = new QButtonGroup( this );
  mShowFromKABButtonGroup->setExclusive( false );
  mShowFromKABButtonGroup->addButton( mShowBirthdaysFromKABBox );
  mShowFromKABButtonGroup->addButton( mShowAnniversariesFromKABBox );

  customDaysChanged( 7 );

  connect( mDaysButtonGroup,
           SIGNAL(buttonClicked(int)), SLOT(modified()) );
  connect( mDaysButtonGroup,
           SIGNAL(buttonClicked(int)), SLOT(buttonClicked(int)) );
  connect( mShowFromCalButtonGroup,
           SIGNAL(buttonClicked(int)), SLOT(modified()) );
  connect( mShowFromKABButtonGroup,
           SIGNAL(buttonClicked(int)), SLOT(modified()) );
  connect( mCustomDays,
           SIGNAL(valueChanged(int)), SLOT(modified()) );
  connect( mCustomDays,
           SIGNAL(valueChanged(int)), SLOT(customDaysChanged(int)) );

  KAcceleratorManager::manage( this );

  load();
}

void KCMSDSummary::modified()
{
  emit changed( true );
}

void KCMSDSummary::buttonClicked( int id )
{
  mCustomDays->setEnabled( id == 2 );
}

void KCMSDSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18np( " day", " days", value ) );
}

void KCMSDSummary::load()
{
  KConfig config( "kcmsdsummaryrc" );

  KConfigGroup group = config.group( "Days" );
  int days = group.readEntry( "DaysToShow", 7 );
  if ( days == 1 ) {
    mDateTodayButton->setChecked( true );
  } else if ( days == 31 ) {
    mDateMonthButton->setChecked( true );
  } else {
    mDateRangeButton->setChecked( true );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  group = config.group( "Show" );

  mShowBirthdaysFromKABBox->setChecked( group.readEntry( "BirthdaysFromContacts", true ) );
  mShowBirthdaysFromCalBox->setChecked( group.readEntry( "BirthdaysFromCalendar", true ) );

  mShowAnniversariesFromKABBox->setChecked( group.readEntry( "AnniversariesFromContacts", true ) );
  mShowAnniversariesFromCalBox->setChecked( group.readEntry( "AnniversariesFromCalendar", true ) );

  mShowHolidaysFromCalBox->setChecked( group.readEntry( "HolidaysFromCalendar", true ) );

  mShowSpecialsFromCalBox->setChecked( group.readEntry( "SpecialsFromCalendar", true ) );

  emit changed( false );
}

void KCMSDSummary::save()
{
  KConfig config( "kcmsdsummaryrc" );

  KConfigGroup group = config.group( "Days" );

  int days;
  switch ( mDaysButtonGroup->checkedId() ) {
  case 0:
    days = 1;
    break;
  case 1:
    days = 31;
    break;
  case 2:
  default:
    days = mCustomDays->value();
    break;
  }

  group.writeEntry( "DaysToShow", days );

  group = config.group( "Show" );

  group.writeEntry( "BirthdaysFromContacts", mShowBirthdaysFromKABBox->isChecked() );
  group.writeEntry( "BirthdaysFromCalendar", mShowBirthdaysFromCalBox->isChecked() );

  group.writeEntry( "AnniversariesFromContacts", mShowAnniversariesFromKABBox->isChecked() );
  group.writeEntry( "AnniversariesFromCalendar", mShowAnniversariesFromCalBox->isChecked() );

  group.writeEntry( "HolidaysFromCalendar", mShowHolidaysFromCalBox->isChecked() );

  group.writeEntry( "SpecialsFromCalendar", mShowSpecialsFromCalBox->isChecked() );

  group.sync();
  emit changed( false );
}

void KCMSDSummary::defaults()
{
  mDateRangeButton->setChecked( true );
  mCustomDays->setValue( 7 );
  mCustomDays->setEnabled( true );

  mShowBirthdaysFromKABBox->setChecked( true );
  mShowBirthdaysFromCalBox->setChecked( true );
  mShowAnniversariesFromKABBox->setChecked( true );
  mShowAnniversariesFromCalBox->setChecked( true );
  mShowHolidaysFromCalBox->setChecked( true );
  mShowSpecialsFromCalBox->setChecked( true );

  emit changed( true );
}

const KAboutData *KCMSDSummary::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmsdsummary" ), 0,
    ki18n( "Upcoming Special Dates Configuration Dialog" ),
    0, KLocalizedString(), KAboutData::License_GPL,
    ki18n( "(c) 2004-2006 Allen Winter" ) );

  about->addAuthor( ki18n( "Allen Winter" ), KLocalizedString(), "winter@kde.org" );
  about->addAuthor( ki18n( "Tobias Koenig" ), KLocalizedString(), "tokoe@kde.org" );

  return about;
}

#include "kcmsdsummary.moc"
