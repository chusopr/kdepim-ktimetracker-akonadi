/*
    This file is part of the KDE alarm daemon.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H
//
// Alarm dialog.
//
#include <qtimer.h>
#include <qdatetime.h>

#include <kdialogbase.h>

#include <libkcal/event.h>
#include <libkcal/calendarresources.h>

using namespace KCal;

class AlarmListItem;
class KOEventViewer;
class KComboBox;
class KListView;
class QSpinBox;
class QSplitter;

class AlarmDialog : public KDialogBase {
  Q_OBJECT
  public:
    explicit AlarmDialog( CalendarResources *calendar, QWidget *parent = 0, const char *name = 0 );

    ~AlarmDialog();

    void addIncidence( Incidence *incidence, const QDateTime &reminderAt,
                       const QString &displayText );
    void eventNotification();

  public slots:

    void slotOk();    // suspend
    void slotUser1(); // edit
    void slotUser2(); // dismiss all
    void slotUser3(); // dismiss selected
    void slotSave();
    void wakeUp();
    void show();
    void edit();
    void suspend();
    void suspendAll();
    void dismissAll();
    void dismissCurrent();

    /**
       If an incidence changed, for example in korg, we must update
       the date and summary shown in the list view.
    */
    void slotCalendarChanged();

  signals:
    void reminderCount( int count );

  private slots:
    void updateButtons();
    void showDetails();

  protected:
    void closeEvent( QCloseEvent * );

  private:

    static QDateTime triggerDateForIncidence( Incidence *inc,
                                              const QDateTime &reminderAt,
                                              QString &displayStr );

    void readLayout();
    void writeLayout();
    AlarmListItem *searchByUid( const QString &uid );
    bool ensureKorganizerRunning() const;
    void setTimer();
    int activeCount();
    QValueList<AlarmListItem*> selectedItems() const;

    CalendarResources *mCalendar;
    KListView *mIncidenceListView;
    KOEventViewer *mDetailView;

    QSpinBox *mSuspendSpin;
    QSplitter *mSplitter;
    KComboBox *mSuspendUnit;
    QTimer mSuspendTimer;
};

#endif
