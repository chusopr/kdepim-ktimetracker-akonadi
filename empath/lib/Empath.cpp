/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
*/

#ifdef __GNUG__
# pragma implementation "Empath.h"
#endif

// System includes
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>

// Qt includes
#include <qcolor.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstddirs.h>

// Local includes
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathJobScheduler.h"
#include "EmpathJob.h"
#include "EmpathCachedMessage.h"
#include "EmpathComposer.h"
#include "EmpathFilterList.h"
#include "EmpathMailSender.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"
#include "EmpathFolder.h"
#include "EmpathTask.h"
#include "config.h"

#ifdef USE_QPTHREAD
#include <qpthr/qp.h>
#endif

Empath * Empath::EMPATH = 0;

    void
Empath::start()
{
    if (0 == EMPATH)
        EMPATH = new Empath;
}
        
    void
Empath::shutdown()
{
    delete this;
}

Empath::Empath()
    :   QObject((QObject *)0L, "Empath"),
        mailboxList_    (0L),
        filterList_     (0L),
        sender_         (0L),
        composer_       (0L),
        jobScheduler_   (0L),
        seq_            (0)
{
#ifdef USE_QPTHREAD
    (void)new QpInit;
#endif
    
    using namespace EmpathConfig;

    DFLT_Q_1   = new QColor(Qt::darkBlue);
    DFLT_Q_2   = new QColor(Qt::darkCyan);
    DFLT_LINK  = new QColor(Qt::blue);
    DFLT_NEW   = new QColor(Qt::darkRed);

    // Don't do dollar expansion by default.
    // Possible security hole.
    KGlobal::config()->setDollarExpansion(false);    

    KGlobal::dirs()->addResourceType("indices", "share/apps/empath/indices");
    KGlobal::dirs()->addResourceType("cache",   "share/apps/empath/cache");
}

    void
Empath::init()
{
    processID_ = int(getpid());
    pidStr_.setNum(processID_);
    
    _saveHostName();
    _setStartTime();

    cache_.setMaxCost(5);

    QString s(i18n("Local"));

    inbox_  .setMailboxName(s);
    outbox_ .setMailboxName(s);
    sent_   .setMailboxName(s);
    drafts_ .setMailboxName(s);
    trash_  .setMailboxName(s);
    
    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_FOLDERS);

    inbox_  .setFolderPath  (c->readEntry(FOLDER_INBOX,   i18n("Inbox")));
    outbox_ .setFolderPath  (c->readEntry(FOLDER_OUTBOX,  i18n("Outbox")));
    sent_   .setFolderPath  (c->readEntry(FOLDER_SENT,    i18n("Sent")));
    drafts_ .setFolderPath  (c->readEntry(FOLDER_DRAFTS,  i18n("Drafts")));
    trash_  .setFolderPath  (c->readEntry(FOLDER_TRASH,   i18n("Trash")));
   
    mailboxList()->loadConfig();
}

Empath::~Empath()
{
    delete mailboxList_;
    mailboxList_ = 0L;

    delete sender_;
    sender_ = 0L;

    delete filterList_;
    filterList_ = 0L;

    delete jobScheduler_;
    jobScheduler_ = 0L;

    delete composer_;
    composer_   = 0L;
    
    using namespace EmpathConfig;

    delete DFLT_Q_1;
    delete DFLT_Q_2;
    delete DFLT_LINK;
    delete DFLT_NEW;

    DFLT_Q_1 = DFLT_Q_2 = DFLT_LINK = DFLT_NEW = 0L;
}

    void
Empath::s_saveConfig()
{
    filterList()->saveConfig();
    mailboxList()->saveConfig();
    KGlobal::config()->sync();
}

    RMM::RMessage
Empath::message(const EmpathURL & source)
{
    QCacheIterator<EmpathCachedMessage> it(cache_);

    for (; it.current(); ++it)

        if (it.current()->refCount() == 0) {
            cache_.remove(it.currentKey());
            break; // One at a time.
        }
    
    EmpathCachedMessage * cached = cache_[source.asString()];

    if (cached == 0)
        return RMM::RMessage();

    return cached->message();
}

    void
Empath::cacheMessage(const EmpathURL & url, RMM::RMessage m)
{
    EmpathCachedMessage * cached = cache_[url.asString()];

    if (cached == 0) {

        cached = new EmpathCachedMessage(m);
        cache_.insert(url.asString(), cached);
    }

    cached->ref();
}
        
    EmpathMailboxList *
Empath::mailboxList()
{
    if (0 == mailboxList_)
        mailboxList_ = new EmpathMailboxList;

    return mailboxList_;
}

    EmpathMailSender *
Empath::_sender()
{
    if (0 == sender_)
        sender_ = new EmpathMailSender;

    return sender_;
}

    void
Empath::updateOutgoingServer()
{
    _sender()->update();
}

    EmpathJobScheduler *
Empath::_jobScheduler()
{
    if (0 == jobScheduler_)
        jobScheduler_ = new EmpathJobScheduler;

    return jobScheduler_;
}

    EmpathComposer *
Empath::_composer()
{
    if (0 == composer_)
        composer_ = new EmpathComposer;

    return composer_;
}

    EmpathFilterList *
Empath::filterList()
{
    if (0 == filterList_) {
        filterList_ = new EmpathFilterList;
        filterList_->loadConfig();
    }

    return filterList_;
}

    EmpathMailbox *
Empath::mailbox(const EmpathURL & url)
{ return (*mailboxList())[url.mailboxName()]; }

     EmpathFolder *
Empath::folder(const EmpathURL & url)
{ EmpathMailbox * m = mailbox(url); return (m == 0 ? 0 : m->folder(url)); }

   EmpathJobID
Empath::copy(const EmpathURL & from, const EmpathURL & to, QObject * o)
{ return _jobScheduler()->newCopyJob(from, to, o); }

    EmpathJobID
Empath::move(const EmpathURL & from, const EmpathURL & to, QObject * o)
{ return _jobScheduler()->newMoveJob(from, to, o); }

    EmpathJobID
Empath::retrieve(const EmpathURL & url, QObject * o)
{ return _jobScheduler()->newRetrieveJob(url, o); }

    EmpathJobID
Empath::write(RMM::RMessage & msg, const EmpathURL & folder, QObject * o)
{ return _jobScheduler()->newWriteJob(msg, folder, o); } 

    EmpathJobID
Empath::remove(const EmpathURL & url, QObject * o)
{ return _jobScheduler()->newRemoveJob(url, o); }

    EmpathJobID
Empath::remove(const EmpathURL & f, const QStringList & IDList, QObject * o)
{ return _jobScheduler()->newRemoveJob(f, IDList, o); }

    EmpathJobID
Empath::mark(const EmpathURL & url, EmpathIndexRecord::Status s, QObject * o)
{ return _jobScheduler()->newMarkJob(url, s, o); }

    EmpathJobID
Empath::mark(
    const EmpathURL & f,
    const QStringList & l,
    EmpathIndexRecord::Status s,
    QObject * o
)
{ return _jobScheduler()->newMarkJob(f, l, s, o); }

    EmpathJobID
Empath::createFolder(const EmpathURL & url, QObject * o) 
{ return _jobScheduler()->newCreateFolderJob(url, o); }

    EmpathJobID
Empath::removeFolder(const EmpathURL & url, QObject * o)
{ return _jobScheduler()->newRemoveFolderJob(url, o); }

    void
Empath::send(RMM::RMessage & m)
{ _sender()->send(m); }

    void
Empath::queue(RMM::RMessage & m)
{ _sender()->queue(m); }

    void
Empath::sendQueued()
{ _sender()->sendQueued(); }

    void
Empath::s_newMailArrived()
{ emit(newMailArrived()); }

    void
Empath::filter(const EmpathURL & m)
{ filterList()->filter(m); }

    void
Empath::s_setup(SetupType t, QWidget * parent)
{ emit(setup(t, parent)); }

    void
Empath::s_about(QWidget * parent)
{ emit(about(parent)); }

    void
Empath::s_newTask(EmpathTask * t)
{ emit(newTask(t)); }

    void 
Empath::s_compose()
{ _composer()->newComposeForm(QString::null); }

    void 
Empath::s_composeTo(const QString & recipient)
{ _composer()->newComposeForm(recipient); }

    void
Empath::s_reply(const EmpathURL & url)
{ _composer()->newComposeForm(ComposeReply, url); }

    void
Empath::s_replyAll(const EmpathURL & url)
{ _composer()->newComposeForm(ComposeReplyAll, url); }

    void
Empath::s_forward(const EmpathURL & url)
{ _composer()->newComposeForm(ComposeForward, url); }

    void
Empath::s_bounce(const EmpathURL & url)
{ _composer()->newComposeForm(ComposeBounce, url); }

    void
Empath::saveMessage(const EmpathURL & url, QWidget * parent)
{ emit(getSaveName(url, parent)); }

    void
Empath::s_configureMailbox(const EmpathURL & u, QWidget * w)
{ emit(configureMailbox(u, w)); }

    void
Empath::s_infoMessage(const QString & s)
{ emit(infoMessage(s)); }

    void
Empath::s_checkMail()
{ emit(checkMail()); }

    void
Empath::s_updateFolderLists()
{ emit(updateFolderLists()); }
    
    void
Empath::s_syncFolderLists()
{ emit(syncFolderLists()); }

    QString
Empath::generateUnique()
{
    return (
        startupSecondsStr_ + '.' + pidStr_ + '_' +
        QString::number(seq_++) + '.' + hostName_);
}

    void
Empath::_setStartTime()
{
    struct timeval timeVal;
    struct timezone timeZone;
    
    gettimeofday(&timeVal, &timeZone);
    startupSeconds_ = timeVal.tv_sec;
    startupSecondsStr_.setNum(startupSeconds_);
}

    void
Empath::_saveHostName()
{
    struct utsname utsName;
    if (uname(&utsName) == 0)
        hostName_ = utsName.nodename;
}



// vim:ts=4:sw=4:tw=78
