/**
 * Copyright (c) 2004 David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "jobscheduler.h"
#include "kmfolder.h"
#include "folderstorage.h"
#include "kmfoldermgr.h"
#include <kdebug.h>

using namespace KMail;

JobScheduler::JobScheduler( QObject* parent, const char* name )
  : QObject( parent, name ), mTimer( this ),
    mCurrentTask( 0 ), mCurrentJob( 0 ), mIgnoreOpenNotify( false )
{
  connect( &mTimer, SIGNAL( timeout() ), SLOT( slotRunNextJob() ) );
  // No need to start the internal timer yet, we wait for a task to be scheduled
}


JobScheduler::~JobScheduler()
{
  // delete tasks in tasklist (no autodelete for QValueList)
  for( QValueList<ScheduledTask *>::Iterator it = mTaskList.begin(); it != mTaskList.end(); ++it ) {
    delete (*it);
  }
  delete mCurrentTask;
  delete mCurrentJob;
}

void JobScheduler::registerTask( ScheduledTask* task )
{
  int typeId = task->taskTypeId();
  if ( typeId ) {
    KMFolder* folder = task->folder();
    // Search for an identical task already scheduled
    for( QValueList<ScheduledTask *>::ConstIterator it = mTaskList.begin(); it != mTaskList.end(); ++it ) {
      if ( (*it)->taskTypeId() == typeId && (*it)->folder() == folder ) {
#ifdef DEBUG_SCHEDULER
        kdDebug(5006) << "JobScheduler: already having task type " << typeId << " for folder " << folder->label() << endl;
#endif
        delete task;
        return;
      }
    }
    // Note that scheduling an identical task as the one currently running is allowed.
  }
#ifdef DEBUG_SCHEDULER
  kdDebug(5006) << "JobScheduler: adding task " << task << " for folder " << task->folder() << " " << task->folder()->label() << endl;
#endif
  mTaskList.append( task );
  if ( !mTimer.isActive() )
    restartTimer();
}

void JobScheduler::notifyOpeningFolder( KMFolder* folder )
{
  if ( mCurrentTask && mCurrentTask->folder() == folder ) {
    if ( mIgnoreOpenNotify ) { // set when starting a job for this folder
#ifdef DEBUG_SCHEDULER
      kdDebug(5006) << "JobScheduler: got the opening-notification for " << folder->label() << " as expected." << endl;
#endif
      mIgnoreOpenNotify = false;
    } else
      interruptCurrentTask();
  }
}

void JobScheduler::interruptCurrentTask()
{
  Q_ASSERT( mCurrentTask );
#ifdef DEBUG_SCHEDULER
  kdDebug(5006) << "JobScheduler: interrupting job " << mCurrentJob << " for folder " << mCurrentTask->folder()->label() << endl;
#endif
  // File it again. This will either delete it or put it in mTaskList.
  registerTask( mCurrentTask );
  mCurrentTask = 0;
  mCurrentJob->kill(); // This deletes the job and calls slotJobFinished!
}

void JobScheduler::slotRunNextJob()
{
#ifdef DEBUG_SCHEDULER
  kdDebug(5006) << "JobScheduler: slotRunNextJob" << endl;
#endif
  Q_ASSERT( mCurrentTask == 0 );
  ScheduledTask* task = 0;
  // Find a task suitable for being run
  for( QValueList<ScheduledTask *>::Iterator it = mTaskList.begin(); it != mTaskList.end(); ++it ) {
    // Remove if folder died
    if ( (*it)->folder() == 0 ) {
#ifdef DEBUG_SCHEDULER
      kdDebug(5006) << "   folder for task " << (*it) << " was deleted" << endl;
#endif
      mTaskList.remove( it );
      if ( !mTaskList.isEmpty() )
        slotRunNextJob(); // to avoid the mess with invalid iterators :)
      else
        mTimer.stop();
      return;
    }
    // The condition is that the folder must be unused (not open)
    // But first we ask search folders to release their access to it
    kmkernel->searchFolderMgr()->tryReleasingFolder( (*it)->folder() );
#ifdef DEBUG_SCHEDULER
    kdDebug(5006) << "   looking at folder " << (*it)->folder()->label()
                  << " " << (*it)->folder()->location()
                  << "  isOpened=" << (*it)->folder()->isOpened() << endl;
#endif
    if ( !(*it)->folder()->isOpened() ) {
      task = *it;
      mTaskList.remove( it );
      break;
    }
  }

  if ( !task ) // found nothing to run, i.e. folder was opened
    return; // Timer keeps running, i.e. try again in 1 minute

  runTaskNow( task );

  if ( !mCurrentJob ) // nothing to do for that task
    slotRunNextJob(); // find another one
}

void JobScheduler::restartTimer()
{
#ifdef DEBUG_SCHEDULER
  mTimer.start( 1000 ); // 1 second
#else
  mTimer.start( 1 * 60000 ); // 1 minute
#endif
}

void JobScheduler::runTaskNow( ScheduledTask* task )
{
  if ( mCurrentTask ) {
    interruptCurrentTask();
  }
  mCurrentTask = task;
  mTimer.stop();
  mCurrentJob = mCurrentTask->run();
#ifdef DEBUG_SCHEDULER
  kdDebug(5006) << "JobScheduler: task " << mCurrentTask << " for folder " << mCurrentTask->folder()->label() << " returned job " << mCurrentJob << endl;
#endif
  if ( !mCurrentJob ) { // nothing to do, e.g. folder deleted
    delete mCurrentTask;
    mCurrentTask = 0;
    return;
  }
  // Register the job in the folder. This makes it autodeleted if the folder is deleted.
  mCurrentTask->folder()->storage()->addJob( mCurrentJob );
  connect( mCurrentJob, SIGNAL( finished() ), this, SLOT( slotJobFinished() ) );
  mIgnoreOpenNotify = true; // Obviously we'll have to open the folder ourselves
  mCurrentJob->start();
  mIgnoreOpenNotify = false;
}

void JobScheduler::slotJobFinished()
{
  // Do we need to test for mCurrentJob->error()? What do we do then?
#ifdef DEBUG_SCHEDULER
  kdDebug(5006) << "JobScheduler: slotJobFinished" << endl;
#endif
  delete mCurrentTask;
  mCurrentTask = 0;
  mCurrentJob = 0;
  if ( !mTaskList.isEmpty() )
    restartTimer();
}

#include "jobscheduler.moc"
