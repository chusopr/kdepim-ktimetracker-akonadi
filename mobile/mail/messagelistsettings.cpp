/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "messagelistsettings.h"

#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>

MessageListSettings::MessageListSettings()
  : mSortingOption( SortByDateTimeMostRecent ),
    mGroupingOption( GroupByDate ),
    mUseThreading( true ),
    mSaveForCollection( false )
{
}

MessageListSettings::~MessageListSettings()
{
}

void MessageListSettings::setSortingOption( SortingOption option )
{
  mSortingOption = option;
}

MessageListSettings::SortingOption MessageListSettings::sortingOption() const
{
  return mSortingOption;
}

void MessageListSettings::setGroupingOption( GroupingOption option )
{
  mGroupingOption = option;
}

MessageListSettings::GroupingOption MessageListSettings::groupingOption() const
{
  return mGroupingOption;
}

void MessageListSettings::setUseThreading( bool threading )
{
  mUseThreading = threading;
}

bool MessageListSettings::useThreading() const
{
  return mUseThreading;
}

void MessageListSettings::setSaveForCollection( bool save )
{
  mSaveForCollection = save;
}

bool MessageListSettings::saveForCollection() const
{
  return mSaveForCollection;
}

MessageListSettings MessageListSettings::fromConfig( qint64 collectionId )
{
  const QString groupName = QString::fromLatin1( "Folder-%1" ).arg( collectionId );

  MessageListSettings settings;
  if ( KGlobal::config()->hasGroup( groupName ) )
    settings.mSaveForCollection = true;

  const KConfigGroup group( KGlobal::config(), QString::fromLatin1( "Folder-%1" ).arg( collectionId ) );

  settings.mSortingOption = static_cast<SortingOption>( group.readEntry<int>( "SortingOption", SortByDateTimeMostRecent ) );
  settings.mGroupingOption = static_cast<GroupingOption>( group.readEntry<int>( "GroupingOption", GroupByDate ) );
  settings.mUseThreading = group.readEntry<bool>( "UseThreading", true );

  return settings;
}

void MessageListSettings::toConfig( qint64 collectionId, const MessageListSettings &settings )
{
  const QString groupName = QString::fromLatin1( "Folder-%1" ).arg( collectionId );

  if ( !settings.saveForCollection() ) {
    KGlobal::config()->deleteGroup( groupName );
  } else {
    KConfigGroup group( KGlobal::config(), groupName );

    group.writeEntry( "SortingOption", static_cast<int>( settings.mSortingOption ) );
    group.writeEntry( "GroupingOption", static_cast<int>( settings.mGroupingOption ) );
    group.writeEntry( "UseThreading", settings.mUseThreading );
  }

  KGlobal::config()->sync();
}
