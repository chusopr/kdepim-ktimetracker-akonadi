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

#include "backupmailapplication.h"
#include "backupmailwidget.h"
#include "backupdata.h"
#include "restoredata.h"
#include "backupmailkernel.h"
#include "selectiontypedialog.h"
#include "backupmailutil.h"

#include <mailcommon/mailkernel.h>

#include <Akonadi/Control>

#include <KStandardAction>
#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KMessageBox>

#include <KLocale>

BackupMailApplication::BackupMailApplication(QWidget *parent)
  : KXmlGuiWindow(parent),mBackupData(0),mRestoreData(0)
{
  BackupMailKernel *kernel = new BackupMailKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

  setupActions();
  setupGUI(Default,"backupmailapplication.rc");
  mBackupMailWidget = new BackupMailWidget(this);

  setCentralWidget(mBackupMailWidget);
  Akonadi::Control::widgetNeedsAkonadi(this);

}

BackupMailApplication::~BackupMailApplication()
{
  if(mBackupData) {
    delete mBackupData;
  }
  if(mRestoreData) {
    delete mRestoreData;
  }
}

void BackupMailApplication::setupActions()
{
  KActionCollection* ac=actionCollection();

  KAction *backupAction = ac->addAction("backup",this,SLOT(slotBackupData()));
  backupAction->setText(i18n("Back Up Data..."));

  KAction *restoreAction = ac->addAction("restore",this,SLOT(slotRestoreData()));
  restoreAction->setText(i18n("Restore Data..."));
  KStandardAction::quit( this, SLOT(close()), ac );
}

void BackupMailApplication::slotBackupData()
{
  KMessageBox::information(this,i18n("Before to backup info, close all kdepim applications."),i18n("Backup"));

  const QString filename = KFileDialog::getSaveFileName(KUrl(),QLatin1String("*.zip"),this,i18n("Create backup"));
  if(filename.isEmpty())
    return;
  SelectionTypeDialog *dialog = new SelectionTypeDialog(this);
  if(dialog->exec()) {
    Util::BackupTypes typeSelected = dialog->backupTypesSelected();
    delete mBackupData;
    mBackupData = new BackupData(typeSelected,filename);
    connect(mBackupData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
    connect(mBackupData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
    mBackupData->startBackup();
  }
  delete dialog;
}

void BackupMailApplication::slotAddInfo(const QString& info)
{
  qDebug()<<" BackupMailApplication::slotAddInfo"<<info;
  mBackupMailWidget->addInfoLogEntry(info);
}

void BackupMailApplication::slotAddError(const QString& info)
{
  qDebug()<<"BackupMailApplication::slotAddError "<<info;
  mBackupMailWidget->addErrorLogEntry(info);
}


void BackupMailApplication::slotRestoreData()
{
  const QString filename = KFileDialog::getOpenFileName(KUrl(),QLatin1String("*.zip"),this,i18n("Restore backup"));
  if(filename.isEmpty())
    return;

  SelectionTypeDialog *dialog = new SelectionTypeDialog(this);
  if(dialog->exec()) {
    Util::BackupTypes typeSelected = dialog->backupTypesSelected();
    delete mRestoreData;
    mRestoreData = new RestoreData(typeSelected,filename);
    connect(mRestoreData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
    connect(mRestoreData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
    mRestoreData->startRestore();
  }
  delete dialog;
}


#include "backupmailapplication.moc"
