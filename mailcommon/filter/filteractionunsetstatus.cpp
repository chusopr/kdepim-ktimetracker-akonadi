/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filteractionunsetstatus.h"

#include <KDE/KLocale>
using namespace MailCommon;

FilterAction* FilterActionUnsetStatus::newAction()
{
  return new FilterActionUnsetStatus;
}

FilterActionUnsetStatus::FilterActionUnsetStatus( QObject *parent )
  : FilterActionStatus( "unset status", i18nc( "action: to unset the status", "Unset Status" ), parent )
{
}

FilterAction::ReturnCode FilterActionUnsetStatus::process( ItemContext &context ) const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index < 1 )
    return ErrorButGoOn;
  
  Akonadi::MessageStatus status;
  status.setStatusFromFlags( context.item().flags() );


  Akonadi::MessageStatus newStatus = FilterActionStatus::stati[ index - 1 ];
  //Laurent unread status doesn't exist. And crash if we use "const Akonadi::Item::Flag flag = *(flags.begin());" etc. => if we unset "unread" we set read
  if ( newStatus == Akonadi::MessageStatus::statusUnread() ) {
    Akonadi::MessageStatus oldStatus = status;
    newStatus.setRead( true );
    if( oldStatus != newStatus ) {
      context.item().setFlags( newStatus.statusFlags() );
      context.setNeedsFlagStore();
    }
    return GoOn;
  }

  QSet<QByteArray> flags = newStatus.statusFlags();
  const Akonadi::Item::Flag flag = *(flags.begin());
  if ( context.item().hasFlag( flag ) ) {
      context.item().clearFlag( flag );
      context.setNeedsFlagStore();
  }
  return GoOn;
}

SearchRule::RequiredPart FilterActionUnsetStatus::requiredPart() const
{
    return SearchRule::Envelope;
}

#include "filteractionunsetstatus.moc"
