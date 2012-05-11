/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ARCHIVEJOB_H
#define ARCHIVEJOB_H


#include <mailcommon/jobscheduler.h>
#include <Akonadi/Collection>

class ArchiveMailInfo;

class ArchiveJob : public MailCommon::ScheduledJob
{
  Q_OBJECT
public:
  explicit ArchiveJob(ArchiveMailInfo *info, const Akonadi::Collection &folder, bool immediate);
  virtual ~ArchiveJob();

  virtual void execute();
  virtual void kill();
protected Q_SLOT:
  void slotBackupDone();
private:
  ArchiveMailInfo *mInfo;
};

/// A scheduled "expire mails in this folder" task.
class ScheduledArchiveTask : public MailCommon::ScheduledTask
{
  public:
    /// If immediate is set, the job will execute synchronously. This is used when
    /// the user requests explicitly that the operation should happen immediately.
    ScheduledArchiveTask( ArchiveMailInfo * info, const Akonadi::Collection &folder, bool immediate )
      : MailCommon::ScheduledTask( folder, immediate ), mInfo(info)
    {
    }

    virtual ~ScheduledArchiveTask()
    {
    }

    virtual MailCommon::ScheduledJob *run();

    virtual int taskTypeId() const
    {
      return 2;
    }
private:
    ArchiveMailInfo *mInfo;
};


#endif // ARCHIVEJOB_H
