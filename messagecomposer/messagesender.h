/*
    messagesender.h

    This file is part of KMail, the KDE mail client
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KMAIL_MESSAGESENDER_H__
#define __KMAIL_MESSAGESENDER_H__
#include <QString>

#include <kmime/kmime_message.h>

class MessageSender {
protected:
  virtual ~MessageSender() = 0;

public:
  enum SendMethod {
    SendDefault = -1,
    SendImmediate = true,
    SendLater = false
  };
  enum SaveIn {
    SaveInNone,
    SaveInDrafts,
    SaveInTemplates
  };
  /**
     Send given message.

     The message is either queued (@p method == SendLater) or sent
     immediately (@p method = SendImmediate). The default behaviour,
     as selected with setSendImmediate(), can be overwritten with the
     parameter @p method.  The sender takes ownership of the given
     message on success, so DO NOT DELETE OR MODIFY the message
     further.

     FIXME: what about send() == false?

     @return true on success.
  */
  bool send( const KMime::Message::Ptr &msg, SendMethod method=SendDefault ) { return doSend( msg, method ); }

  /**
     Start sending all queued messages.

     FIXME: what does success mean here, if it's only _start_ sending?

     Optionally a transport can be specified that will be used as the
     default transport.

     @return true on success.
  */
  bool sendQueued( const QString & transport=QString() ) { return doSendQueued( transport ); }

protected:
  virtual bool doSend( const KMime::Message::Ptr &msg, short sendNow ) = 0;
  virtual bool doSendQueued( const QString& transport ) = 0;
};

inline MessageSender::~MessageSender() {}


#endif /* __KMAIL_MESSAGESENDER_H__ */

