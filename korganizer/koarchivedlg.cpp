// ArchiveDialog -- archive/delete past appointments.
// $Id$

#include <qlabel.h>
#include <qlayout.h>
#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <klineedit.h>

#include "kdateedit.h"
#include "koevent.h"
#include "calobject.h"

#include "koarchivedlg.h"
#include "koarchivedlg.moc"

ArchiveDialog::ArchiveDialog(CalObject *cal,QWidget *parent, const char *name)
  : KDialogBase (Plain,i18n("Archive / Delete Past Appointments"),
                 User1|User2|Cancel,User1,parent,name,false,true,
                 i18n("Archive"),i18n("Delete"))
{
  mCalendar = cal;

  QFrame *topFrame = plainPage();
  QGridLayout *topLayout = new QGridLayout(topFrame);
  topLayout->setSpacing(spacingHint());
  
  QLabel *dateLabel = new QLabel(i18n("Appointments older than:"),topFrame);
  topLayout->addWidget(dateLabel,0,0);

  mDateEdit = new KDateEdit(topFrame);
  topLayout->addWidget(mDateEdit,0,1);

  QHBox *fileBox = new QHBox(topFrame);
  fileBox->setSpacing(spacingHint());
  topLayout->addMultiCellWidget(fileBox,1,1,0,1);
  (void)new QLabel(i18n("Archive file:"),fileBox);
  mArchiveFile = new KURLRequester (fileBox);
  mArchiveFile->fileDialog()->setMode(KFile::File);
  mArchiveFile->fileDialog()->setFilter("*vcs|vCalendar Files");
}

ArchiveDialog::~ArchiveDialog()
{
}

// Archive old events
void ArchiveDialog::slotUser1()
{
/*
  QString str = mArchiveFile->lineEdit()->text();
  if (str.isEmpty()) {
    KMessageBox::sorry(this,i18n("The archive file name is not valid.\n"));
    return;
  }

  qDebug("str: %s",str.latin1());
  KURL destUrl;
  destUrl.setPath(str);
  qDebug("url: %s",destUrl.prettyURL().latin1());
  QString filename = destUrl.fileName();
  if (filename.right(4) != ".vcs") {
    filename.append(".vcs");
    destUrl.setFileName(filename);
  }
  qDebug("url: %s",destUrl.prettyURL().latin1());
*/

  // Get destination URL
  KURL destUrl = mArchiveFile->url();
  if (destUrl.isMalformed()) {
    KMessageBox::sorry(this,i18n("The archive file name is not valid.\n"));
    return;
  }
  // Force filename to be ending with vCalendar extension
  QString filename = destUrl.fileName();
  if (filename.right(4) != ".vcs") {
    filename.append(".vcs");
    destUrl.setFileName(filename);
  }
  
  // Get events to be archived
  QList<KOEvent> events = mCalendar->getEvents(QDate(1800,1,1),
                                               mDateEdit->getDate(),true);
  if (events.count() == 0) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->getDate())));
    return;
  }

  // Save current calendar to disk
  KTempFile tmpFile;
  tmpFile.setAutoDelete(true);
  if (!mCalendar->save(tmpFile.name())) {
    qDebug("ArchiveDialog::slotUser1(): Can't save calendar to temp file");
    return;
  }

  // Duplicate current calendar by loading in new calendar object
  CalObject archiveCalendar;
  if (!archiveCalendar.load(tmpFile.name())) {
    qDebug("ArchiveDialog::slotUser1(): Can't load calendar from temp file");
    return;
  }

  // Strip active events from calendar so that only events to be archived
  // remain.
  QList<KOEvent> activeEvents = archiveCalendar.getEvents(mDateEdit->getDate(),
                                                          QDate(3000,1,1),
                                                          false);
  KOEvent *ev;
  for(ev=activeEvents.first();ev;ev=activeEvents.next()) {
    archiveCalendar.deleteEvent(ev);
  }

  // Get or create the archive file
  QString archiveFile;

  if (KIO::NetAccess::exists(destUrl)) {
    if(!KIO::NetAccess::download(destUrl,archiveFile)) {
      qDebug("ArchiveDialog::slotUser1(): Can't download archive file");
      return;
    }
    // Merge with events to be archived.
    if (!archiveCalendar.load(archiveFile)) {
      qDebug("ArchiveDialog::slotUser1(): Can't merge with archive file");
      return;
    }
/*    
    QList<KOEvent> es = archiveCalendar.getEvents(QDate(1800,1,1),
                                                  QDate(3000,1,1),
                                                  false);
    qDebug("--Following events in archive calendar:");
    KOEvent *e;
    for(e=es.first();e;e=es.next()) {
      qDebug("-----Event: %s",e->getSummary().latin1());
    }
*/
  } else {
    archiveFile = tmpFile.name();
  }
   
  // Save archive calendar
  if (!archiveCalendar.save(archiveFile)) {
    KMessageBox::error(this,i18n("Cannot write archive file."));
    return;
  }

  // Upload if necessary
  KURL srcUrl;
  srcUrl.setPath(archiveFile);
  if (srcUrl != destUrl) {
    if (!KIO::NetAccess::upload(archiveFile,destUrl)) {
      KMessageBox::error(this,i18n("Cannot write archive to final destination."));
      return;
    }
  }

  KIO::NetAccess::removeTempFile(archiveFile);
   
  // Delete archived events from calendar    
  for(ev=events.first();ev;ev=events.next()) {
    mCalendar->deleteEvent(ev);
  }
  emit eventsDeleted();

#if 0
  // NetAccess::upload deletes the contents of the file. This is a bug!
  QString testFile = "/home/corni/tmp/piep";
  KURL testUrl;
  testUrl.setPath(testFile);
  if (!KIO::NetAccess::upload(testFile,testUrl)) {
    qDebug("test upload failed");
  }
#endif

  accept();
}

// Delete old events
void ArchiveDialog::slotUser2()
{
  QList<KOEvent> events = mCalendar->getEvents(QDate(1769,12,1),
                                               mDateEdit->getDate(),true);

  if (events.count() == 0) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->getDate())));
    return;
  }

  QStringList eventStrs;
  KOEvent *ev;
  for(ev=events.first();ev;ev=events.next()) {
    eventStrs.append(ev->getSummary());
  }
  
  int result = KMessageBox::questionYesNoList(this,
      i18n("Delete all events before %1?\nThe following events will be deleted:")
      .arg(KGlobal::locale()->formatDate(mDateEdit->getDate())),eventStrs,
      i18n("Delete old events"),i18n("Delete"));
  if (result == KMessageBox::Yes) {
    for(ev=events.first();ev;ev=events.next()) {
      mCalendar->deleteEvent(ev);
    }
    emit eventsDeleted();
    accept();
  }
}
