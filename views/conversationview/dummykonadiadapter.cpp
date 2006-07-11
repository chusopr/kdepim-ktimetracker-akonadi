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

#include <QDate>
#include <QTime>

#include "dummykonadiadapter.h"
#include "message.h"

DummyKonadiAdapter::DummyKonadiAdapter(QStringList &manyMe)
{
    listOfMe = manyMe;
    Conversation* one = new Conversation(manyMe, &QString("My first post"));
    Message* msg;
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("This is my first post.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(13, 18, 26)));
    msg->markAs(true);
    one->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Aron Bostrom");
    msg->setContent("Great! Welcome along.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(17, 20)));
    msg->markAs(true);
    one->addMessage(*msg);
  Conversation *two = new Conversation(manyMe, &QString("Fwd: Eran d0llars in this oi| inve$tment"));
    msg = new Message();
    msg->setAuthor("Nigeria Shell Inv. inc.");
    msg->setContent("Read this:\n>Dear sire/madame. We have a great opportunity, you can be a millionaire!!! All you have to do is to help as with initial cash in our newst business idea.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(13, 19, 52)));
    msg->markAs(true);
    two->addMessage(*msg);
  Conversation *three = new Conversation(manyMe, &QString("[KDE-PSYKO] Hi there!"));
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("Who are you?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(15, 1)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Aron Bostrom");
    msg->setContent("I am you.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(15, 12)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Anonymous bastard");
    msg->setContent("And I am schizofrene.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(17, 20)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("Well, that explains a lot...");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(23, 11)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Aron Bostrom");
    msg->setContent("Hrafnahnef wrote:\n>Well, that explains a lot...\n\nLike what? \n//aron");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(0, 1)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("It explains you *both*, the two of you, or the three of us or meybee _me_?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(0, 7)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Anonymous bastad");
    msg->setContent("Yep, that's it. It explains me. But it doesn't answer who I am. Who am i?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(0, 18)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("Anonymous bastard wrote:\n>Yep, that's it. It explains me. But it doesn't answer who I am. Who am i?\n\n$ whoami\nhrafnahnef\n\nSettled?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(0, 23)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Anonymous bastard");
    msg->setContent("Agreed!\n\nYou wrote:\n>$ whoami\n>hrafnahnef\n>\n>Settled?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(0, 27)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Aron Bostrom");
    msg->setContent("But if I am you, and you are you, then who is me?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(0, 32)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Anonymous bastard");
    msg->setContent("That would be me!\n\nAron Bostrom wrote:\n>But if I am you, and you are you, then who is me?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(0, 48)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("Really?\n\nOn Tue 13 Bastard, anonymous wrote:\n>That would be me!\n>\n>Aron Bostrom wrote:\n>>But if I am you, and you are you, then who is me?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(1, 7)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Supervisor");
    msg->setContent("Reallyreally!\n\nNow shut up, the three or three of you. Elseif I will kickban you in the privates, this is a _public_ channel, not your own brain.\n\n..admin");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(7, 52)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Transistor guy");
    msg->setContent("Finally!!! Somethree understands us, except Hnefahrafn. Nobody will ever stand him.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(8, 12)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("It's Hrafnahnef!! H-r-a-f-n-a-h-n-e-f, and I am standable, though not understandable.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(8, 48)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Supervisor");
    msg->setContent("Ok, that's it. You are banned.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(9, 18)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Anonymous bastard, Jr.");
    msg->setContent("I'm alive! Death couldn't keep me from this list! I have returned to haunt you from my new brain, and e-mail.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(15, 23)));
    msg->markAs(true);
    three->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Mr. Troll");
    msg->setContent("*Sigh*\n\nAt least not all three of him returned (he or she?).\nAll we can do is to not feed the troll. (I'm getting hungry.)\n\nIn KMail filters I'm about to put my hope.\n\n//*gruntgrunt* from the depth of the woods\n\nAnonymous bastard, Jr. wrote:\n>I'm alive! Death couldn't keep me from this list! I have returned to haunt you from my new brain, and e-mail.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(16, 43)));
    three->addMessage(*msg);
  Conversation *four = new Conversation(manyMe, &QString("What does 'Hrafnahnef' mean?"));
    msg = new Message();
    msg->setAuthor("Mr. Troll");
    msg->setContent("What does 'Hrafnahnef' mean. Is it really your true name?");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 23), QTime(23, 7)));
    msg->markAs(true);
    four->addMessage(*msg);
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("It's old norse, meaning 'chief of ravens' (Hrafn=raven; hnef=chief/king). And it's *not* my true name. At least not yet.");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 24), QTime(12, 1)));
    four->addMessage(*msg);
  Conversation *five = new Conversation(manyMe, &QString("foldermodel.h"));
    msg = new Message();
    msg->setAuthor("Hrafnahnef");
    msg->setContent("#ifndef FOLDERMODEL_H\n#define FOLDERMODEL_H\n\n#include <QVariant>\n#include <QModelIndex>\n#include <QAbstractListModel>\n#include <QStringList>\n\n#include \"dummykonadiadapter.h\"\n\nclass FolderModel : public QAbstractListModel\n{\n  Q_OBJECT\npublic:\n  FolderModel(const DummyKonadiAdapter &dummydata, QObject *parent = 0) : QAbstractListModel(parent), backend(dummydata) {}\n\n  int rowCount(const QModelIndex &parent = QModelIndex()) const;\n  QVariant data(const QModelIndex &index, int role) const;\n  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;\n\nprivate:\n  DummyKonadiAdapter backend;\n};\n\n#endif\n");
    msg->setArrivalTime(QDateTime(QDate(2006, 6, 25), QTime(23, 7)));
    msg->markAs(true);
    five->addMessage(*msg);
  conversations << one << two << three << four << five;
//  sort();
}

DummyKonadiAdapter::~DummyKonadiAdapter()
{
  Conversation* tmpConversation;
  foreach (tmpConversation, conversations) {
    delete tmpConversation;
  }
}

/*void DummyKonadiAdapter::sort(bool descending)
{
  if (descending)
    qSort(conversations.begin(), conversations.end(), qGreater<Conversation*>());
  else
    qSort(conversations);
}*/

int DummyKonadiAdapter::conversationCount() const
{
  return conversations.count();
}

QString DummyKonadiAdapter::conversationTitle(int conversationId) const
{
  if (!tryConversationId(conversationId))
    return 0;
  return conversations.at(conversationId)->conversationTitle();
}

int DummyKonadiAdapter::messageCount(int conversationId) const
{
  if (!tryConversationId(conversationId)) 
    return -1;
  return conversations.at(conversationId)->count();
}


QString DummyKonadiAdapter::messageContent(int conversationId, int messageId) const
{
  if (!tryConversationId(conversationId)) 
    return 0;
  return conversations.at(conversationId)->content(messageId);
}

QString DummyKonadiAdapter::messageAuthor(int conversationId, int messageId) const
{
  if (!tryConversationId(conversationId)) 
    return 0;
  return conversations.at(conversationId)->author(messageId);
}

bool DummyKonadiAdapter::tryConversationId(int conversationId) const
{
  int lowerLimit = 0;
  int upperLimit = conversations.count() - 1;
  if (conversationId < lowerLimit || conversationId > upperLimit)
    return false;
  return true;
}

Conversation* DummyKonadiAdapter::conversation(int conversationId) const
{
  if (!tryConversationId(conversationId)) 
    return 0;
  return conversations.at(conversationId);
}

QDateTime DummyKonadiAdapter::messageSendTime(int conversationId, int messageId) const
{
  return conversations.at(conversationId)->sendTime(messageId);
}

QDateTime DummyKonadiAdapter::messageArrivalTime(int conversationId, int messageId) const
{
  return conversations.at(conversationId)->arrivalTime(messageId);
}

QString DummyKonadiAdapter::messageArrivalTimeInText(int conversationId, int messageId) const
{
  return conversations.at(conversationId)->arrivalTimeInText(messageId);
}

QDateTime DummyKonadiAdapter::conversationSendTime(int conversationId) const
{
  return conversations.at(conversationId)->sendTime();
}

QDateTime DummyKonadiAdapter::conversationArrivalTime(int conversationId) const
{
  return conversations.at(conversationId)->arrivalTime();
}

QString DummyKonadiAdapter::conversationArrivalTimeInText(int conversationId) const
{
  return conversations.at(conversationId)->arrivalTimeInText();
}

QStringList DummyKonadiAdapter::authorsInConversation(int index) const
{
  Conversation *tmp = conversations.at(index);
  QStringList tmpList;
  int max = tmp->count();
  for (int count = 0; count < max; ++count) {
    tmpList << tmp->author(count);
  }
  return tmpList;
}


QString DummyKonadiAdapter::subjectOfConversation(int index) const
{
  return conversationTitle(index);
}

/* TODO */
QString DummyKonadiAdapter::previewOfConversation(int /*index*/) const
{
  return QString("");
}

int DummyKonadiAdapter::nbrOfMessagesInConversation(int index) const
{
  return messageCount(index);
}

QDateTime DummyKonadiAdapter::dateTimeOfConversation(int index) const
{
  return conversationArrivalTime(index);
}

QString DummyKonadiAdapter::dateTimeOfConversationInText(int index) const
{
  return conversationArrivalTimeInText(index);
}

/* TODO */
bool DummyKonadiAdapter::unreadStatusOfConversation(int /*index*/) const
{
  return false;
}
