/***************************************************************************
                          knaccnewssettings.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
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
#include <qpushbutton.h>
#include <qframe.h>
#include <qwidget.h>
#include <qgroupbox.h>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <knumvalidator.h>
#include <kmessagebox.h>

#include "knnntpaccount.h"
#include "knlistbox.h"
#include "knaccountmanager.h"
#include "utilities.h"
#include "knaccnewssettings.h"


KNAccNewsSettings::KNAccNewsSettings(QWidget *p, KNAccountManager *am)
	: KNSettingsWidget(p), aManager(am), pm(UserIcon("server"))
{
  QGridLayout *topL=new QGridLayout(this, 5,2, 5, 5);

  // account listbox
  lb=new KNListBox(this);
  connect(lb, SIGNAL(selected(int)), this, SLOT(slotItemSelected(int)));
  connect(lb, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(lb, 0,3, 0,0);

  // info box
	QGroupBox *gb = new QGroupBox(2,Qt::Horizontal,QString::null,this);
	topL->addWidget(gb,4,0);
	
  serverInfo = new QLabel(gb);
  portInfo = new QLabel(gb);

	// buttons
  addBtn=new QPushButton(i18n("&Add"), this);
  connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(addBtn, 0,1);
	
  delBtn=new QPushButton(i18n("&Delete"), this);
  connect(delBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(delBtn, 1,1);
	
  editBtn=new QPushButton(i18n("&Edit"), this);
  connect(editBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(editBtn, 2,1);
		
  topL->setRowStretch(3,1);   // stretch the server listbox
  topL->activate();	

	for(KNNntpAccount *a=aManager->first(); a; a=aManager->next())
		slotAddItem(a);

	// the settings dialog is non-modal, so we have to react to changes
	// made outside of the dialog
  connect(aManager, SIGNAL(accountAdded(KNNntpAccount*)), this, SLOT(slotAddItem(KNNntpAccount*)));
  connect(aManager, SIGNAL(accountRemoved(KNNntpAccount*)), this, SLOT(slotRemoveItem(KNNntpAccount*)));
  connect(aManager, SIGNAL(accountModified(KNNntpAccount*)), this, SLOT(slotUpdateItem(KNNntpAccount*)));
		
  slotSelectionChanged();     // disable Delete & Edit initially
}



KNAccNewsSettings::~KNAccNewsSettings()
{
}



void KNAccNewsSettings::slotAddItem(KNNntpAccount *a)
{
	KNLBoxItem *it;
	it=new KNLBoxItem(a->name(), a, &pm);
	lb->insertItem(it);
}



void KNAccNewsSettings::slotRemoveItem(KNNntpAccount *a)
{
	KNLBoxItem *it;
	for(uint i=0; i<lb->count(); i++) {
		it=lb->itemAt(i);
		if(it && it->data()==a) {
			lb->removeItem(i);
			if(lb->currentItem()!=-1)
			  lb->setSelected(lb->currentItem(), true);
			break;
		}
	}
}



void KNAccNewsSettings::slotUpdateItem(KNNntpAccount *a)
{
  KNLBoxItem *it;
  for(uint i=0; i<lb->count(); i++) {
    it=lb->itemAt(i);
    if(it && it->data()==a) {
      it=new KNLBoxItem(a->name(), a, &pm);
      lb->changeItem(it, i);
      break;
    }
  }
}



void KNAccNewsSettings::slotSelectionChanged()
{
  delBtn->setEnabled(lb->currentItem()!=-1);
  editBtn->setEnabled(lb->currentItem()!=-1);

  KNLBoxItem *it = lb->itemAt(lb->currentItem());
  if (it) {
    KNNntpAccount *a = static_cast<KNNntpAccount*>(it->data());
    serverInfo->setText(i18n("Server: %1").arg(a->server().data()));
    portInfo->setText(i18n("Port: %1").arg(a->port()));
  } else {
    serverInfo->setText(i18n("Server: "));
    portInfo->setText(i18n("Port: "));
  }
}



void KNAccNewsSettings::slotItemSelected(int id)
{
  slotEditBtnClicked();
}



void KNAccNewsSettings::slotAddBtnClicked()
{
  KNNntpAccount *acc = new KNNntpAccount();
  KNAccNewsConfDialog *confDlg = new KNAccNewsConfDialog(acc,this);

  if (confDlg->exec())
  	aManager->newAccount(acc);
  else
    delete acc;

  delete confDlg;
}



void KNAccNewsSettings::slotDelBtnClicked()
{
  KNLBoxItem *it = lb->itemAt(lb->currentItem());

  if (it)
    aManager->removeAccount(static_cast<KNNntpAccount*>(it->data()));
}



void KNAccNewsSettings::slotEditBtnClicked()
{
  KNLBoxItem *it = lb->itemAt(lb->currentItem());

  if (it) {
    KNNntpAccount *a = static_cast<KNNntpAccount*>(it->data());
    KNAccNewsConfDialog *confDlg = new KNAccNewsConfDialog(a,this);

    if (confDlg->exec())
      aManager->applySettings(a);   // the account manager will emit accountModified()...

    delete confDlg;
  }
}


//===============================================================================


KNAccNewsConfDialog::KNAccNewsConfDialog(KNNntpAccount *a, QWidget *parent, const char *name)
  : KDialogBase(Plain, (a->id()!=-1)? i18n("Properties of %1").arg(a->name()):i18n("New Account"),
	              Ok|Cancel|Help, Ok, parent, name),
	  acc(a)
{
  QFrame* page=plainPage();
  QGridLayout *topL=new QGridLayout(page, 9, 3, 5);

  QLabel *l=new QLabel(i18n("Name:"),page);	
  topL->addWidget(l, 0,0);	
  n_ame=new QLineEdit(page);
  n_ame->setText(acc->name());
  topL->addMultiCellWidget(n_ame, 0, 0, 1, 2);	

  l=new QLabel(i18n("Server:"), page);	
  topL->addWidget(l, 1,0);
  s_erver=new QLineEdit(page);	
  s_erver->setText(acc->server());
  if (acc->id()!=-1) s_erver->setEnabled(false);
  topL->addMultiCellWidget(s_erver, 1, 1, 1, 2);
	
  l=new QLabel(i18n("Port:"), page);	
  topL->addWidget(l, 2,0);
  p_ort=new QLineEdit(page);	
  p_ort->setValidator(new KIntValidator(0,65536,this));
  p_ort->setText(QString::number(acc->port()));	
  topL->addWidget(p_ort, 2,1);

  l = new QLabel(i18n("Hold connection for:"), page);
  topL->addWidget(l,3,0);
  h_old = new QSpinBox(5,1800,5,page);
  h_old->setValue(acc->hold());
  topL->addWidget(h_old,3,1);
  l = new QLabel(i18n("secs"),page);
  topL->addWidget(l,3,2);

  l = new QLabel(i18n("Timeout:"), page);
  topL->addWidget(l,4,0);
  t_imeout = new QSpinBox(5,600,5,page);
  t_imeout->setValue(acc->timeout());
  topL->addWidget(t_imeout,4,1);
  l = new QLabel(i18n("secs"),page);
  topL->addWidget(l,4,2);

  authCB=new QCheckBox(i18n("Server requires &authentication"), page);
  connect(authCB, SIGNAL(toggled(bool)), this, SLOT(slotAuthChecked(bool)));
  topL->addMultiCellWidget(authCB, 5,5, 0,3);

  l=new QLabel(i18n("User:"), page);
  topL->addWidget(l, 6,0);
  u_ser=new QLineEdit(page);
  u_ser->setText(acc->user());
  topL->addMultiCellWidget(u_ser, 6,6, 1,2);
	
  l=new QLabel(i18n("Password:"), page);
  topL->addWidget(l, 7,0);
  p_ass=new QLineEdit(page);
  p_ass->setEchoMode(QLineEdit::Password);
  p_ass->setText(acc->pass());		
  topL->addMultiCellWidget(p_ass, 7,7, 1,2);

  slotAuthChecked(acc->needsLogon());

  topL->setColStretch(2, 1);
	topL->activate();
	
	restoreWindowSize("accNewsPropDLG", this, sizeHint());
}



KNAccNewsConfDialog::~KNAccNewsConfDialog()
{
  saveWindowSize("accNewsPropDLG", size());
}


void KNAccNewsConfDialog::slotOk()
{
  if (n_ame->text().isEmpty() || s_erver->text().isEmpty()) {
    KMessageBox::information(this, i18n("Please enter an arbitrary name for the account and the\nhostname of the news server."));
    return;
  }

	acc->setName(n_ame->text());
	acc->setServer(s_erver->text().local8Bit());
	acc->setPort(p_ort->text().toInt());
	acc->setHold(h_old->value());
	acc->setTimeout(t_imeout->value());
	acc->setNeedsLogon(authCB->isChecked());
	acc->setUser(u_ser->text().local8Bit());
	acc->setPass(p_ass->text().local8Bit());
	
	accept();
}


void KNAccNewsConfDialog::slotAuthChecked(bool b)
{
  authCB->setChecked(b);
	u_ser->setEnabled(b);
	p_ass->setEnabled(b);
}


//--------------------------------

#include "knaccnewssettings.moc"
