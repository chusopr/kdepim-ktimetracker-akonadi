/*
 *   This file only:
 *     Copyright (C) 2003, 2004  Mark Bucciarelli <mark@hubcapconsulting.com>
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
 *      59 Temple Place - Suite 330
 *      Boston, MA  02111-1307  USA.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cassert>

#include <qfile.h>
#include <qsize.h>
#include <qdict.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "incidence.h"
#include "kapplication.h"       // kapp
#include <kdebug.h>
#include <kemailsettings.h>
#include <klocale.h>            // i18n
#include <kmessagebox.h>
#include <kprogress.h>
#include <ktempfile.h>
#include <resourcecalendar.h>
#include <resourcelocal.h>
#include <kpimprefs.h>
#include <taskview.h>
#include <timekard.h>
#include <karmutility.h>
#include <kio/netaccess.h>
#include <vector>

//#include <calendarlocal.h>
//#include <journal.h>
//#include <event.h>
//#include <todo.h>

#include "karmstorage.h"
#include "preferences.h"
#include "task.h"
#include "reportcriteria.h"

using namespace std;

KarmStorage *KarmStorage::_instance = 0;    
static long linenr;  // how many lines written by printTaskHistory so far


KarmStorage *KarmStorage::instance()
{
  if (_instance == 0) _instance = new KarmStorage();
  return _instance;
}

KarmStorage::KarmStorage()
{
  _calendar = 0;
}

QString KarmStorage::load (TaskView* view, const Preferences* preferences)
{
  // When I tried raising an exception from this method, the compiler
  // complained that exceptions are not allowed.  Not sure how apps
  // typically handle error conditions in KDE, but I'll return the error
  // as a string (empty is no error).  -- Mark, Aug 8, 2003

  // Use KDE_CXXFLAGS=$(USE_EXCEPTIONS) in Makefile.am if you want to use
  // exceptions (David Faure)

  QString err;
  KEMailSettings settings;

  // If same file, don't reload
  if ( preferences->iCalFile() == _icalfile ) return err;


  // If file doesn't exist, create a blank one to avoid ResourceLocal load
  // error.  We make it user and group read/write, others read.  This is
  // masked by the users umask.  (See man creat)
  int handle;
  handle = open (
      QFile::encodeName( preferences->iCalFile() ),
      O_CREAT|O_EXCL|O_WRONLY,
      S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH
      );
  if (handle != -1) close(handle);

  if ( _calendar) closeStorage(view);
  else _calendar = new KCal::CalendarResources();

  // Create local file resource and add to resources
  _icalfile = preferences->iCalFile();
  KCal::ResourceCalendar *l = new KCal::ResourceLocal( _icalfile );
  QObject::connect (l, SIGNAL(resourceChanged(ResourceCalendar *)),
  	            view, SLOT(iCalFileModified(ResourceCalendar *)));
  l->setTimeZoneId( KPimPrefs::timezone() );
  l->setResourceName( QString::fromLatin1("KArm") );
  l->open();
  l->load();

  KCal::CalendarResourceManager *m = _calendar->resourceManager();
  m->add(l);
  m->setStandardResource(l);

  // Claim ownership of iCalendar file if no one else has.
  KCal::Person owner = _calendar->getOwner();
  if ( owner.isEmpty() )
  {
    _calendar->setOwner( KCal::Person( 
          settings.getSetting( KEMailSettings::RealName ), 
          settings.getSetting( KEMailSettings::EmailAddress ) ) );
  }

  // Build task view from iCal data
  if (!err)
  {
    KCal::Todo::List todoList;
    KCal::Todo::List::ConstIterator todo;
    QDict< Task > map;

    // Build dictionary to look up Task object from Todo uid.  Each task is a
    // QListViewItem, and is initially added with the view as the parent.
    todoList = _calendar->rawTodos();
    kdDebug(5970) << "KarmStorage::load "
      << "rawTodo count (includes completed todos) ="
      << todoList.count() << endl;
    for( todo = todoList.begin(); todo != todoList.end(); ++todo )
    {
      // Initially, if a task was complete, it was removed from the view.
      // However, this increased the complexity of reporting on task history.
      //
      // For example, if a task is complete yet has time logged to it during
      // the date range specified on the history report, we have to figure out
      // how that task fits into the task hierarchy.  Currently, this
      // structure is held in memory by the structure in the list view.
      //
      // I considered creating a second tree that held the full structure of
      // all complete and incomplete tasks.  But this seemed to much of a
      // change with an impending beta release and a full todo list.
      //
      // Hence this "solution".  Include completed tasks, but mark them as
      // inactive in the view.
      //
      //if ((*todo)->isCompleted()) continue;

      Task* task = new Task(*todo, view);
      map.insert( (*todo)->uid(), task );
      view->setRootIsDecorated(true);
      if ((*todo)->isCompleted())
      {
        task->setEnabled(false);
        task->setOpen(false);
      }
      else
        task->setOpen(true);
    }

    // Load each task under it's parent task.
    for( todo = todoList.begin(); todo != todoList.end(); ++todo )
    {
      Task* task = map.find( (*todo)->uid() );

      // No relatedTo incident just means this is a top-level task.
      if ( (*todo)->relatedTo() )
      {
        Task* newParent = map.find( (*todo)->relatedToUid() );

        // Complete the loading but return a message
        if ( !newParent )
          err = i18n("Error loading \"%1\": could not find parent (uid=%2)")
            .arg(task->name())
            .arg((*todo)->relatedToUid());

        if (!err) task->move( newParent);
      }
    }

    kdDebug(5970) << "KarmStorage::load - loaded " << view->count()
      << " tasks from " << _icalfile << endl;
  }

  return err;
}
 
QString KarmStorage::buildTaskView(KCal::ResourceCalendar *rc, TaskView *view)
// makes *view contain the tasks out of *rc.
{
  QString err;
  KCal::Todo::List todoList;
  KCal::Todo::List::ConstIterator todo;
  QDict< Task > map;
  
  // 1. delete old tasks
  while (view->item_at_index(0)) view->item_at_index(0)->cut();
  
  // 1. insert tasks form rc into taskview
  // 1.1. Build dictionary to look up Task object from Todo uid.  Each task is a
  // QListViewItem, and is initially added with the view as the parent.
  todoList = rc->rawTodos();
  for( todo = todoList.begin(); todo != todoList.end(); ++todo )
  {
    Task* task = new Task(*todo, view);
    map.insert( (*todo)->uid(), task );
    view->setRootIsDecorated(true);
    if ((*todo)->isCompleted())
    {
      task->setEnabled(false);
      task->setOpen(false);
    }
    else
      task->setOpen(true);
  }

  // 1.1. Load each task under it's parent task.
  for( todo = todoList.begin(); todo != todoList.end(); ++todo )
  {
    Task* task = map.find( (*todo)->uid() );

    // No relatedTo incident just means this is a top-level task.
    if ( (*todo)->relatedTo() )
    {
      Task* newParent = map.find( (*todo)->relatedToUid() );

      // Complete the loading but return a message
      if ( !newParent )
        err = i18n("Error loading \"%1\": could not find parent (uid=%2)")
          .arg(task->name())
          .arg((*todo)->relatedToUid());

      if (!err) task->move( newParent);
    }
  }
  return err;
}

void KarmStorage::closeStorage(TaskView* view)
{
  if ( _calendar )
  {

    _calendar->close();

    KCal::CalendarResourceManager *m = _calendar->resourceManager();
    m->remove( m->standardResource() );

    view->clear();
  }
}

QString KarmStorage::save(TaskView* taskview)
{
  QString err="";

  QPtrStack< KCal::Todo > parents;

  for (Task* task=taskview->first_child(); task; task = task->nextSibling())
  {
    writeTaskAsTodo(task, 1, parents );
  }

  if (!_calendar->save(_calendar->requestSaveTicket
    ( _calendar->resourceManager()->standardResource() ))) err="Could not save";

  kdDebug(5970)
    << "KarmStorage::save : wrote "
    << taskview->count() << " tasks to " << _icalfile << endl;
  return err;
}

void KarmStorage::writeTaskAsTodo(Task* task, const int level,
    QPtrStack< KCal::Todo >& parents )
{
  KCal::Todo* todo;

  todo = _calendar->todo(task->uid());
  task->asTodo(todo);
  if ( !parents.isEmpty() ) todo->setRelatedTo( parents.top() );
  parents.push( todo );

  for (Task* nextTask = task->firstChild(); nextTask;
      nextTask = nextTask->nextSibling() )
  {
    writeTaskAsTodo(nextTask, level+1, parents );
  }

  parents.pop();
}

bool KarmStorage::isEmpty()
{
  KCal::Todo::List todoList;

  todoList = _calendar->rawTodos();
  return todoList.empty();
}

bool KarmStorage::isNewStorage(const Preferences* preferences) const
{
  if ( !_icalfile.isNull() ) return preferences->iCalFile() != _icalfile;
  else return false;
}

//----------------------------------------------------------------------------
// Routines that handle legacy flat file format.
// These only stored total and session times.
//

QString KarmStorage::loadFromFlatFile(TaskView* taskview,
    const QString& filename)
{
  QString err;

  kdDebug(5970)
    << "KarmStorage::loadFromFlatFile: " << filename << endl;

  QFile f(filename);
  if( !f.exists() )
    err = i18n("File \"%1\" not found.").arg(filename);

  if (!err)
  {
    if( !f.open( IO_ReadOnly ) )
      err = i18n("Could not open \"%1\".").arg(filename);
  }

  if (!err)
  {

    QString line;

    QPtrStack<Task> stack;
    Task *task;

    QTextStream stream(&f);

    while( !stream.atEnd() ) {
      // lukas: this breaks for non-latin1 chars!!!
      // if ( file.readLine( line, T_LINESIZE ) == 0 )
      //   break;

      line = stream.readLine();
      kdDebug(5970) << "DEBUG: line: " << line << "\n";

      if (line.isNull())
        break;

      long minutes;
      int level;
      QString name;
      DesktopList desktopList;
      if (!parseLine(line, &minutes, &name, &level, &desktopList))
        continue;

      unsigned int stackLevel = stack.count();
      for (unsigned int i = level; i<=stackLevel ; i++) {
        stack.pop();
      }

      if (level == 1) {
        kdDebug(5970) << "KarmStorage::loadFromFlatFile - toplevel task: "
          << name << " min: " << minutes << "\n";
        task = new Task(name, minutes, 0, desktopList, taskview);
        task->setUid(addTask(task, 0));
      }
      else {
        Task *parent = stack.top();
        kdDebug(5970) << "KarmStorage::loadFromFlatFile - task: " << name
            << " min: " << minutes << " parent" << parent->name() << "\n";
        task = new Task(name, minutes, 0, desktopList, parent);

        task->setUid(addTask(task, parent));

        // Legacy File Format (!):
        parent->changeTimes(0, -minutes);
        taskview->setRootIsDecorated(true);
        parent->setOpen(true);
      }
      if (!task->uid().isNull())
        stack.push(task);
      else
        delete task;
    }

    f.close();

  }

  return err;
}

QString KarmStorage::loadFromFlatFileCumulative(TaskView* taskview,
    const QString& filename)
{
  QString err = loadFromFlatFile(taskview, filename);
  if (!err)
  {
    for (Task* task = taskview->first_child(); task;
        task = task->nextSibling())
    {
      adjustFromLegacyFileFormat(task);
    }
  }
  return err;
}

bool KarmStorage::parseLine(QString line, long *time, QString *name,
    int *level, DesktopList* desktopList)
{
  if (line.find('#') == 0) {
    // A comment line
    return false;
  }

  int index = line.find('\t');
  if (index == -1) {
    // This doesn't seem like a valid record
    return false;
  }

  QString levelStr = line.left(index);
  QString rest = line.remove(0,index+1);

  index = rest.find('\t');
  if (index == -1) {
    // This doesn't seem like a valid record
    return false;
  }

  QString timeStr = rest.left(index);
  rest = rest.remove(0,index+1);

  bool ok;

  index = rest.find('\t'); // check for optional desktops string
  if (index >= 0) {
    *name = rest.left(index);
    QString deskLine = rest.remove(0,index+1);

    // now transform the ds string (e.g. "3", or "1,4,5") into
    // an DesktopList
    QString ds;
    int d;
    int commaIdx = deskLine.find(',');
    while (commaIdx >= 0) {
      ds = deskLine.left(commaIdx);
      d = ds.toInt(&ok);
      if (!ok)
        return false;

      desktopList->push_back(d);
      deskLine.remove(0,commaIdx+1);
      commaIdx = deskLine.find(',');
    }

    d = deskLine.toInt(&ok);

    if (!ok)
      return false;

    desktopList->push_back(d);
  }
  else {
    *name = rest.remove(0,index+1);
  }

  *time = timeStr.toLong(&ok);

  if (!ok) {
    // the time field was not a number
    return false;
  }
  *level = levelStr.toInt(&ok);
  if (!ok) {
    // the time field was not a number
    return false;
  }

  return true;
}

void KarmStorage::adjustFromLegacyFileFormat(Task* task)
{
  // unless the parent is the listView
  if ( task->parent() )
    task->parent()->changeTimes(-task->sessionTime(), -task->time());

  // traverse depth first -
  // as soon as we're in a leaf, we'll substract it's time from the parent
  // then, while descending back we'll do the same for each node untill
  // we reach the root
  for ( Task* subtask = task->firstChild(); subtask;
      subtask = subtask->nextSibling() )
    adjustFromLegacyFileFormat(subtask);
}

//----------------------------------------------------------------------------
// Routines that handle Comma-Separated Values export file format.
//
QString KarmStorage::exportcsvFile( TaskView *taskview, 
                                    const ReportCriteria &rc )
{
  QString delim = rc.delimiter;
  QString dquote = rc.quote;
  QString double_dquote = dquote + dquote;
  bool to_quote = true;
  
  QString err;
  Task* task;
  int maxdepth=0; 

  kdDebug(5970)
    << "KarmStorage::exportcsvFile: " << rc.url << endl;

  QString title = i18n("Export Progress");
  KProgressDialog dialog( taskview, 0, title );
  dialog.setAutoClose( true );
  dialog.setAllowCancel( true );
  dialog.progressBar()->setTotalSteps( 2 * taskview->count() );

  // The default dialog was not displaying all the text in the title bar.
  int width = taskview->fontMetrics().width(title) * 3;
  QSize dialogsize;
  dialogsize.setWidth(width);
  dialog.setInitialSize( dialogsize, true );
   
  if ( taskview->count() > 1 ) dialog.show();
    
  QString retval;

  // Find max task depth
  int tasknr = 0;
  while ( tasknr < taskview->count() && !dialog.wasCancelled() )
  { 
    dialog.progressBar()->advance( 1 );
    if ( tasknr % 15 == 0 ) kapp->processEvents(); // repainting is slow 
    if ( taskview->item_at_index(tasknr)->depth() > maxdepth ) 
      maxdepth = taskview->item_at_index(tasknr)->depth(); 
    tasknr++;
  } 

  // Export to file
  tasknr = 0;
  while ( tasknr < taskview->count() && !dialog.wasCancelled() )
  {
    task = taskview->item_at_index( tasknr );
    dialog.progressBar()->advance( 1 );
    if ( tasknr % 15 == 0 ) kapp->processEvents();

    // indent the task in the csv-file:
    for ( int i=0; i < task->depth(); ++i ) retval += delim;
      
    /*
    // CSV compliance
    // Surround the field with quotes if the field contains 
    // a comma (delim) or a double quote
    if (task->name().contains(delim) || task->name().contains(dquote))
      to_quote = TRUE;
    else
      to_quote = FALSE;
    */
    to_quote = true;
      
    if (to_quote)
      retval += dquote;
        
    // Double quotes replaced by a pair of consecutive double quotes 
    retval += task->name().replace( dquote, double_dquote );
      
    if (to_quote)
      retval += dquote;
      
    // maybe other tasks are more indented, so to align the columns:
    for ( int i = 0; i < maxdepth - task->depth(); ++i ) retval += delim;
      
    retval += delim + formatTime( task->sessionTime(),
                                   rc.decimalMinutes )
           + delim + formatTime( task->time(),
                                   rc.decimalMinutes )
           + delim + formatTime( task->totalSessionTime(),
                                   rc.decimalMinutes )
           + delim + formatTime( task->totalTime(),
                                   rc.decimalMinutes )
           + "\n";
    tasknr++;
  }
  
  // save, either locally or remote
  if ((rc.url.isLocalFile()) or (!rc.url.url().contains("/")))
  {    
    QString filename=rc.url.path();
    if (filename.isEmpty()) filename=rc.url.url();
    QFile f( filename );
    if( !f.open( IO_WriteOnly ) ) {
        err = i18n( "Could not open \"%1\"." ).arg( filename );
    }
    if (!err)
    {
      QTextStream stream(&f);
      // Export to file
      stream << retval;
      f.close();
    }
  }
  else // use remote file
  {
    KTempFile tmpFile;
    if ( tmpFile.status() != 0 ) err = QString::fromLatin1( "Unable to get temporary file" ); 
    else
    {
      QTextStream *stream=tmpFile.textStream();
      *stream << retval;
      tmpFile.close(); 
      if (!KIO::NetAccess::upload( tmpFile.name(), rc.url, 0 )) err=QString::fromLatin1("Could not upload");
    }
  }
  
  return err;
}

//----------------------------------------------------------------------------
// Routines that handle logging KArm history
//

//
// public routines:
//

QString KarmStorage::addTask(const Task* task, const Task* parent)
{
  KCal::Todo* todo;
  QString uid;

  todo = new KCal::Todo();
  if ( _calendar->addTodo( todo ) )
  {
    task->asTodo( todo  );
    if (parent)
      todo->setRelatedTo(_calendar->todo(parent->uid()));
    uid = todo->uid();
  }

  return uid;
}

bool KarmStorage::removeTask(Task* task)
{

  // delete history
  KCal::Event::List eventList = _calendar->rawEvents();
  for(KCal::Event::List::iterator i = eventList.begin();
      i != eventList.end();
      ++i)
  {
    //kdDebug(5970) << "KarmStorage::removeTask: "
    //  << (*i)->uid() << " - relatedToUid() "
    //  << (*i)->relatedToUid()
    //  << ", relatedTo() = " << (*i)->relatedTo() <<endl;
    if ( (*i)->relatedToUid() == task->uid()
        || ( (*i)->relatedTo()
            && (*i)->relatedTo()->uid() == task->uid()))
    {
      _calendar->deleteEvent(*i);
    }
  }

  // delete todo
  KCal::Todo *todo = _calendar->todo(task->uid());
  _calendar->deleteTodo(todo);

  // Save entire file
  _calendar->save(_calendar->requestSaveTicket
    (_calendar->resourceManager()->standardResource()));

  return true;
}

void KarmStorage::addComment(const Task* task, const QString& comment)
{

  KCal::Todo* todo;

  todo = _calendar->todo(task->uid());

  // Do this to avoid compiler warnings about comment not being used.  once we
  // transition to using the addComment method, we need this second param.
  QString s = comment;

  // TODO: Use libkcal comments 
  // todo->addComment(comment);
  // temporary
  todo->setDescription(task->comment());

  _calendar->save(_calendar->requestSaveTicket
    ( _calendar->resourceManager()->standardResource() ));
}

long KarmStorage::printTaskHistory (
        const Task               *task, 
        const QMap<QString,long> &taskdaytotals, 
        QMap<QString,long>       &daytotals, 
        const QDate              &from,
        const QDate              &to, 
        const int                level,
	vector <QString>         &matrix,
        const ReportCriteria     &rc)
// to>=from is precondition
{
  long ownline=linenr++; // the how many-th instance of this function is this 
  long colrectot=0;      // colum where to write the task's total recursive time
  vector <QString> cell; // each line of the matrix is stored in an array of cells, one containing the recursive total
  long add;              // total recursive time of all subtasks
  QString delim = rc.delimiter;
  QString dquote = rc.quote;
  QString double_dquote = dquote + dquote;
  bool to_quote = true;

  const QString cr = QString::fromLatin1("\n");
  QString buf;
  QString daytaskkey, daykey;
  QDate day;
  long sum;

  if ( !task ) return 0;

  day = from;
  sum = 0;
  while (day <= to)
  {
    // write the time in seconds for the given task for the given day to s
    daykey = day.toString(QString::fromLatin1("yyyyMMdd"));
    daytaskkey = QString::fromLatin1("%1_%2")
      .arg(daykey)
      .arg(task->uid());

    if (taskdaytotals.contains(daytaskkey))
    {
      cell.push_back(QString::fromLatin1("%1")
        .arg(formatTime(taskdaytotals[daytaskkey]/60, rc.decimalMinutes)));
      sum += taskdaytotals[daytaskkey];  // in seconds

      if (daytotals.contains(daykey))
        daytotals.replace(daykey, daytotals[daykey]+taskdaytotals[daytaskkey]);
      else
        daytotals.insert(daykey, taskdaytotals[daytaskkey]);
    }
    cell.push_back(delim);

    day = day.addDays(1);
  }

  // Total for task 
  cell.push_back(QString::fromLatin1("%1").arg(formatTime(sum/60, rc.decimalMinutes)));
  
  // room for the recursive total time (that cannot be calculated now)
  cell.push_back(delim);
  colrectot = cell.size();
  cell.push_back("???");
  cell.push_back(delim);
  
  // Task name
  for ( int i = level + 1; i > 0; i-- ) cell.push_back(delim);

  /*
  // CSV compliance
  // Surround the field with quotes if the field contains 
  // a comma (delim) or a double quote
  to_quote = task->name().contains(delim) || task->name().contains(dquote);
  */
  to_quote = true; 
  if ( to_quote) cell.push_back(dquote);


  // Double quotes replaced by a pair of consecutive double quotes 
  cell.push_back(task->name().replace( dquote, double_dquote ));

  if ( to_quote) cell.push_back(dquote);

  cell.push_back(cr);

  add=0;
  for (Task* subTask = task->firstChild();
      subTask;
      subTask = subTask->nextSibling())
  {
    add += printTaskHistory( subTask, taskdaytotals, daytotals, from, to , level+1, matrix,
                      rc );
  }
  cell[colrectot]=(QString::fromLatin1("%1").arg(formatTime((add+sum)/60, rc.decimalMinutes )));
  for (unsigned int i=0; i < cell.size(); i++) matrix[ownline]+=cell[i];
  return add+sum;
}

QString KarmStorage::report( TaskView *taskview, const ReportCriteria &rc )
{
  QString err;
  if ( rc.reportType == ReportCriteria::CSVHistoryExport )
      err = exportcsvHistory( taskview, rc.from, rc.to, rc );
  else if ( rc.reportType == ReportCriteria::CSVTotalsExport )
      err = exportcsvFile( taskview, rc );
  else
      // hmmmm ... assert(0)?
      ;
  return err;
};

// export history report as csv, all tasks X all dates in one block
QString KarmStorage::exportcsvHistory ( TaskView      *taskview, 
                                            const QDate   &from, 
                                            const QDate   &to,
                                            const ReportCriteria &rc)
{
  QString delim = rc.delimiter;
  const QString cr = QString::fromLatin1("\n");
  QString err;
  
  // below taken from timekard.cpp
  QString retval;
  QString taskhdr, totalhdr;
  QString line, buf;
  long sum;
  
  QValueList<HistoryEvent> events;
  QValueList<HistoryEvent>::iterator event;
  QMap<QString, long> taskdaytotals;
  QMap<QString, long> daytotals;
  QString daytaskkey, daykey;
  QDate day;
  QDate dayheading;

  // parameter-plausi
  if ( from > to ) 
  {
    err = QString::fromLatin1 (
            "'to' has to be a date later than or equal to 'from'.");
  }
 
  // header
  retval += i18n("Task History\n");
  retval += i18n("From %1 to %2")
    .arg(KGlobal::locale()->formatDate(from))
    .arg(KGlobal::locale()->formatDate(to));
  retval += cr;
  retval += i18n("Printed on: %1")
    .arg(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()));
  retval += cr;

  day=from;
  events = taskview->getHistory(from, to);
  taskdaytotals.clear();
  daytotals.clear();
 
  // Build lookup dictionary used to output data in table cells.  keys are
  // in this format: YYYYMMDD_NNNNNN, where Y = year, M = month, d = day and
  // NNNNN = the VTODO uid.  The value is the total seconds logged against
  // that task on that day.  Note the UID is the todo id, not the event id,
  // so times are accumulated for each task.
  for (event = events.begin(); event != events.end(); ++event)
  {
    daykey = (*event).start().date().toString(QString::fromLatin1("yyyyMMdd"));
    daytaskkey = QString(QString::fromLatin1("%1_%2"))
        .arg(daykey)
        .arg((*event).todoUid());
        
    if (taskdaytotals.contains(daytaskkey))
        taskdaytotals.replace(daytaskkey, 
                taskdaytotals[daytaskkey] + (*event).duration());
    else
        taskdaytotals.insert(daytaskkey, (*event).duration());
  }
        
  // day headings
  dayheading = from;
  while ( dayheading <= to )
  {
    // Use ISO 8601 format for date.
    retval += dayheading.toString(QString::fromLatin1("yyyy-MM-dd"));
    retval += delim;
    dayheading=dayheading.addDays(1);
  }
  retval += i18n("Sum") + delim + i18n("Total Sum") + delim + i18n("Task Hierarchy");
  retval += cr;
  retval += line;
        
  // the tasks
  vector <QString> matrix;
  linenr=0;
  for (int i=0; i<=taskview->count()+1; i++) matrix.push_back("");
  if (events.empty())
  {
    retval += i18n("  No hours logged.");
  }
  else
  {
    if ( rc.allTasks ) 
    {
      for ( Task* task= taskview->item_at_index(0);
            task; task= task->nextSibling() )
      {
        printTaskHistory( task, taskdaytotals, daytotals, from, to, 0, 
                          matrix, rc );
      }
    }
    else
    {
      printTaskHistory( taskview->current_item(), taskdaytotals, daytotals, 
                        from, to, 0, matrix, rc );
    }
    for (unsigned int i=0; i<matrix.size(); i++) retval+=matrix[i];
    retval += line;
        
    // totals
    sum = 0;
    day = from;
    while (day<=to)
    {
      daykey = day.toString(QString::fromLatin1("yyyyMMdd"));
        
      if (daytotals.contains(daykey))
      {
        retval += QString::fromLatin1("%1")
            .arg(formatTime(daytotals[daykey]/60, rc.decimalMinutes));
        sum += daytotals[daykey];  // in seconds
      }
      retval += delim;
      day = day.addDays(1);
    }
        
    retval += QString::fromLatin1("%1%2%3%4")
        .arg( formatTime( sum/60, rc.decimalMinutes ) )
        .arg( delim ).arg( delim )
        .arg( i18n( "Total" ) );
  }

  // above taken from timekard.cpp
  
  // save, either locally or remote
  
  if ((rc.url.isLocalFile()) or (!rc.url.url().contains("/")))
  {    
    QString filename=rc.url.path();
    if (filename.isEmpty()) filename=rc.url.url();
    QFile f( filename );
    if( !f.open( IO_WriteOnly ) ) {
        err = i18n( "Could not open \"%1\"." ).arg( filename );
    }
    if (!err)
    {
      QTextStream stream(&f);
      // Export to file
      stream << retval;
      f.close();
    }
  }
  else // use remote file
  {
    KTempFile tmpFile;
    if ( tmpFile.status() != 0 ) err = QString::fromLatin1( "Unable to get temporary file" ); 
    else
    {
      QTextStream *stream=tmpFile.textStream();
      *stream << retval;
      tmpFile.close(); 
      if (!KIO::NetAccess::upload( tmpFile.name(), rc.url, 0 )) err=QString::fromLatin1("Could not upload");
    }
  }
  return err;
}

void KarmStorage::stopTimer(const Task* task)
{
  long delta = task->startTime().secsTo(QDateTime::currentDateTime());
  changeTime(task, delta);
}

void KarmStorage::changeTime(const Task* task, const long deltaSeconds)
{
  KCal::Event* e;
  QDateTime end;

  // Don't write events (with timer start/stop duration) if user has turned
  // this off in the settings dialog.
  if ( ! task->taskView()->preferences()->logging() ) return;

  e = baseEvent(task);

  // Don't use duration, as ICalFormatImpl::writeIncidence never writes a
  // duration, even though it looks like it's used in event.cpp.
  end = task->startTime();
  if ( deltaSeconds > 0 ) end = task->startTime().addSecs(deltaSeconds);
  e->setDtEnd(end);

  // Use a custom property to keep a record of negative durations
  e->setCustomProperty( kapp->instanceName(),
      QCString("duration"),
      QString::number(deltaSeconds));

  _calendar->addEvent(e);

  // This saves the entire iCal file each time, which isn't efficient but
  // ensures no data loss.  A faster implementation would be to append events
  // to a file, and then when KArm closes, append the data in this file to the
  // iCal file.
  //
  // Meanwhile, we simply use a timer to delay the full-saving until the GUI
  // has updated, for better user feedback. Feel free to get rid of this
  // if/when implementing the faster saving (DF).
  task->taskView()->scheduleSave();
}


KCal::Event* KarmStorage::baseEvent(const Task * task)
{
  KCal::Event* e;
  QStringList categories;

  e = new KCal::Event;
  e->setSummary(task->name());

  // Can't use setRelatedToUid()--no error, but no RelatedTo written to disk
  e->setRelatedTo(_calendar->todo(task->uid()));

  // Debugging: some events where not getting a related-to field written.
  assert(e->relatedTo()->uid() == task->uid());

  // Have to turn this off to get datetimes in date fields.
  e->setFloats(false);
  e->setDtStart(task->startTime());

  // So someone can filter this mess out of their calendar display
  categories.append(i18n("KArm"));
  e->setCategories(categories);

  return e;
}

HistoryEvent::HistoryEvent(QString uid, QString name, long duration,
        QDateTime start, QDateTime stop, QString todoUid)
{
  _uid = uid;
  _name = name;
  _duration = duration;
  _start = start;
  _stop = stop;
  _todoUid = todoUid;
}


QValueList<HistoryEvent> KarmStorage::getHistory(const QDate& from,
    const QDate& to)
{
  QValueList<HistoryEvent> retval;
  QStringList processed;
  KCal::Event::List events;
  KCal::Event::List::iterator event;
  QString duration;

  for(QDate d = from; d <= to; d = d.addDays(1))
  {
    events = _calendar->events(d);
    for (event = events.begin(); event != events.end(); ++event)
    {

      // KArm events have the custom property X-KDE-Karm-duration
      if (! processed.contains( (*event)->uid()))
      {
        // If an event spans multiple days, CalendarLocal::rawEventsForDate
        // will return the same event on both days.  To avoid double-counting
        // such events, we (arbitrarily) attribute the hours from both days on
        // the first day.  This mis-reports the actual time spent, but it is
        // an easy fix for a (hopefully) rare situation.
        processed.append( (*event)->uid());

        duration = (*event)->customProperty(kapp->instanceName(),
            QCString("duration"));
        if ( ! duration.isNull() )
        {
          if ( (*event)->relatedTo()
              &&  ! (*event)->relatedTo()->uid().isEmpty() )
          {
            retval.append(HistoryEvent(
                (*event)->uid(),
                (*event)->summary(),
                duration.toLong(),
                (*event)->dtStart(),
                (*event)->dtEnd(),
                (*event)->relatedTo()->uid()
                ));
          }
          else
            // Something is screwy with the ics file, as this KArm history event
            // does not have a todo related to it.  Could have been deleted
            // manually?  We'll continue with report on with report ...
            kdDebug(5970) << "KarmStorage::getHistory(): "
              << "The event " << (*event)->uid()
              << " is not related to a todo.  Dropped." << endl;
        }
      }
    }
  }

  return retval;
}

/*
 * Obsolete methods for writing to flat file format.
 * Aug 8, 2003, Mark
 *
void KarmStorage::saveToFileFormat()
{
  //QFile f(_preferences->saveFile());
  QFile f(_preferences->flatFile());

  if ( !f.open( IO_WriteOnly | IO_Truncate ) ) {
    QString msg = i18n( "There was an error trying to save your data file.\n"
                       "Time accumulated during this session will not be saved!\n");
    KMessageBox::error(0, msg );
    return;
  }
  const char * comment = "# TaskView save data\n";

  f.writeBlock(comment, strlen(comment));  //comment
  f.flush();

  QTextStream stream(&f);
  for (Task* child = firstChild();
             child;
             child = child->nextSibling())
    writeTaskToFile(&stream, child, 1);

  f.close();
  kdDebug(5970) << "Saved data to file " << f.name() << endl;
}
void KarmStorage::writeTaskToFile( QTextStream *strm, Task *task,
                                int level)
{
  //lukas: correct version for non-latin1 users
  QString _line = QString::fromLatin1("%1\t%2\t%3").arg(level).
          arg(task->time()).arg(task->name());

  DesktopList d = task->getDesktops();
  int dsize = d.size();
  if (dsize>0) {
    _line += '\t';
    for (int i=0; i<dsize-1; i++) {
      _line += QString::number(d[i]);
      _line += ',';
    }
    _line += QString::number(d[dsize-1]);
  }
  *strm << _line << "\n";

  for ( Task* child= task->firstChild();
              child;
              child=child->nextSibling()) {
    writeTaskToFile(strm, child, level+1);
  }
}

*/
