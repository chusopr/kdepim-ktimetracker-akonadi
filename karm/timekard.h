#ifndef KARM_TIMEKARD_H
#define KARM_TIMEKARD_H

#undef Color // X11 headers
#undef GrayScale // X11 headers
#include <kprinter.h>
//#include <qdate.h>

#include "karmstorage.h"

class QString;
class QDate;

class TaskView;


/**
 *  Seven consecutive days.
 *
 *  The timecard report prints out one table for each week of data.  The first
 *  day of the week should be read from the KControlPanel.  Currently, it is
 *  hardcoded to Sunday.
 */
class Week
{
  public:
    /** Need an empty constructor to use in a QValueList. */
    Week();
    Week(QDate from);
    QDate start() const;
    QDate end() const;
    QValueList<QDate> days() const;

    /**
     * Returns a list of weeks for the given date range.
     *
     * The first day of the week is picked up from the settings in the
     * KontrolPanel.
     *
     * The list is inclusive; for example, if you pass in a date range of two
     * days, one being a Sunday and the other being a Monday, you will get two
     * weeks back in the list.
     */
    static QValueList<Week> weeksFromDateRange(const QDate& from, 
        const QDate& to);

    /**
     *  Return the name of the week.
     *
     *  Uses whatever the user has set up for the long date format in
     *  KControlPanel, prefixed by "Week of".
     */
    QString name() const;


  private:
    QDate _start;
};

/**
 *  Routines to output timecard data.
 */
class TimeKard
{
  public:
    TimeKard() {};

    /**
     * Generates ascii text of task totals, for current task on down.
     *
     * Formatted for pasting into clipboard.
     *
     * @param justThisTask Only useful when user has picked a root task.  We
     * use this parameter to distinguish between when a user just wants to
     * print the task subtree for a root task and when they want to print 
     * all tasks.
     */
    QString totalsAsText(TaskView* taskview, bool justThisTask = true);

    /**
     * Generates ascii text of weekly task history, for current task on down.
     *
     * Formatted for pasting into clipboard.
     */
    QString historyAsText(TaskView* taskview, const QDate& from, 
        const QDate& to, bool justThisTask = true);

    void printTask(Task *t, QString &s, int level);

    void printWeekTask(const Task *t, const QMap<QString,long>& datamap, 
         const Week& week, const int level, QString& retval, long& sum);
  
};
#endif // KARM_TIMEKARD_H
