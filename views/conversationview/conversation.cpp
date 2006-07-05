/*
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

#include "conversation.h"

int Conversation::count() const
{
  return messages.count();
}

QString Conversation::conversationTitle() const
{
  return title;
}

Message Conversation::message(int messageId) const
{
  if (messageId < 0 || messageId >= messages.count())
    return Message(true);
  return messages.at(messageId);
}

void Conversation::addMessage(Message &message)
{
  messages << message;
//  emit messageAdded();
}

QString Conversation::author(int messageId) const
{
  Message tmp = message(messageId);
  if (! tmp.isNull())
    return tmp.author();
  return 0;
}

QString Conversation::content(int messageId) const
{
  Message tmp = message(messageId);
  if (! tmp.isNull())
    return tmp.content();
  return 0;
}

QDateTime Conversation::arrivalTime(int messageId) const
{
  Message tmp = message(messageId);
  return tmp.arrivalTime();
}

QString Conversation::arrivalTimeInText(int messageId) const
{
  Message tmp = message(messageId);
  return tmp.arrivalTimeInText();
}

QDateTime Conversation::sendTime(int messageId) const
{
  Message tmp = message(messageId);
  return tmp.sendTime();
}

QDateTime Conversation::arrivalTime() const
{
  return messages.last().arrivalTime();
}

QString Conversation::arrivalTimeInText() const
{
  return messages.last().arrivalTimeInText();
}

QDateTime Conversation::sendTime() const
{
  int max = count();
  QDateTime tmp;
  QDateTime *oldest = new QDateTime(QDate(0, 0, 0), QTime(0, 0));
  for (int count = 0; count < max; ++count) {
    tmp = messages.at(count).sendTime();
    if (tmp > *oldest)
      oldest = &tmp;
  }
  return *oldest;
}

bool Conversation::operator!=(Conversation &compare) const
{
  return arrivalTime() != compare.arrivalTime();
}

bool Conversation::operator<(Conversation &compare) const
{
  return arrivalTime() < compare.arrivalTime();
}

bool Conversation::operator<=(Conversation &compare) const
{
  return arrivalTime() <= compare.arrivalTime();
}

bool Conversation::operator==(Conversation &compare) const
{
  return arrivalTime() == compare.arrivalTime();
}

bool Conversation::operator>=(Conversation &compare) const
{
  return arrivalTime() >= compare.arrivalTime();
}

bool Conversation::operator>(Conversation &compare) const
{
  return arrivalTime() > compare.arrivalTime();
}

/** 
 * This function generates a list of authors contributing to this conversation. No author is included twice.
 * @return a list of authors contributing to this conversation
 * TODO: feed it with a length to determine how much can be displayed. 
 * TODO: only display two or three authors: first, first unread and last unread
 */
QString Conversation::authors() const
{
  QString text = author(0);
  QString me;
  foreach (me, listOfMe) {
    text.replace(QRegExp(me), "me");
  }
  int max = count();
  for (int i = 1; i < max; ++i) {
    QString tmpAuthor = author(i);
    foreach (me, listOfMe) {
      tmpAuthor.replace(QRegExp(me), tr("me"));
    }
    if (!text.contains(tmpAuthor)) {
      text.append(", ");
      text.append(tmpAuthor);
    }
  }
  return text;
}
