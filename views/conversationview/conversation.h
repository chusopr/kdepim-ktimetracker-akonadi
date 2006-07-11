/*
 * conversation.h
 *
 * copyright (c) Aron Bostrom <Aron.Bostrom at gmail.com>, 2006 
 *
 * this library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QDateTime>

#include "message.h"

class Conversation : QObject
{
  Q_OBJECT
public:
  Conversation(QStringList &manyMe, QString *conversationTitle = 0, QObject *parent = 0) : QObject(parent), title(*conversationTitle), listOfMe(manyMe) {}
  int count() const;
  QString conversationTitle() const;
  QDateTime arrivalTime() const;
  QString arrivalTimeInText() const;
  QDateTime sendTime() const;
  Message message(int messageId) const;
  void addMessage(Message &message);
  QString author(int messageId) const;
  QString authors() const;
  QString content(int messageId) const;
  QDateTime arrivalTime(int messageId) const;
  QString arrivalTimeInText(int messageId) const;
  QDateTime sendTime(int messageId) const;
  bool isUnread() const;
  bool numberUnread() const;
  void markAs(bool read);
  bool operator!=(Conversation &compare) const;
  bool operator<(Conversation &compare) const;
  bool operator<=(Conversation &compare) const;
  bool operator==(Conversation &compare) const;
  bool operator>=(Conversation &compare) const;
  bool operator>(Conversation &compare) const;

//public signals:
//  void messageAdded();

private:
  QString title;
  QList<Message> messages;
  QStringList listOfMe;
};

#endif
