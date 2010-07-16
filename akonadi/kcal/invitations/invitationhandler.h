/*
    Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INVITATIONSENDER_H
#define INVITATIONSENDER_H

class QWidget;

#include <KCal/ICalFormat>

/// TODO Come up with a slightly nicer api?!

namespace Akonadi {

class Calendar;

/**
  This class handles sending of invitations to attendees when Incidences (e.g.
  events or todos) are created/modified/deleted.

  There are two scenarios:
  o "we" are the organizer, where "we" means any of the identities or mail
    addresses known to Kontact/PIM. If there are attendees, we need to mail
    them all, even if one or more of them are also "us". Otherwise there
    would be no way to invite a resource or our boss, other identities we
    also manage.
  o "we: are not the organizer, which means we changed the completion status
    of a todo, or we changed our attendee status from, say, tentative to
    accepted. In both cases we only mail the organizer. All other changes
    bring us out of sync with the organizer, so we won't mail, if the user
    insists on applying them.

  NOTE: Currently only events and todos are support, meaning Incidence::type()
        should either return "Event" or "Todo"
 */
class InvitationHandler : public QObject
{
    Q_OBJECT
public:
    InvitationHandler( Akonadi::Calendar *cal );
    ~InvitationHandler();

    enum SendStatus {
      Canceled,        ///> Sending was canceled by the user, meaning there are local changes of which other attendees are not aware.
      FailKeepUpdate,  ///> Sending failed, the changes to the incidence must be kept.
      FailAbortUpdate, ///> Sending failed, the changes to the incidence must be undone.
      NoSendingNeeded, ///> In some cases it is not needed to send an invitation (e.g. when we are the only attendee)
      Success          ///> The invitation was sent to all attendees.
    };

    bool receiveInvitation( const QString& receiver,
                            const QString& iCal,
                            const QString& type );

    /**
      Before an invitation is sent the user is asked for confirmation by means of
      an dialog.
      @param parent The parent widget used for the dialogs.
     */
    void setDialogParent( QWidget *parent );

    /**
      Handles sending of invitations for newly created incidences. This method
      asserts that we (as in any of the identities or mail addresses known to
      Kontact/PIM) are the organizer.
      @param incidence The new incidence.
     */
    SendStatus sendIncidenceCreatedMessage( const KCal::Incidence::Ptr &incidence );

    /**
      Handles sending of invitations for modified incidences.
      @param incidence The modified incidence.
      @param attendeeSatusChanged ????
     */
    SendStatus sendIncidenceModifiedMessage( KCal::iTIPMethod method,
                                             const KCal::Incidence::Ptr &incidence,
                                             bool attendeeStatusChanged );

    /**
      Handles sending of ivitations for deleted incidences.
      @param incidence The deleted incidence.
     */
    SendStatus sendIncidenceDeletedMessage( KCal::iTIPMethod method,
                                            const KCal::Incidence::Ptr &incidence );

    /**
      Send counter proposal message.
      @param oldEvent The original event provided in the invitations.
      @param newEvent The new event as edited by the user.
    */
    SendStatus sendCounterProposal( const KCal::Event::Ptr &oldEvent, const KCal::Event::Ptr &newEvent ) const;

Q_SIGNALS:
    /**
      This signal is emitted when an invitation for a counter proposal is sent.
      @param incidence The incidence for which the counter proposal must be specified.
     */
    void editorRequested( const KCal::Incidence::Ptr &incidence );

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( InvitationHandler )
};

}

#endif // INVITATIONSENDER_H
