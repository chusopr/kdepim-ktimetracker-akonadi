/*
 *     Copyright (C) 1997 by Stephan Kulow <coolo@kde.org>
 *                   2007 the ktimetracker developers
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the
 *      Free Software Foundation, Inc.
 *      51 Franklin Street, Fifth Floor
 *      Boston, MA  02110-1301  USA.
 *
 */

#include "task.h"

#include <QDateTime>
#include <QString>
#include <QTimer>
#include <QPixmap>

#include <KDebug>
#include <KIconLoader>
#include <KComponentData>

#include <KCalCore/Event>

#include "ktimetrackerutility.h"
#include "ktimetracker.h"
#include "preferences.h"

QVector<QPixmap*> *Task::icons = 0;

Task::Task( const QString& taskName, const QString& taskDescription, long minutes, long sessionTime,
            DesktopList desktops, TaskView *parent, bool konsolemode )
  : QObject(), QTreeWidgetItem(parent), taskTodo(new KCalCore::Todo)
{
    populateTodo(taskName, taskDescription, minutes, sessionTime, desktops);
    init(konsolemode);
}

Task::Task( const QString& taskName, const QString& taskDescription, long minutes, long sessionTime,
            DesktopList desktops, Task *parent)
  : QObject(), QTreeWidgetItem(parent), taskTodo(new KCalCore::Todo)
{
    populateTodo(taskName, taskDescription, minutes, sessionTime, desktops);
    init();
}

Task::Task( const KCalCore::Todo::Ptr &todo, TaskView* parent, bool konsolemode )
  : QObject(), QTreeWidgetItem( parent )
{
    long sessionTime = 0;
    taskTodo = todo;

    parseIncidence( taskTodo, sessionTime );
    init(konsolemode);
}

int Task::depth()
// Deliver the depth of a task, i.e. how many tasks are supertasks to it.
// A toplevel task has the depth 0.
{
    kDebug(5970) << "Entering function";
    int res=0;
    Task* t=this;
    while ( ( t = t->parent() ) ) res++;
    kDebug(5970) << "Leaving function. depth is:" << res;
    return res;
}

void Task::populateTodo(const QString& taskName, const QString& taskDescription, long minutes, long sessionTime, DesktopList desktops)
{
    taskTodo->setSummary(taskName);
    taskTodo->setDescription(taskDescription);
    taskTodo->setCustomProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray( "totalTaskTime" ), QString::number(minutes));
    taskTodo->setCustomProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray( "totalSessionTime" ), QString::number(sessionTime));
    taskTodo->setCustomProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray( "sessionStartTiMe" ), "0");
    taskTodo->setOrganizer( KTimeTrackerSettings::userRealName() );
    setDesktopList(desktops);
}

void Task::init(bool konsolemode )
{
    const TaskView *taskView = qobject_cast<TaskView*>( treeWidget() );
    // If our parent is the taskview then connect our totalTimesChanged
    // signal to its receiver
    if ( ! parent() )
        connect( this, SIGNAL(totalTimesChanged(long,long)),
                 taskView, SLOT(taskTotalTimesChanged(long,long)));

    connect( this, SIGNAL(deletingTask(Task*)),
             taskView, SLOT(deletingTask(Task*)));

    if (icons == 0)
    {
        icons = new QVector<QPixmap*>(8);
        if (!konsolemode)
        {
            KIconLoader kil("ktimetracker");
            for (int i=0; i<8; i++)
            {
                QPixmap *icon = new QPixmap();
                QString name;
                name.sprintf("watch-%d.xpm",i);
                *icon = kil.loadIcon( name, KIconLoader::User );
                icons->insert(i,icon);
            }
        }
    }

    mRemoving = false;
    mLastStart = QDateTime::currentDateTime();
    mTotalTime = time();
    mTotalSessionTime = sessionTime();
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updateActiveIcon()));
    if ( !konsolemode ) setIcon(1, UserIcon(QString::fromLatin1("empty-watch.xpm")));
    mCurrentPic = 0;

    update();
    changeParentTotalTimes( sessionTime(), time());

    // alignment of the number items
    for (int i = 1; i < columnCount(); ++i)
    {
        setTextAlignment( i, Qt::AlignRight );
    }

    // .. but not the priority column
    setTextAlignment( 5, Qt::AlignCenter );
}

Task::~Task()
{
    emit deletingTask(this);
    delete mTimer;
}

void Task::delete_recursive()
{
    while ( this->child(0) )
    {
        Task* t=(Task*) this->child(0);
        t->delete_recursive();
    }
    delete this;
}

void Task::setRunning( bool on, timetrackerstorage* storage, const QDateTime &when )
// This is the back-end, the front-end is StartTimerFor()
{
    kDebug(5970) << "Entering function";
    if ( on )
    {
        if (!mTimer->isActive())
        {
            mTimer->start(1000);
            storage->startTimer(this);
            mCurrentPic=7;
            mLastStart = when;
            kDebug(5970) << "task has been started for " << when;
            updateActiveIcon();
        }
    }
    else
    {
        if (mTimer->isActive())
        {
            mTimer->stop();
            if ( ! mRemoving )
            {
                storage->stopTimer(this, when);
                setIcon(1, UserIcon(QString::fromLatin1("empty-watch.xpm")));
            }
        }
    }
}

void Task::resumeRunning()
// setRunning is the back-end, the front-end is StartTimerFor().
// resumeRunning does the same as setRunning, but not add a new
// start date to the storage.
{
    kDebug(5970) << "Entering function";
    if (!mTimer->isActive())
    {
        mTimer->start(1000);
        mCurrentPic=7;
        updateActiveIcon();
    }
}

void Task::setUid( const QString &uid )
{
    taskTodo->setUid(uid);
}

bool Task::isRunning() const
{
    return mTimer->isActive();
}

void Task::setName( const QString& name, timetrackerstorage* storage )
{
    kDebug(5970) << "Entering function, name=" << name;

    QString oldname = taskTodo->summary().trimmed();
    if ( oldname != name )
    {
        taskTodo->setSummary(name);
        storage->setName(this, oldname);
        update();
    }
}

void Task::setDescription( const QString& description )
{
    kDebug(5970) << "Entering function, description=" << description;

    QString olddescription = taskTodo->description();
    if ( olddescription != description )
    {
        taskTodo->setDescription(description);
        update();
    }
}

void Task::setPercentComplete(const int percent, timetrackerstorage *storage)
{
    kDebug(5970) << "Entering function(" << percent <<", storage):" << taskTodo->uid();

    if (!percent)
        taskTodo->setPercentComplete(0);
    else if (percent > 100)
        taskTodo->setPercentComplete(100);
    else if (percent < 0)
        taskTodo->setPercentComplete(0);
    else
        taskTodo->setPercentComplete(percent);

    if (isRunning() && percentComplete()==100) taskView()->stopTimerFor(this);

    setPixmapProgress();

    // When parent marked as complete, mark all children as complete as well.
    // This behavior is consistent with KOrganizer (as of 2003-09-24).
    if (percentComplete() == 100)
    {
        for ( int i = 0; i < childCount(); ++i )
        {
            Task *task = static_cast< Task* >( child( i ) );
            task->setPercentComplete(percentComplete(), storage);
        }
    }
    // maybe there is a column "percent completed", so do a ...
    update();
}

void Task::setPriority( int priority )
{
    if ( priority < 0 )
    {
        priority = 0;
    }
    else if ( priority > 9 )
    {
        priority = 9;
    }

    taskTodo->setPriority(priority);
    update();
}

void Task::setPixmapProgress()
{
    kDebug(5970) << "Entering function";
    QPixmap icon;
    KIconLoader* kil = new KIconLoader();
    if (percentComplete() >= 100)
    {
        const QString iconcomplete=QString("task-complete.xpm");
        icon = kil->loadIcon( iconcomplete, KIconLoader::User );
    }
    else
    {
        const QString iconincomplete=QString("task-incomplete.xpm");
        icon = kil->loadIcon( iconincomplete, KIconLoader::User );
    }
    setIcon(0, icon);
    delete kil;
    kDebug(5970) << "Leaving function";
}

bool Task::isComplete() { return percentComplete() == 100; }

void Task::setDesktopList ( DesktopList desktopList )
{
    if ( desktopList.empty() )
    {
        taskTodo->removeCustomProperty( KGlobal::mainComponent().componentName().toUtf8(),
            QByteArray( "desktopList" ));
    }
    else
    {
        QString desktopstr;
        for ( DesktopList::const_iterator iter = desktopList.begin();
            iter != desktopList.end();
            ++iter )
        {
            desktopstr += QString::number( *iter ) + QString::fromLatin1( "," );
        }
        desktopstr.remove( desktopstr.length() - 1, 1 );
        taskTodo->setCustomProperty( KGlobal::mainComponent().componentName().toUtf8(),
            QByteArray( "desktopList" ), desktopstr);
    }
}

QString Task::addTime( long minutes )
{
    kDebug(5970) << "Entering function";
    QString err;
    taskTodo->setCustomProperty(
      KGlobal::mainComponent().componentName().toUtf8(),
      QByteArray("totalTaskTime"),
      QString::number(time()+minutes)
    );
    this->addTotalTime( minutes );
    kDebug(5970) << "Leaving function";
    return err;
}

QString Task::addTotalTime( long minutes )
{
    kDebug(5970) << "Entering function";
    QString err;
    mTotalTime+=minutes;
    if ( parent() ) parent()->addTotalTime( minutes );
    kDebug(5970) << "Leaving function";
    return err;
}

QString Task::addSessionTime( long minutes )
{
    kDebug(5970) << "Entering function";
    QString err;
    taskTodo->setCustomProperty(
      KGlobal::mainComponent().componentName().toUtf8(),
      QByteArray( "totalSessionTime" ),
      QString::number(sessionTime()+minutes)
    );
    this->addTotalSessionTime( minutes );
    kDebug(5970) << "Leaving function";
    return err;
}

QString Task::addTotalSessionTime( long minutes )
{
    kDebug(5970) << "Entering function";
    QString err;
    mTotalSessionTime+=minutes;
    if ( parent() ) parent()->addTotalSessionTime( minutes );
    kDebug(5970) << "Leaving function";
    return err;
}

QString Task::setTime( long minutes )
{
    kDebug(5970) << "Entering function";
    QString err;
    taskTodo->setCustomProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray("totalTaskTime"), QString::number(minutes));
    mTotalTime+=minutes;
    kDebug(5970) << "Leaving function";
    return err;
}

QString Task::recalculatetotaltime()
{
    QString result;
    setTotalTime(0);
    Task* child;
    for (int i=0; i<this->childCount(); ++i)
        child=(Task*)this->child(i);
    addTotalTime(time());
    return result;
}

QString Task::recalculatetotalsessiontime()
{
    QString result;
    setTotalSessionTime(0);
    Task* child;
    for (int i=0; i<this->childCount(); ++i)
        child=(Task*)this->child(i);
    addTotalSessionTime(time());
    return result;
}

QString Task::setSessionTime( long minutes )
{
    kDebug(5970) << "Entering function";
    QString err;
    taskTodo->setCustomProperty(
      KGlobal::mainComponent().componentName().toUtf8(),
      QByteArray( "totalSessionTime" ),
      QString::number(minutes)
    );
    mTotalSessionTime+=minutes;
    kDebug(5970) << "Leaving function";
    return err;
}

void Task::changeTimes( long minutesSession, long minutes, timetrackerstorage* storage)
{
    kDebug(5970) << "Entering function";
    kDebug() << "Task's sessionStartTiMe is " << sessionStartTiMe();
    if( minutesSession != 0 || minutes != 0)
    {
        taskTodo->setCustomProperty(
          KGlobal::mainComponent().componentName().toUtf8(),
          QByteArray( "totalSessionTime" ),
          QString::number(sessionTime()+minutesSession)
        );
        taskTodo->setCustomProperty(
          KGlobal::mainComponent().componentName().toUtf8(),
          QByteArray("totalTaskTime"),
          QString::number(time()+minutes)
        );
        if ( storage ) storage->changeTime(this, minutes * secsPerMinute);
        changeTotalTimes( minutesSession, minutes );
    }
    kDebug(5970) << "Leaving function";
}

void Task::changeTime( long minutes, timetrackerstorage* storage )
{
    changeTimes( minutes, minutes, storage);
}

void Task::changeTotalTimes( long minutesSession, long minutes )
{
    kDebug(5970)
        << "Task::changeTotalTimes(" << minutesSession << ","
        << minutes << ") for" << name();
    mTotalSessionTime += minutesSession;
    mTotalTime += minutes;
    update();
    changeParentTotalTimes( minutesSession, minutes );
    kDebug(5970) << "Leaving function";
}

long Task::time() const
{
  return taskTodo->customProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray("totalTaskTime")).toLong();
};

long Task::sessionTime() const
{
  return taskTodo->customProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray("totalSessionTime")).toLong();
}

void Task::resetTimes()
{
    kDebug(5970) << "Entering function";
    mTotalSessionTime -= sessionTime();
    mTotalTime -= time();
    changeParentTotalTimes( -sessionTime(), -time());
    taskTodo->setCustomProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray( "totalSessionTime" ), "0");
    taskTodo->setCustomProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray("totalTaskTime"), "0");
    update();
    kDebug(5970) << "Leaving function";
}

void Task::changeParentTotalTimes( long minutesSession, long minutes )
{
    if ( isRoot() )
        emit totalTimesChanged( minutesSession, minutes );
    else
        parent()->changeTotalTimes( minutesSession, minutes );
}

bool Task::remove( timetrackerstorage* storage)
{
    kDebug(5970) << "entering function" << taskTodo->summary().trimmed();
    bool ok = true;

    mRemoving = true;
    storage->removeTask(this);
    if( isRunning() ) setRunning( false, storage );

    for ( int i = 0; i < childCount(); ++i )
    {
        Task *task = static_cast< Task* >( child( i ) );
        if ( task->isRunning() )
            task->setRunning( false, storage );
        task->remove( storage );
    }

    changeParentTotalTimes( -sessionTime(), -time());
    mRemoving = false;
    return ok;
}

void Task::updateActiveIcon()
{
    mCurrentPic = (mCurrentPic+1) % 8;
    setIcon(1, *(*icons)[mCurrentPic]);
}

QString Task::fullName() const
{
    if (isRoot())
        return name();
    else
        return parent()->fullName() + QString::fromLatin1("/") + name();
}

KCalCore::Todo::Ptr Task::asTodo(KCalCore::Todo::Ptr &todo) const
{
    kDebug(5970) <<"Task::asTodo: name() = '" << name() <<"'";

    if (todo != NULL)
      todo = taskTodo;
    return taskTodo;
}

bool Task::parseIncidence( const KCalCore::Incidence::Ptr &incident,
    long& sessionMinutes)
{
    kDebug(5970) << "Entering function";
    bool ok;
    ok = false;

    // if a KDE-karm-duration exists and not KDE-ktimetracker-duration, change this
    if (
        incident->customProperty( KGlobal::mainComponent().componentName().toUtf8(),
        QByteArray( "totalTaskTime" )) == QString::null && incident->customProperty( "karm",
        QByteArray( "totalTaskTime" )) != QString::null )
            incident->setCustomProperty(
                KGlobal::mainComponent().componentName().toUtf8(),
                QByteArray( "totalTaskTime" ), incident->customProperty( "karm",
                QByteArray( "totalTaskTime" )));

    incident->customProperty( KGlobal::mainComponent().componentName().toUtf8(),
                QByteArray( "totalTaskTime" )).toInt(&ok);
    if ( !ok )
    incident->setCustomProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray( "totalTaskTime" ), "0");
    ok = false;

    // if a KDE-karm-totalSessionTime exists and not KDE-ktimetracker-totalSessionTime, change this
    if (
        incident->customProperty( KGlobal::mainComponent().componentName().toUtf8(),
        QByteArray( "totalSessionTime" )) == QString::null && incident->customProperty( "karm",
        QByteArray( "totalSessionTime" )) != QString::null )
            incident->setCustomProperty(
                KGlobal::mainComponent().componentName().toUtf8(),
                QByteArray( "totalSessionTime" ), incident->customProperty( "karm",
                QByteArray( "totalSessionTime" )));
    sessionMinutes = incident->customProperty( KGlobal::mainComponent().componentName().toUtf8(),
        QByteArray( "totalSessionTime" )).toInt( &ok );
    if ( !ok )
        sessionMinutes = 0;

    // if a KDE-karm-deskTopList exists and no KDE-ktimetracker-DeskTopList, change this
    if (
        incident->customProperty( KGlobal::mainComponent().componentName().toUtf8(),
        QByteArray( "desktopList" )) == QString::null && incident->customProperty( "karm",
        QByteArray( "desktopList" )) != QString::null )
            incident->setCustomProperty(
                KGlobal::mainComponent().componentName().toUtf8(),
                QByteArray( "desktopList" ), incident->customProperty( "karm",
                QByteArray( "desktopList" )));

    return true;
}

QString Task::getDesktopStr() const
{
    if ( desktops().empty() )
        return QString();

    return taskTodo->customProperty( KGlobal::mainComponent().componentName().toUtf8(),
            QByteArray( "desktopList" ));
}

void Task::cut()
// This is needed e.g. to move a task under its parent when loading.
{
    kDebug(5970) << "Entering function";
    changeParentTotalTimes( -mTotalSessionTime, -mTotalTime);
    if ( ! parent() )
        treeWidget()->takeTopLevelItem(treeWidget()->indexOfTopLevelItem(this));
    else
        parent()->takeChild(indexOfChild(this));
    kDebug(5970) << "Leaving function";
}

void Task::paste(Task* destination)
// This is needed e.g. to move a task under its parent when loading.
{
    kDebug(5970) << "Entering function";
    destination->QTreeWidgetItem::insertChild(0,this);
    changeParentTotalTimes( mTotalSessionTime, mTotalTime);
    kDebug(5970) << "Leaving function";
}

void Task::move(Task* destination)
// This is used e.g. to move each task under its parent after loading.
{
    kDebug(5970) << "Entering function";
    cut();
    paste(destination);
    kDebug(5970) << "Leaving function";
}

void Task::update()
// Update a row, containing one task
{
    kDebug( 5970 ) << "Entering function";
    bool b = KTimeTrackerSettings::decimalFormat();
    setText( 0, taskTodo->summary().trimmed() );
    setText( 1, formatTime( sessionTime(), b ) );
    setText( 2, formatTime( time(), b ) );
    setText( 3, formatTime( mTotalSessionTime, b ) );
    setText( 4, formatTime( mTotalTime, b ) );
    setText( 5, priority() > 0 ? QString::number(priority()) : "--" );
    setText( 6, QString::number(percentComplete()) );
    kDebug( 5970 ) << "Leaving function";
}

void Task::addComment( const QString &comment, timetrackerstorage* storage )
{
    taskTodo->setDescription(taskTodo->description() + QString::fromLatin1("\n") + comment);
    storage->addComment(this, comment);
}

void Task::startNewSession()
{
    changeTimes( -sessionTime(), 0 );
    taskTodo->setCustomProperty(
      KGlobal::mainComponent().componentName().toUtf8(),
      QByteArray("sessionStartTiMe"),
      KDateTime::currentLocalDateTime().toString()
    );
}

//BEGIN Properties
QString Task::uid() const
{
    return taskTodo->uid();
}

QString Task::comment() const
{
    return taskTodo->description();
}

int Task::percentComplete() const
{
    return taskTodo->percentComplete();
}

int Task::priority() const
{
    return taskTodo->priority();
}

QString Task::name() const
{
    return taskTodo->summary().trimmed();
}

QString Task::description() const
{
    return taskTodo->description();
}

QDateTime Task::startTime() const
{
    return mLastStart;
}

KDateTime Task::sessionStartTiMe() const
{
    return KDateTime::fromString(taskTodo->customProperty(KGlobal::mainComponent().componentName().toUtf8(), QByteArray("sessionStartTiMe")));
}

DesktopList Task::desktops() const
{
    QString desktopList = taskTodo->customProperty( KGlobal::mainComponent().componentName().toUtf8(),
        QByteArray( "desktopList" ) );
    QStringList desktopStrList = desktopList.split( QString::fromLatin1(","),
        QString::SkipEmptyParts );
    DesktopList mDesktops;

    for ( QStringList::iterator iter = desktopStrList.begin();
        iter != desktopStrList.end();
        ++iter )
    {
        bool ok;
        int desktopInt = (*iter).toInt( &ok );
        if ( ok )
        {
            mDesktops.push_back( desktopInt );
        }
    }
    return mDesktops;
}
//END

#include "task.moc"
