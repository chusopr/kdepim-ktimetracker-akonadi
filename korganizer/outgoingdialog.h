/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef OUTGOINGDIALOG_H
#define OUTGOINGDIALOG_H
// $Id$

#include <qlistview.h>
#include <qmap.h>
#include <qstring.h>

#include <libkcal/scheduler.h>

#include "outgoingdialog_base.h"

using namespace KCal;

class ScheduleItemOut : public QListViewItem
{
  public:
    ScheduleItemOut(QListView *parent,Event *ev,Scheduler::Method method,
                 const QString &recipients=QString::null);
    virtual ~ScheduleItemOut() {}

    Event *event() { return mEvent; }
    Scheduler::Method method() { return mMethod; }
    QString recipients() { return mRecipients; }

  private:
    Event *mEvent;
    Scheduler::Method mMethod;
    QString mRecipients;
};

class OutgoingDialog : public OutgoingDialog_base
{
    Q_OBJECT
  public:
    OutgoingDialog(Calendar *,QWidget* parent=0,const char* name=0,
                   bool modal=false,WFlags fl=0);
    ~OutgoingDialog();

    bool addMessage(Event *,Scheduler::Method);
    bool addMessage(Event *,Scheduler::Method,const QString &recipients);

  signals:
    void numMessagesChanged(int);

  protected slots:
    void send();
    void deleteItem();
    void showEvent(QListViewItem *);

  private:
    bool saveMessage(Incidence *,Scheduler::Method,const QString &recipients=0);
    bool deleteMessage(Incidence *);
    void loadMessages();
    Calendar *mCalendar;
    Scheduler *mScheduler;
    QMap<Incidence*, QString> mMessageMap;
};

#endif // OUTGOINGDIALOG_H
