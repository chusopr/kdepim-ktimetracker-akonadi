/* setupDialog.cc			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
**
** This file is part of the popmail conduit, a conduit for KPilot that
** synchronises the Pilot's email application with the outside world,
** which currently means:
**	-- sendmail or SMTP for outgoing mail
**	-- POP or mbox for incoming mail
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
// This is an old trick so you can determine what revisions
// make up a binary distribution.
//
//
static const char *setupDialog_id=
	"$Id$";

#include "options.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream.h>

#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

#ifndef _KAPP_H
#include <kapp.h>
#endif

#ifndef _KLOCALE_H
#include <klocale.h>
#endif

#ifndef _KSTDDIRS_H
#include <kstddirs.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif



#ifndef QLABEL_H
#include <qlabel.h>
#endif

#ifndef QCHKBOX_H
#include <qchkbox.h>
#endif

#ifndef QLINED_H
#include <qlined.h>
#endif

#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef QFILEDLG_H
#include <qfiledlg.h>
#endif

#ifndef QBTTNGRP_H
#include <qbttngrp.h>
#endif

#ifndef QPUSHBUTTON_H
#include <qpushbutton.h>
#endif

#ifndef QRADIOBT_H
#include <qradiobt.h>
#endif

#ifndef QLAYOUT_H
#include <qlayout.h>
#endif

#ifndef QVBUTTONGROUP_H
#include <qvbuttongroup.h>
#endif


#ifndef _KPILOT_POPMAIL_CONDUIT_H
#include "popmail-conduit.h"
#endif

#ifndef _KPILOT_SETUPDIALOG_H
#include "setupDialog.moc"
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif


PopMailSendPage::PopMailSendPage(setupDialog *parent,KConfig& config) :
	setupDialogPage(i18n("Sending Mail"),parent)
{
	FUNCTIONSETUP;
	QGridLayout *grid=new QGridLayout(this,6,3,SPACING);
	QLabel *currentLabel;

	sendGroup=new QVButtonGroup(i18n("Send Method"),
		this,"sb");

	fNoSend=new QRadioButton(i18n("&Do not send mail"),sendGroup);
	fSendmail=new QRadioButton(i18n("Use &Sendmail"),sendGroup);
	fSMTP=new QRadioButton(i18n("Use S&MTP"),sendGroup);

	connect(fNoSend,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fSMTP,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fSendmail,SIGNAL(clicked()),
		this,SLOT(toggleMode()));

	sendGroup->adjustSize();

	grid->addMultiCellWidget(sendGroup,0,0,0,2);
	

	//-----------------------------------------------
	//
	// Sending mail options.
	//
	currentLabel = new QLabel(i18n("Email Address: "), 
			    this);

	fEmailFrom = new QLineEdit(this);
	fEmailFrom->setText(config.readEntry("EmailAddress", "$USER"));
	fEmailFrom->resize(200, fEmailFrom->height());

	grid->addWidget(currentLabel,1,0);
	grid->addWidget(fEmailFrom,1,1);

	currentLabel = new QLabel(i18n("Signature File: "), 
			    this);
	currentLabel->adjustSize();

	fSignature = new QLineEdit(this);
	fSignature->setText(config.readEntry("Signature", ""));
	fSignature->resize(200, fSignature->height());
	
	fSignatureBrowse=new QPushButton(i18n("Browse"),this);
	fSignatureBrowse->adjustSize();

	connect(fSignatureBrowse,SIGNAL(clicked()),
		this,SLOT(browseSignature()));

	grid->addWidget(currentLabel,2,0);
	grid->addWidget(fSignature,2,1);
	grid->addWidget(fSignatureBrowse,2,2);

	currentLabel = new QLabel(i18n("Sendmail Command:"), this);
	currentLabel->adjustSize();
	
	fSendmailCmd = new QLineEdit(this);
	fSendmailCmd->setText(config.readEntry("SendmailCmd", 
		"/usr/lib/sendmail -t -i"));
	fSendmailCmd->resize(300, fSendmailCmd->height());

	grid->addWidget(currentLabel,4,0);
	grid->addWidget(fSendmailCmd,4,1);

	currentLabel = new QLabel(i18n("SMTP Server:"), this);
	currentLabel->adjustSize();
	
	fSMTPServer = new QLineEdit(this);
	fSMTPServer->setText(config.readEntry("SMTPServer", "mail"));
	fSMTPServer->resize(200, fSendmailCmd->height());

	grid->addWidget(currentLabel,6,0);
	grid->addWidget(fSMTPServer,6,1);

	currentLabel = new QLabel(i18n("SMTP Port:"), this);
	currentLabel->adjustSize();
	
	fSMTPPort = new QLineEdit(this);
	fSMTPPort->setText(config.readEntry("SMTPPort", "25"));
	fSMTPPort->resize(200, fSendmailCmd->height());

	grid->addWidget(currentLabel,7,0);
	grid->addWidget(fSMTPPort,7,1);

	setMode(PopMailConduit::SendMode(config.readNumEntry("SyncOutgoing",
		PopMailConduit::SEND_NONE)));
}

/* virtual */ int PopMailSendPage::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;


	if (parentSetup->queryFile(i18n("Signature File %1 is missing."),
		fSignature->text())!=KMessageBox::No)
	{
		config.writeEntry("Signature", fSignature->text());
	}

	config.writeEntry("EmailAddress", fEmailFrom->text());


	config.writeEntry("SendmailCmd", fSendmailCmd->text());
	config.writeEntry("SMTPServer", fSMTPServer->text());
	config.writeEntry("SMTPPort", fSMTPPort->text());

	config.writeEntry("SyncOutgoing", (int)getMode());

	return 0;
}


/* slot */ void PopMailSendPage::toggleMode()
{
	if (fNoSend->isChecked()) setMode(PopMailConduit::SEND_NONE);
	if (fSendmail->isChecked()) setMode(PopMailConduit::SEND_SENDMAIL);
	if (fSMTP->isChecked()) setMode(PopMailConduit::SEND_SMTP);
}

void PopMailSendPage::setMode(PopMailConduit::SendMode m)
{
	FUNCTIONSETUP;

	switch(m)
	{
	case PopMailConduit::SEND_SENDMAIL :
		fSendmailCmd->setEnabled(true);
		fSMTPServer->setEnabled(false);
		fSMTPPort->setEnabled(false);
		fSendmail->setChecked(true);
		break;
	case PopMailConduit::SEND_SMTP :
		fSendmailCmd->setEnabled(false);
		fSMTPServer->setEnabled(true);
		fSMTPPort->setEnabled(true);
		fSMTP->setChecked(true);
		break;
	case PopMailConduit::SEND_NONE :
		fSendmailCmd->setEnabled(false);
		fSMTPServer->setEnabled(false);
		fSMTPPort->setEnabled(false);
		fNoSend->setChecked(true);
		break;
	default :
		cerr << __FUNCTION__
			<< ": Unknown mode " << (int) m
			<< endl;
		return;
	}

	fMode=m;
}




void PopMailSendPage::browseSignature()
{
	FUNCTIONSETUP;

	QString filename=fSignature->text();

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Signature currently "
			<< fSignature->text() << endl;
	}
#endif

	if (filename.isEmpty()) 
	{
		filename=QDir::currentDirPath();
	}
	else
	{
		filename=QFileInfo( filename ).dirPath();
	}

	filename = QFileDialog::getOpenFileName(filename,"*");

#ifdef DEBUG
	if (debug_level & UI_MINOR)
	{
		kdDebug() << fname << ": Signature selected "
			<< filename << endl;
	}
#endif

	if (!filename.isEmpty())
	{
		fSignature->setText(filename);
	}
}



PopMailReceivePage::PopMailReceivePage(setupDialog *parent,
	KConfig& config) :
	setupDialogPage(i18n("Receiving Mail"),parent)
{
	FUNCTIONSETUP;
	QLabel *currentLabel;
	QGridLayout *grid=new QGridLayout(this,8,3,SPACING);

	methodGroup=new QVButtonGroup(i18n("Retrieve Method"),
		this,"bg");

	fNoReceive=new QRadioButton(i18n("Do &Not receive mail"),
		methodGroup);
	fReceivePOP=new QRadioButton(i18n("Use &POP3 server"),
		methodGroup);
	fReceiveUNIX=new QRadioButton(i18n("Use &UNIX Mailbox"),
		methodGroup);

	connect(fNoReceive,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fReceivePOP,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fReceiveUNIX,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	methodGroup->adjustSize();


	grid->addMultiCellWidget(methodGroup,0,0,0,2);

	currentLabel = new QLabel(i18n("UNIX Mailbox:"),this);
	currentLabel->adjustSize();

	{
	QString defaultMailbox;
	char *u=getenv("USER");
	if (u==0L)
	{
		u=getenv("HOME");
		if (u==0L)
		{
			defaultMailbox="mbox";
		}
		else
		{
			defaultMailbox=QString(u)+QString("mbox");
		}
	}
	else
	{
		defaultMailbox=QString("/var/spool/mail/")+QString(u);
	}
	fMailbox=new QLineEdit(this);
	fMailbox->setText(config.readEntry("UNIX Mailbox",defaultMailbox));
	fMailbox->resize(200,fMailbox->height());
	}
	fMailboxBrowse=new QPushButton(i18n("Browse"),this);
	fMailboxBrowse->adjustSize();
	
	connect(fMailboxBrowse,SIGNAL(clicked()),
		this,SLOT(browseMailbox()));

	grid->addWidget(currentLabel,1,0);
	grid->addWidget(fMailbox,1,1);
	grid->addWidget(fMailboxBrowse,1,2);

	//-----------------------------------------------
	//
	// Receiving mail options.
	//

	currentLabel = new QLabel(i18n("POP Server:"), this);
	currentLabel->adjustSize();
	
	fPopServer = new QLineEdit(this);
	fPopServer->setText(config.readEntry("PopServer", "pop"));
	fPopServer->resize(200, fPopServer->height());
	
	grid->addWidget(currentLabel,3,0);
	grid->addWidget(fPopServer,3,1);

	currentLabel = new QLabel(i18n("POP Port:"), this);
	currentLabel->adjustSize();
	
	fPopPort = new QLineEdit(this);
	fPopPort->setText(config.readEntry("PopPort", "110"));
	fPopPort->resize(200, fPopPort->height());
	
	grid->addWidget(currentLabel,4,0);
	grid->addWidget(fPopPort,4,1);

	currentLabel = new QLabel(i18n("POP Username:"), this);
	currentLabel->adjustSize();
	
	fPopUser = new QLineEdit(this);
	fPopUser->setText(config.readEntry("PopUser", "$USER"));
	fPopUser->resize(200, fPopUser->height());
	
	grid->addWidget(currentLabel,5,0);
	grid->addWidget(fPopUser,5,1);

	fLeaveMail = new QCheckBox(i18n("&Leave mail on server."), this);
	fLeaveMail->adjustSize();
	fLeaveMail->setChecked(config.readNumEntry("LeaveMail", 1));
	
	grid->addWidget(fLeaveMail,6,1);

	currentLabel = new QLabel(i18n("Pop Password:"), this);
	currentLabel->adjustSize();
	
	fPopPass = new QLineEdit(this);
	fPopPass->setEchoMode(QLineEdit::Password);
	fPopPass->setText(config.readEntry("PopPass", ""));
	fPopPass->resize(200, fPopPass->height());
	
	fPopPass->setEnabled(config.readNumEntry("StorePass", 0));

	grid->addWidget(currentLabel,7,0);
	grid->addWidget(fPopPass,7,1);


	fStorePass = new QCheckBox(i18n("Save &Pop password."), this);
	connect(fStorePass, SIGNAL(clicked()), this, SLOT(togglePopPass()));
	fStorePass->adjustSize();
	fStorePass->setChecked(config.readNumEntry("StorePass", 0));
	togglePopPass();
	
	grid->addWidget(fStorePass,8,1);

	setMode(PopMailConduit::RetrievalMode(
		config.readNumEntry("SyncIncoming",
			PopMailConduit::RECV_NONE)));
}

/* virtual */ int PopMailReceivePage::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;
	config.writeEntry("UNIX Mailbox", fMailbox->text());

	config.writeEntry("PopServer", fPopServer->text().latin1());
	config.writeEntry("PopPort", atoi(fPopPort->text().latin1()));
	config.writeEntry("PopUser", fPopUser->text().latin1());
	config.writeEntry("LeaveMail", (int)fLeaveMail->isChecked());
	config.writeEntry("StorePass", (int)fStorePass->isChecked());
	config.sync(); 
	//
	// Make sure permissions are safe (still not a good idea)
	//
	if(fStorePass->isChecked()) 
	{
		chmod(KGlobal::dirs()->findResource("config", "kpilotrc")
			.latin1(), 0600);
		config.writeEntry("PopPass", fPopPass->text().latin1());
	}
	else
	{
		config.writeEntry("PopPass",QString::null);
	}

	config.writeEntry("SyncIncoming", (int)getMode());
	config.sync();

	return 0;
}

/* slot */ void PopMailReceivePage::toggleMode()
{
	if (fNoReceive->isChecked()) setMode(PopMailConduit::RECV_NONE);
	if (fReceivePOP->isChecked()) setMode(PopMailConduit::RECV_POP);
	if (fReceiveUNIX->isChecked()) setMode(PopMailConduit::RECV_UNIX);
}

void PopMailReceivePage::setMode(PopMailConduit::RetrievalMode m)
{
	FUNCTIONSETUP;

	switch(m)
	{
	case PopMailConduit::RECV_NONE :
		fMailbox->setEnabled(false);
		fPopServer->setEnabled(false);
		fPopPort->setEnabled(false);
		fPopUser->setEnabled(false);
		fLeaveMail->setEnabled(false);
		fStorePass->setEnabled(false);
		fPopPass->setEnabled(false);
		fNoReceive->setChecked(true);
		break;
	case PopMailConduit::RECV_POP :
		fMailbox->setEnabled(false);
		fPopServer->setEnabled(true);
		fPopPort->setEnabled(true);
		fPopUser->setEnabled(true);
		fLeaveMail->setEnabled(true);
		fStorePass->setEnabled(true);
		togglePopPass();
		fReceivePOP->setChecked(true);
		break;
	case PopMailConduit::RECV_UNIX :
		fMailbox->setEnabled(true);
		fPopServer->setEnabled(false);
		fPopPort->setEnabled(false);
		fPopUser->setEnabled(false);
		fLeaveMail->setEnabled(false);
		fStorePass->setEnabled(false);
		fPopPass->setEnabled(false);
		fReceiveUNIX->setChecked(true);
		break;
	default :
		cerr << __FUNCTION__
			<< ": Unknown mode " << (int) m
			<< endl;
		return;
	}

	fMode=m;
}

/* slot */ void PopMailReceivePage::browseMailbox()
{
	FUNCTIONSETUP;

	QString filename=fMailbox->text();

#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Mailbox currently "
			<< fMailbox->text() << endl;
	}
#endif

	if (filename.isEmpty()) 
	{
		filename=QDir::currentDirPath();
	}
	else
	{
		filename=QFileInfo( filename ).dirPath();
	}

	filename = QFileDialog::getOpenFileName(filename,"*");

#ifdef DEBUG
	if (debug_level & UI_MINOR)
	{
		kdDebug() << fname << ": Mailbox selected "
			<< filename << endl;
	}
#endif

	if (!filename.isEmpty())
	{
		fMailbox->setText(filename);
	}
}

void PopMailReceivePage::togglePopPass()
{
	FUNCTIONSETUP;

	if(fStorePass->isChecked())
	{
		fPopPass->setEnabled(true);
	}
	else
	{
		fPopPass->setEnabled(false);
	}
}


/* static */ const QString PopMailOptions::PopGroup("popmailOptions");

PopMailOptions::PopMailOptions(QWidget *parent) :
	setupDialog(parent, PopGroup,
		PopMailConduit::version())
{
	FUNCTIONSETUP;
	setupWidget();
	setupDialog::setupWidget();
}

PopMailOptions::~PopMailOptions()
{
	FUNCTIONSETUP;

}


  




void
PopMailOptions::setupWidget()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotConfig::getConfig();
	config.setGroup(PopGroup);


	addPage(new PopMailSendPage(this,config));
	addPage(new PopMailReceivePage(this,config));
	addPage(new setupInfoPage(this));
}


// $Log$
// Revision 1.15  2001/03/09 09:46:14  adridg
// Large-scale #include cleanup
//
// Revision 1.14  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.13  2001/02/09 15:59:28  habenich
// replaced "char *id" with "char *<filename>_id", because of --enable-final in configure
//
// Revision 1.12  2001/02/07 15:46:31  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
// Revision 1.11  2001/01/03 00:05:13  adridg
// Administrative
//
