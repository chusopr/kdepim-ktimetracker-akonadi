/***************************************************************************
                     kngroupdialog.cpp - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include <qstrlist.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kglobal.h>
#include <kdatepik.h>
#include <klocale.h>

#include "utilities.h"
#include "kngroupmanager.h"
#include "knnntpaccount.h"
#include "kngroupdialog.h"


KNGroupDialog::KNGroupDialog(QWidget *parent, KNNntpAccount *a) :
  KNGroupBrowser(parent, i18n("Subscribe to Newsgroups"),a, User1 | User2, true, i18n("New &List"), i18n("New &Groups") )
{
  rightLabel->setText(i18n("Current changes:"));
  subView=new QListView(page);
  subView->addColumn(i18n("subscribe to"));
  unsubView=new QListView(page);
  unsubView->addColumn(i18n("unsubscribe from"));

  QVBoxLayout *protL=new QVBoxLayout(3);
  listL->addLayout(protL, 1,2);
  protL->addWidget(subView);
  protL->addWidget(unsubView);

  dir1=right;
  dir2=left;

  connect(groupView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));
  connect(subView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));
  connect(unsubView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));

  connect(arrowBtn1, SIGNAL(clicked()), this, SLOT(slotArrowBtn1()));
  connect(arrowBtn2, SIGNAL(clicked()), this, SLOT(slotArrowBtn2()));

  restoreWindowSize("groupDlg", this, QSize(662,393));  // optimized for 800x600
}



KNGroupDialog::~KNGroupDialog()
{
  saveWindowSize("groupDlg", this->size());
}



void KNGroupDialog::itemChangedState(CheckItem *it, bool s)
{
  qDebug("KNGroupDialog::itemChangedState()");
  if(s){
    if(itemInListView(unsubView, it->text(0))) {
      removeListItem(unsubView, it->text(0));
      setButtonDirection(btn2, right);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(true);
    }
    else {
      new QListViewItem(subView, it->text(0));
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
  else {
    if(itemInListView(subView, it->text(0))) {
      removeListItem(subView, it->text(0));
      setButtonDirection(btn1, right);
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
    }
    else {
      new QListViewItem(unsubView, it->text(0));
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
}



void KNGroupDialog::updateItemState(CheckItem *it)
{
  it->setChecked( (it->info->subscribed && !itemInListView(unsubView, it->text(0))) ||
                  (!it->info->subscribed && itemInListView(subView, it->text(0))) );

  if((it->info->subscribed || it->info->newGroup) && it->pixmap(0)==0)
    it->setPixmap(0, (it->info->newGroup)? pmNew:pmGroup);
}



void KNGroupDialog::toSubscribe(QSortedList<KNGroupInfo> *l)
{
  l->clear();

  for (KNGroupInfo *i=allList->first(); i; i=allList->next())
    if (itemInListView(subView, i->name))
      l->append(i);
}



void KNGroupDialog::toUnsubscribe(QStrList *l)
{
  l->clear();
  QListViewItemIterator it(unsubView);
  for(; it.current(); ++it)
    l->append(it.current()->text(0).latin1());
}



void KNGroupDialog::setButtonDirection(arrowButton b, arrowDirection d)
{
  QPushButton *btn=0;
  if(b==btn1 && dir1!=d) {
    btn=arrowBtn1;
    dir1=d;
  }
  else if(b==btn2 && dir2!=d) {
    btn=arrowBtn2;
    dir2=d;
  }

  if(btn) {
    if(d==right)
      btn->setPixmap(pmRight);
    else
      btn->setPixmap(pmLeft);
  }
}



void KNGroupDialog::slotItemSelected(QListViewItem *it)
{
  const QObject *s=sender();


  if(s==subView) {
    unsubView->clearSelection();
    groupView->clearSelection();
    arrowBtn2->setEnabled(false);
    arrowBtn1->setEnabled(true);
    setButtonDirection(btn1, left);
  }
  else if(s==unsubView) {
    subView->clearSelection();
    groupView->clearSelection();
    arrowBtn1->setEnabled(false);
    arrowBtn2->setEnabled(true);
    setButtonDirection(btn2, left);
  }
  else {
    CheckItem *cit;
    subView->clearSelection();
    unsubView->clearSelection();
    cit=static_cast<CheckItem*>(it);
    if(!cit->isOn() && !itemInListView(subView, cit->text(0)) && !itemInListView(unsubView, cit->text(0))) {
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
      setButtonDirection(btn1, right);
    }
    else if(cit->isOn() && !itemInListView(unsubView, cit->text(0)) && !itemInListView(subView, cit->text(0))) {
      arrowBtn2->setEnabled(true);
      arrowBtn1->setEnabled(false);
      setButtonDirection(btn2, right);
    }
    else {
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
}



void KNGroupDialog::slotArrowBtn1()
{
  QListViewItem *it=0;

  if(dir1==right) {
    it=groupView->selectedItem();
    new QListViewItem(subView, it->text(0));
    (static_cast<CheckItem*>(it))->setChecked(true);
  }
  else {
    it=subView->selectedItem();
    changeItemState(it->text(0), false);
    delete it;
  }

  arrowBtn1->setEnabled(false);
}



void KNGroupDialog::slotArrowBtn2()
{
  QListViewItem *it=0;

  if(dir2==right) {
    it=groupView->selectedItem();
    new QListViewItem(unsubView, it->text(0));
    (static_cast<CheckItem*>(it))->setChecked(false);
  }
  else {
    it=unsubView->selectedItem();
    changeItemState(it->text(0), true);
    delete it;
  }

  arrowBtn2->setEnabled(false);
}


// new list
void KNGroupDialog::slotUser1()
{
  leftLabel->setText(i18n("Downloading groups..."));
  enableButton(User1,false);
  enableButton(User2,false);
  emit(fetchList(a_ccount));
}


// new groups
void KNGroupDialog::slotUser2()
{
  QDate lastDate = a_ccount->lastNewFetch();
  KDialogBase *dlg = new KDialogBase( this, 0L, true, i18n("New Groups"), Ok | Cancel, Ok);

  QButtonGroup *btnGrp = new QButtonGroup(i18n("Check for new groups:"),dlg);
  dlg->setMainWidget(btnGrp);
  QGridLayout *topL = new QGridLayout(btnGrp,4,2,25,10);

  QRadioButton *takeLast = new QRadioButton( i18n("created since last check:"), btnGrp );
  topL->addMultiCellWidget(takeLast, 0, 0, 0, 1);

  QLabel *l = new QLabel(KGlobal::locale()->formatDate(lastDate, false),btnGrp);
  topL->addWidget(l, 1, 1, Qt::AlignLeft);

  connect(takeLast, SIGNAL(toggled(bool)), l, SLOT(setEnabled(bool)));

  QRadioButton *takeCustom = new QRadioButton( i18n("created since this date:"), btnGrp );
  topL->addMultiCellWidget(takeCustom, 2, 2, 0, 1);

  KDatePicker *dateSel = new KDatePicker(btnGrp, lastDate);
  dateSel->setMinimumSize(dateSel->sizeHint());
  topL->addWidget(dateSel, 3, 1, Qt::AlignLeft);

  connect(takeCustom, SIGNAL(toggled(bool)), dateSel, SLOT(setEnabled(bool)));

  takeLast->setChecked(true);
  dateSel->setEnabled(false);

  topL->addColSpacing(0,30);
  dlg->disableResize();

  if (dlg->exec()) {
    if (takeCustom->isChecked())
      lastDate = dateSel->getDate();
    a_ccount->setLastNewFetch(QDate::currentDate());
    a_ccount->saveInfo();
    leftLabel->setText(i18n("Checking for new groups..."));
    enableButton(User1,false);
    enableButton(User2,false);
    filterEdit->clear();
    subCB->setChecked(false);
    newCB->setChecked(true);
    emit(checkNew(a_ccount,lastDate));
    slotRefilter();
  }

  delete dlg;
}


//--------------------------------

#include "kngroupdialog.moc"
