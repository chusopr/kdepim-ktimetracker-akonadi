#ifndef SSK_KARM_H
#define SSK_KARM_H

#include <stdio.h>
#include <stdlib.h>

#include <qsplitter.h>
#include <klistview.h>
#include <qptrlist.h>
#include <qtextstream.h>

class KMenuBar;
class KToolBar;
class QListBox;
class AddTaskDialog;
class IdleTimer;
class QTimer;
class Preferences;
class Task;

class Karm : public KListView
{
    Q_OBJECT

private: // member variables
    IdleTimer *_idleTimer;
    QTimer *_minuteTimer;
    QTimer *_autoSaveTimer;

    Preferences *_preferences;

    QPtrList<Task> activeTasks;

public:
    Karm( QWidget *parent = 0, const char *name = 0 );
    virtual ~Karm();
    static QString formatTime(long minutes);

private:
    void updateParents( QListViewItem* task, long totalDiff, long sesssionDiff );

public slots:
    /*
    File format:
    zero or more lines of
    1 		number
    time	in minutes
    string	task name
    */
    void load();
    void save();
    void writeTaskToFile(QTextStream *, QListViewItem *, int);
    bool parseLine(QString line, long *time, QString *name, int *level);
    void stopCurrentTimer();
    void stopAllTimers();
    void startTimer();
    void changeTimer(QListViewItem *);
    void newTask();
    void newTask(QString caption, QListViewItem *parent);
    void newSubTask();
    void editTask();
    void deleteTask();
    void extractTime(int minutes);
    void resetSessionTimeForAllTasks();

protected slots:
    void autoSaveChanged(bool);
    void autoSavePeriodChanged(int period);
    void minuteUpdate();

signals:
    void sessionTimeChanged( long difference );
    void timerTick();
    void updateButtons();

protected slots:
    void stopChildCounters(Task *item);
    void addTimeToActiveTasks(int minutes);
};

inline QString Karm::formatTime( long minutes )
{
    QString time;
    time.sprintf("%ld:%02ld", minutes / 60, labs(minutes % 60));
    return time;
}

#endif
