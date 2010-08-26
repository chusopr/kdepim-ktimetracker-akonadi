/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com

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

#include "prefs.h"

#include "prefs_base.h"

#include <akonadi/collection.h>

#include <kmime/kmime_header_parsing.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <kpimutils/email.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <kdatetime.h>
#include <kstringhandler.h>
#include <ksystemtimezone.h>

#include <QDir>
#include <QHash>
#include <QString>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QStringList>

#include <time.h>
#include <unistd.h>

using namespace KPIMIdentities;

using namespace EventViews;

class BaseConfig : public PrefsBase
{
  public:
    BaseConfig();

    void setResourceColor( const QString &resource, const QColor &color );

    void setTimeScaleTimezones( const QStringList &timeZones );
    QStringList timeScaleTimezones() const;

  public:
    QHash<QString,QColor> mCategoryColors;
    QColor mDefaultCategoryColor;

    QHash<QString,QColor> mResourceColors;
    QColor mDefaultResourceColor;

    QFont mDefaultMonthViewFont;
    QFont mDefaultAgendaTimeLabelsFont;

    KDateTime::Spec mTimeSpec;
    QStringList mTimeScaleTimeZones;

  protected:
    void usrSetDefaults();
    void usrReadConfig();
    void usrWriteConfig();

    void fillMailDefaults();
    void setTimeZoneDefault();
};

BaseConfig::BaseConfig() : PrefsBase()
{
  mDefaultCategoryColor = QColor( 151, 235, 121 );
  mDefaultResourceColor = QColor(); //Default is a color invalid

  mDefaultAgendaTimeLabelsFont = KGlobalSettings::generalFont();
  // make a large default time bar font, at least 16 points.
  mDefaultAgendaTimeLabelsFont.setPointSize(
    qMax( mDefaultAgendaTimeLabelsFont.pointSize() + 4, 16 ) );

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize(
    qMax( mDefaultMonthViewFont.pointSize() - 2, 6 ) );

  agendaTimeLabelsFontItem()->setDefaultValue( mDefaultAgendaTimeLabelsFont );
  monthViewFontItem()->setDefaultValue( mDefaultMonthViewFont );
}

void BaseConfig::setResourceColor( const QString &resource, const QColor &color )
{
  mResourceColors.insert( resource, color );
}

void BaseConfig::setTimeScaleTimezones( const QStringList &list )
{
  mTimeScaleTimeZones = list;
}

QStringList BaseConfig::timeScaleTimezones() const
{
  return mTimeScaleTimeZones;
}

void BaseConfig::usrSetDefaults()
{
  // Default should be set a bit smarter, respecting username and locale
  // settings for example.

  KEMailSettings settings;
  QString tmp = settings.getSetting( KEMailSettings::RealName );
  if ( !tmp.isEmpty() ) {
    setUserName( tmp );
  }
  tmp = settings.getSetting( KEMailSettings::EmailAddress );
  if ( !tmp.isEmpty() ) {
    setUserEmail( tmp );
  }

  fillMailDefaults();

  setAgendaTimeLabelsFont( mDefaultAgendaTimeLabelsFont );
  setMonthViewFont( mDefaultMonthViewFont );

  setTimeZoneDefault();

  PrefsBase::usrSetDefaults();
}

void BaseConfig::usrReadConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  // Note that the [Category Colors] group was removed after 3.2 due to
  // an algorithm change. That's why we now use [Category Colors2]

  // Category colors
  KConfigGroup colorsConfig( config(), "Category Colors2" );
/*  CategoryConfig cc( this );
  const QStringList cats = cc.customCategories();
  Q_FOREACH( const QString& i, cats ) {
    QColor c = colorsConfig.readEntry( i, mDefaultCategoryColor );
    if ( c != mDefaultCategoryColor ) {
      setCategoryColor( i, c );
    }
    }*/

  // Resource colors
  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  const QStringList colorKeyList = rColorsConfig.keyList();

  QStringList::ConstIterator it3;
  for ( it3 = colorKeyList.begin(); it3 != colorKeyList.end(); ++it3 ) {
    QColor color = rColorsConfig.readEntry( *it3, mDefaultResourceColor );
    //kDebug() << "key:" << (*it3) << "value:" << color;
    setResourceColor( *it3, color );
  }

  if ( !mTimeSpec.isValid() ) {
    setTimeZoneDefault();
  }

#if 0
  config()->setGroup( "FreeBusy" );
  if ( mRememberRetrievePw ) {
    mRetrievePassword =
      KStringHandler::obscure( config()->readEntry( "Retrieve Server Password" ) );
  }
#endif
  KConfigGroup defaultCalendarConfig( config(), "Calendar" );

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  setTimeScaleTimezones( timeScaleConfig.readEntry( "Timescale Timezones", QStringList() ) );

  KConfigSkeleton::usrReadConfig();
  fillMailDefaults();
}

void BaseConfig::usrWriteConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  KConfigGroup colorsConfig( config(), "Category Colors2" );
  QHash<QString, QColor>::const_iterator i = mCategoryColors.constBegin();
  while ( i != mCategoryColors.constEnd() ) {
    colorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  i = mResourceColors.constBegin();
  while ( i != mResourceColors.constEnd() ) {
    rColorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

  if ( !mFreeBusyPublishSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyPublishPasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }
  if ( !mFreeBusyRetrieveSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyRetrievePasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }

#if 0
  if ( mRememberRetrievePw ) {
    config()->writeEntry( "Retrieve Server Password",
                          KStringHandler::obscure( mRetrievePassword ) );
  } else {
    config()->deleteEntry( "Retrieve Server Password" );
  }
#endif

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  timeScaleConfig.writeEntry( "Timescale Timezones", timeScaleTimezones() );


  KConfigSkeleton::usrWriteConfig();
}

void BaseConfig::fillMailDefaults()
{
  userEmailItem()->swapDefault();
  QString defEmail = userEmailItem()->value();
  userEmailItem()->swapDefault();

  if ( userEmail() == defEmail ) {
    // No korg settings - but maybe there's a kcontrol[/kmail] setting available
    KEMailSettings settings;
    if ( !settings.getSetting( KEMailSettings::EmailAddress ).isEmpty() ) {
      mEmailControlCenter = true;
    }
  }
}

void BaseConfig::setTimeZoneDefault()
{
  KTimeZone zone = KSystemTimeZones::local();
  if ( !zone.isValid() ) {
    kError() << "KSystemTimeZones::local() return 0";
    return;
  }

  kDebug () << "----- time zone:" << zone.name();

  mTimeSpec = zone;
}

class Prefs::Private
{
  public:
    Private( Prefs *parent ) : mAppConfig( 0 ), q( parent ) {}
    Private( Prefs *parent, KCoreConfigSkeleton *appConfig )
      : mAppConfig( appConfig ), q( parent ) {}

    void fillMailDefaults();
    void setTimeZoneDefault();

    KConfigSkeletonItem *appConfigItem( const KConfigSkeletonItem *baseConfigItem ) const;

    void setBool( KCoreConfigSkeleton::ItemBool *baseConfigItem, bool value );
    bool getBool( const KCoreConfigSkeleton::ItemBool *baseConfigItem ) const;

    void setInt( KCoreConfigSkeleton::ItemInt *baseConfigItem, int value );
    int getInt( const KCoreConfigSkeleton::ItemInt *baseConfigItem ) const;

    void setString( KCoreConfigSkeleton::ItemString *baseConfigItem, const QString &value );
    QString getString( const KCoreConfigSkeleton::ItemString *baseConfigItem ) const;

    void setDateTime( KCoreConfigSkeleton::ItemDateTime *baseConfigItem, const QDateTime &value );
    QDateTime getDateTime( const KCoreConfigSkeleton::ItemDateTime *baseConfigItem ) const;

    void setStringList( KCoreConfigSkeleton::ItemStringList *baseConfigItem, const QStringList &value );
    QStringList getStringList( const KCoreConfigSkeleton::ItemStringList *baseConfigItem ) const;

    void setColor( KConfigSkeleton::ItemColor *baseConfigItem, const QColor &value );
    QColor getColor( const KConfigSkeleton::ItemColor *baseConfigItem ) const;

    void setFont( KConfigSkeleton::ItemFont *baseConfigItem, const QFont &value );
    QFont getFont( const KConfigSkeleton::ItemFont *baseConfigItem ) const;

  public:
    BaseConfig mBaseConfig;
    KCoreConfigSkeleton *mAppConfig;

  private:
    Prefs *q;
};

KConfigSkeletonItem *Prefs::Private::appConfigItem( const KConfigSkeletonItem *baseConfigItem ) const
{
  Q_ASSERT( baseConfigItem );

  if ( mAppConfig ) {
    return mAppConfig->findItem( baseConfigItem->name() );
  }

  return 0;
}

void Prefs::Private::setBool( KCoreConfigSkeleton::ItemBool *baseConfigItem, bool value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemBool *item = dynamic_cast<KCoreConfigSkeleton::ItemBool*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Bool";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

bool Prefs::Private::getBool( const KCoreConfigSkeleton::ItemBool *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemBool *item = dynamic_cast<KCoreConfigSkeleton::ItemBool*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Bool";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setInt( KCoreConfigSkeleton::ItemInt *baseConfigItem, int value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemInt *item = dynamic_cast<KCoreConfigSkeleton::ItemInt*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Int";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

int Prefs::Private::getInt( const KCoreConfigSkeleton::ItemInt *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemInt *item = dynamic_cast<KCoreConfigSkeleton::ItemInt*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Int";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setString( KCoreConfigSkeleton::ItemString *baseConfigItem, const QString &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemString *item = dynamic_cast<KCoreConfigSkeleton::ItemString*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type String";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QString Prefs::Private::getString( const KCoreConfigSkeleton::ItemString *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemString *item = dynamic_cast<KCoreConfigSkeleton::ItemString*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type String";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setDateTime( KCoreConfigSkeleton::ItemDateTime *baseConfigItem, const QDateTime &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemDateTime *item = dynamic_cast<KCoreConfigSkeleton::ItemDateTime*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type DateTime";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QDateTime Prefs::Private::getDateTime( const KCoreConfigSkeleton::ItemDateTime *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemDateTime *item = dynamic_cast<KCoreConfigSkeleton::ItemDateTime*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type DateTime";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setStringList( KCoreConfigSkeleton::ItemStringList *baseConfigItem, const QStringList &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemStringList *item = dynamic_cast<KCoreConfigSkeleton::ItemStringList*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type StringList";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QStringList Prefs::Private::getStringList( const KCoreConfigSkeleton::ItemStringList *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemStringList *item = dynamic_cast<KCoreConfigSkeleton::ItemStringList*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type StringList";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setColor( KConfigSkeleton::ItemColor *baseConfigItem, const QColor &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemColor *item = dynamic_cast<KConfigSkeleton::ItemColor*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Color";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QColor Prefs::Private::getColor( const KConfigSkeleton::ItemColor *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemColor *item = dynamic_cast<KConfigSkeleton::ItemColor*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Color";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setFont( KConfigSkeleton::ItemFont *baseConfigItem, const QFont &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemFont *item = dynamic_cast<KConfigSkeleton::ItemFont*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Font";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QFont Prefs::Private::getFont( const KConfigSkeleton::ItemFont *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemFont *item = dynamic_cast<KConfigSkeleton::ItemFont*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Font";
  }
  return baseConfigItem->value();
}

Prefs::Prefs() : d( new Private( this ) )
{
}

Prefs::Prefs( KCoreConfigSkeleton *appConfig ) : d( new Private( this, appConfig ) )
{
}

Prefs::~Prefs()
{
  kDebug();
  delete d;
}

void Prefs::readConfig()
{
  d->mBaseConfig.readConfig();
  if ( d->mAppConfig ) {
    d->mAppConfig->readConfig();
  }
}

void Prefs::writeConfig()
{
  d->mBaseConfig.writeConfig();
  if ( d->mAppConfig ) {
    d->mAppConfig->writeConfig();
  }
}

void Prefs::setMarcusBainsShowSeconds( bool showSeconds )
{
  d->setBool( d->mBaseConfig.marcusBainsShowSecondsItem(), showSeconds );
}

bool Prefs::marcusBainsShowSeconds() const
{
  return d->getBool( d->mBaseConfig.marcusBainsShowSecondsItem() );
}

void Prefs::setAgendaMarcusBainsLineLineColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaMarcusBainsLineLineColorItem(), color );
}

QColor Prefs::agendaMarcusBainsLineLineColor() const
{
  return d->getColor( d->mBaseConfig.agendaMarcusBainsLineLineColorItem() );
}

void Prefs::setMarcusBainsEnabled( bool enabled )
{
  d->setBool( d->mBaseConfig.marcusBainsEnabledItem(), enabled );
}

bool Prefs::marcusBainsEnabled() const
{
  return d->getBool( d->mBaseConfig.marcusBainsEnabledItem() );
}

void Prefs::setAgendaMarcusBainsLineFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.agendaMarcusBainsLineFontItem(), font );
}

QFont Prefs::agendaMarcusBainsLineFont() const
{
  return d->getFont( d->mBaseConfig.agendaMarcusBainsLineFontItem() );
}

void Prefs::setHourSize( int size )
{
  d->setInt( d->mBaseConfig.hourSizeItem(), size );
}

int Prefs::hourSize() const
{
  return d->getInt( d->mBaseConfig.hourSizeItem() );
}

void Prefs::setDayBegins( const QDateTime &dateTime )
{
  d->setDateTime( d->mBaseConfig.dayBeginsItem(), dateTime );
}

QDateTime Prefs::dayBegins() const
{
  return d->getDateTime( d->mBaseConfig.dayBeginsItem() );
}

void Prefs::setWorkingHoursStart( const QDateTime &dateTime )
{
  d->setDateTime( d->mBaseConfig.workingHoursStartItem(), dateTime );
}

QDateTime Prefs::workingHoursStart() const
{
  return d->getDateTime( d->mBaseConfig.workingHoursStartItem() );
}

void Prefs::setWorkingHoursEnd( const QDateTime &dateTime )
{
  d->setDateTime( d->mBaseConfig.workingHoursEndItem(), dateTime );
}

QDateTime Prefs::workingHoursEnd() const
{
  return d->getDateTime( d->mBaseConfig.workingHoursEndItem() );
}

void Prefs::setSelectionStartsEditor( bool startEditor )
{
  d->setBool( d->mBaseConfig.selectionStartsEditorItem(), startEditor );
}

bool Prefs::selectionStartsEditor() const
{
  return d->getBool( d->mBaseConfig.selectionStartsEditorItem() );
}

void Prefs::setAgendaGridWorkHoursBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaGridWorkHoursBackgroundColorItem(), color );
}

QColor Prefs::agendaGridWorkHoursBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaGridWorkHoursBackgroundColorItem() );
}

void Prefs::setAgendaGridHighlightColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaGridHighlightColorItem(), color );
}

QColor Prefs::agendaGridHighlightColor() const
{
  return d->getColor( d->mBaseConfig.agendaGridHighlightColorItem() );
}

void Prefs::setAgendaGridBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaGridBackgroundColorItem(), color );
}

QColor Prefs::agendaGridBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaGridBackgroundColorItem() );
}

void Prefs::setEnableAgendaItemIcons( bool enable )
{
  d->setBool( d->mBaseConfig.enableAgendaItemIconsItem(), enable );
}

bool Prefs::enableAgendaItemIcons() const
{
  return d->getBool( d->mBaseConfig.enableAgendaItemIconsItem() );
}

void Prefs::setTodosUseCategoryColors( bool useColors )
{
  d->setBool( d->mBaseConfig.todosUseCategoryColorsItem(), useColors );
}

bool Prefs::todosUseCategoryColors() const
{
  return d->getBool( d->mBaseConfig.todosUseCategoryColorsItem() );
}

void Prefs::setAgendaCalendarItemsToDosOverdueBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaCalendarItemsToDosOverdueBackgroundColorItem(), color );
}

QColor Prefs::agendaCalendarItemsToDosOverdueBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaCalendarItemsToDosOverdueBackgroundColorItem() );
}

void Prefs::setAgendaCalendarItemsToDosDueTodayBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaCalendarItemsToDosDueTodayBackgroundColorItem(), color );
}

QColor Prefs::agendaCalendarItemsToDosDueTodayBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaCalendarItemsToDosDueTodayBackgroundColorItem() );
}

void Prefs::setUnsetCategoryColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.unsetCategoryColorItem(), color );
}

QColor Prefs::unsetCategoryColor() const
{
  return d->getColor( d->mBaseConfig.unsetCategoryColorItem() );
}

void Prefs::setAgendaViewColors( int colors )
{
  d->setInt( d->mBaseConfig.agendaViewColorsItem(), colors );
}

int Prefs::agendaViewColors() const
{
  return d->getInt( d->mBaseConfig.agendaViewColorsItem() );
}

void Prefs::setAgendaViewFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.agendaViewFontItem(), font );
}

QFont Prefs::agendaViewFont() const
{
  return d->getFont( d->mBaseConfig.agendaViewFontItem() );
}

void Prefs::setMonthViewFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.monthViewFontItem(), font );
}

QFont Prefs::monthViewFont() const
{
  return d->getFont( d->mBaseConfig.monthViewFontItem() );
}

void Prefs::setEnableToolTips( bool enable )
{
  d->setBool( d->mBaseConfig.enableToolTipsItem(), enable );
}

bool Prefs::enableToolTips() const
{
  return d->getBool( d->mBaseConfig.enableToolTipsItem() );
}

void Prefs::setDefaultDuration( const QDateTime &dateTime )
{
  d->setDateTime( d->mBaseConfig.defaultDurationItem(), dateTime );
}

QDateTime Prefs::defaultDuration() const
{
  return d->getDateTime( d->mBaseConfig.defaultDurationItem() );
}

void Prefs::setShowTodosAgendaView( bool show )
{
  d->setBool( d->mBaseConfig.showTodosAgendaViewItem(), show );
}

bool Prefs::showTodosAgendaView() const
{
  return d->getBool( d->mBaseConfig.showTodosAgendaViewItem() );
}

void Prefs::setAgendaTimeLabelsFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.agendaTimeLabelsFontItem(), font );
}

QFont Prefs::agendaTimeLabelsFont() const
{
  return d->getFont( d->mBaseConfig.agendaTimeLabelsFontItem() );
}

void Prefs::setWorkWeekMask( int mask )
{
  d->setInt( d->mBaseConfig.workWeekMaskItem(), mask );
}

int Prefs::workWeekMask() const
{
  return d->getInt( d->mBaseConfig.workWeekMaskItem() );
}

void Prefs::setExcludeHolidays( bool exclude )
{
  d->setBool( d->mBaseConfig.excludeHolidaysItem(), exclude );
}

bool Prefs::excludeHolidays() const
{
  return d->getBool( d->mBaseConfig.excludeHolidaysItem() );
}

KDateTime::Spec Prefs::timeSpec() const
{
  return KSystemTimeZones::local();
}

void Prefs::setTimeSpec( const KDateTime::Spec &spec )
{
  d->mBaseConfig.mTimeSpec = spec;
}

void Prefs::setCategoryColor( const QString &cat, const QColor &color )
{
  d->mBaseConfig.mCategoryColors.insert( cat, color );
}

QColor Prefs::categoryColor( const QString &cat ) const
{
  QColor color;

  if ( !cat.isEmpty() ) {
    color = d->mBaseConfig.mCategoryColors.value( cat );
  }

  if ( color.isValid() ) {
    return color;
  } else {
    return d->mBaseConfig.mDefaultCategoryColor;
  }
}

bool Prefs::hasCategoryColor( const QString &cat ) const
{
    return d->mBaseConfig.mCategoryColors[ cat ].isValid();
}

void Prefs::setResourceColor ( const QString &cal, const QColor &color )
{
  d->mBaseConfig.setResourceColor( cal, color );
}

QColor Prefs::resourceColor( const QString &cal )
{
  QColor color;
  if ( !cal.isEmpty() ) {
    if ( d->mBaseConfig.mResourceColors.contains( cal ) ) {
      color = d->mBaseConfig.mResourceColors.value( cal );
      if ( !color.isValid() )
        return color;
    }
  } else {
    return d->mBaseConfig.mDefaultResourceColor;
  }

  // assign default color if enabled
  if ( !cal.isEmpty() && !color.isValid() && d->getBool( d->mBaseConfig.assignDefaultResourceColorsItem() ) ) {
    QColor defColor( 0x37, 0x7A, 0xBC );
    const int seed = d->getInt( d->mBaseConfig.defaultResourceColorSeedItem() );
    const QStringList colors = d->getStringList( d->mBaseConfig.defaultResourceColorsItem() );
    if ( seed > 0 && seed - 1 < (int)colors.size() ) {
        defColor = QColor( colors[seed-1] );
    } else {
        int h, s, v;
        defColor.getHsv( &h, &s, &v );
        h = ( seed % 12 ) * 30;
        s -= s * static_cast<int>( ( ( seed / 12 ) % 2 ) * 0.5 );
        defColor.setHsv( h, s, v );
    }
    d->setInt( d->mBaseConfig.defaultResourceColorSeedItem(), ( seed + 1 ));
    d->mBaseConfig.setResourceColor( cal, defColor );
    color = d->mBaseConfig.mResourceColors[cal];
  }

  if ( color.isValid() ) {
    return color;
  } else {
    return d->mBaseConfig.mDefaultResourceColor;
  }
}

QStringList Prefs::timeScaleTimezones() const
{
  return d->mBaseConfig.timeScaleTimezones();
}

void Prefs::setTimeScaleTimezones( const QStringList &list )
{
  d->mBaseConfig.setTimeScaleTimezones( list );
}

KConfigSkeleton::ItemFont* Prefs::fontItem( const QString &name ) const
{
  KConfigSkeletonItem *item = d->mAppConfig ? d->mAppConfig->findItem( name ) : 0;

  if ( !item ) {
    item = d->mBaseConfig.findItem( name );
  }

  return dynamic_cast<KConfigSkeleton::ItemFont *>( item );
}

QStringList Prefs::selectedPlugins() const
{
  return d->mBaseConfig.mSelectedPlugins;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
