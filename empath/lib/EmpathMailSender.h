/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma interface "EmpathMailSender.h"
#endif

#ifndef EMPATHMAILSENDER_H
#define EMPATHMAILSENDER_H

// Qt includes
#include <qvaluelist.h>
#include <qobject.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include <RMM_Message.h>

/**
 * @short Sender base class
 * Responsibility is to queue messages in a local spool for later delivery
 * by a derived class.
 * @author Rikkus
 */

class EmpathMailSender : public QObject
{
    Q_OBJECT

    public:

        enum OutgoingServerType    { Sendmail, Qmail, SMTP };
        enum SendPolicy            { SendNow, SendLater };

        EmpathMailSender();

        virtual ~EmpathMailSender() = 0L;
        
        /**
         * Queue up a message for sending.
         */
        void queue(RMM::RMessage &);

        /**
         * Send one message.
         * 
         * @returns false if the message could not be delivered.
         *
         * Message will be returned to user on failure. FIXME: How ?
         */
        void send(RMM::RMessage &);

        /**
         * Send one message.
         */
        virtual void sendOne(RMM::RMessage & message) = 0L;

        /**
         * Kick off a send using all queued messages.
         */
        void sendQueued();

        /**
         * Save your config now !
         * Called by Empath when settings have changed.
         */
        virtual void saveConfig() = 0;
        /**
         * Load your config now !
         * Called by Empath on startup.
         */
        virtual void readConfig() = 0;
        
    private:
        
        void emergencyBackup(RMM::RMessage &);
};

#endif

// vim:ts=4:sw=4:tw=78
