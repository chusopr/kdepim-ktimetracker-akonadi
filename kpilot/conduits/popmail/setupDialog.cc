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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
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

#include <kconfig.h>
#include <kstddirs.h>
#include <klineedit.h>

#include <qcheckbox.h>
#include <qdir.h>
#include <qcombobox.h>

#undef OLDSTYLE
#ifdef OLDSTYLE
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qfiledlg.h>
#include <qbttngrp.h>
#include <qpushbutton.h>
#include <qradiobt.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#endif

#include "kfiledialog.h"

#include <kurlrequester.h>

#include "uiDialog.h"

#include "popmail-factory.h"
#include "setup-dialog.h"
#include "setupDialog.moc"
#include "popmailSettings.h"

#if 0
PopMailSendPage::PopMailSendPage(QWidget *parent) :
	QWidget(parent,"SendMail")
{
	FUNCTIONSETUP;
	QGridLayout *grid=new QGridLayout(this,6,3,SPACING);
	QLabel *currentLabel;

	sendGroup=new QVButtonGroup(i18n("Send Method"),
		this,"sb");

	fNoSend=new QRadioButton(i18n("&Do not send mail"),sendGroup);
	fSendmail=new QRadioButton(i18n("Use &sendmail"),sendGroup);
	fSMTP=new QRadioButton(i18n("Use S&MTP"),sendGroup);
	fKMail=new QRadioButton(i18n("Use &KMail"),sendGroup);

	connect(fNoSend,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fSMTP,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fSendmail,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fKMail,SIGNAL(clicked()),
		this,SLOT(toggleMode()));

	sendGroup->adjustSize();

	grid->addMultiCellWidget(sendGroup,0,0,0,2);
	

	//-----------------------------------------------
	//
	// Sending mail options.
	//
	currentLabel = new QLabel(i18n("Email address: "),
			    this);

	fEmailFrom = new QLineEdit(this);
	fEmailFrom->resize(200, fEmailFrom->height());

	grid->addWidget(currentLabel,1,0);
	grid->addWidget(fEmailFrom,1,1);

	currentLabel = new QLabel(i18n("Signature file: "),
			    this);
	currentLabel->adjustSize();

	fSignature = new QLineEdit(this);
	fSignature->resize(200, fSignature->height());

	fSignatureBrowse=new QPushButton(i18n("Browse..."),this);
	fSignatureBrowse->adjustSize();

	connect(fSignatureBrowse,SIGNAL(clicked()),
		this,SLOT(browseSignature()));

	grid->addWidget(currentLabel,2,0);
	grid->addWidget(fSignature,2,1);
	grid->addWidget(fSignatureBrowse,2,2);

	currentLabel = new QLabel(i18n("Sendmail command:"), this);
	currentLabel->adjustSize();

	fSendmailCmd = new QLineEdit(this);
	fSendmailCmd->resize(300, fSendmailCmd->height());

	grid->addWidget(currentLabel,4,0);
	grid->addWidget(fSendmailCmd,4,1);

	currentLabel = new QLabel(i18n("SMTP server:"), this);
	currentLabel->adjustSize();

	fSMTPServer = new QLineEdit(this);
	fSMTPServer->resize(200, fSendmailCmd->height());

	grid->addWidget(currentLabel,6,0);
	grid->addWidget(fSMTPServer,6,1);

	currentLabel = new QLabel(i18n("SMTP port:"), this);
	currentLabel->adjustSize();

	fSMTPPort = new QLineEdit(this);
	fSMTPPort->resize(200, fSendmailCmd->height());

	grid->addWidget(currentLabel,7,0);
	grid->addWidget(fSMTPPort,7,1);

	currentLabel = new QLabel(i18n("Firewall:"), this);
	currentLabel->adjustSize();

	fFirewallFQDN = new QLineEdit(this);
	fFirewallFQDN->resize(200, fSendmailCmd->height());

	grid->addWidget(currentLabel,9,0);
	grid->addWidget(fFirewallFQDN,9,1);

	fKMailSendImmediate = new QCheckBox(
		i18n("Send mail through KMail immediately"),
		this);
	grid->addRowSpacing(10,SPACING);
	grid->addWidget(fKMailSendImmediate,11,1);
	QWhatsThis::add(fKMailSendImmediate,
		i18n("<qt>Check this box if you want the conduit "
			"to send all items in the outbox as soon "
			"as it is done, as if you clicked KMail's "
			"File->Send Queued menu item.</qt>"));



	(void) setupDialog_id;
}

void PopMailSendPage::readSettings(KConfig &config)
{
	fEmailFrom->setText(config.readEntry("EmailAddress", CSL1("$USER")));
	fSignature->setText(config.readPathEntry("Signature"));
	fSendmailCmd->setText(config.readPathEntry("SendmailCmd",
		CSL1("/usr/lib/sendmail -t -i")));
	fSMTPServer->setText(config.readEntry("SMTPServer", CSL1("mail")));
	fSMTPPort->setText(QString::number(config.readNumEntry("SMTPPort", 25)));
	fFirewallFQDN->setText(config.readEntry("explicitDomainName", CSL1("$MAILDOMAIN")));
	fKMailSendImmediate->setChecked(config.readBoolEntry("SendImmediate",
		true));
	setMode(SendMode(config.readNumEntry(PopMailConduitFactory::syncOutgoing,SEND_NONE)));
}

/* virtual */ int PopMailSendPage::commitChanges(KConfig& config)
{
	FUNCTIONSETUP;

#if 0
	if (parentSetup->queryFile(i18n("Signature File %1 is missing."),
		fSignature->text())!=KMessageBox::No)
#endif

	{
			config.writePathEntry("Signature", fSignature->text());
	}

	config.writeEntry("EmailAddress", fEmailFrom->text());


	config.writePathEntry("SendmailCmd", fSendmailCmd->text());
	config.writeEntry("SMTPServer", fSMTPServer->text());
	config.writeEntry("SMTPPort", fSMTPPort->text());
	config.writeEntry("explicitDomainName", fFirewallFQDN->text());

	config.writeEntry(PopMailConduitFactory::syncOutgoing, (int)getMode());

	config.writeEntry("SendImmediate", fKMailSendImmediate->isChecked());
	return 0;
}


/* slot */ void PopMailSendPage::toggleMode()
{
	if (fNoSend->isChecked()) setMode(SEND_NONE);
	if (fSendmail->isChecked()) setMode(SEND_SENDMAIL);
	if (fSMTP->isChecked()) setMode(SEND_SMTP);
	if (fKMail->isChecked()) setMode(SEND_KMAIL);
}

void PopMailSendPage::setMode(SendMode m)
{
	FUNCTIONSETUP;

	switch(m)
	{
	case SEND_SENDMAIL :
		fSendmailCmd->setEnabled(true);
		fSMTPServer->setEnabled(false);
		fSMTPPort->setEnabled(false);
		fKMailSendImmediate->setEnabled(false);
		fSendmail->setChecked(true);
		break;
	case SEND_SMTP :
		fSendmailCmd->setEnabled(false);
		fSMTPServer->setEnabled(true);
		fSMTPPort->setEnabled(true);
		fKMailSendImmediate->setEnabled(false);
		fSMTP->setChecked(true);
		break;
	case SEND_KMAIL :
		fSendmailCmd->setEnabled(false);
		fSMTPServer->setEnabled(false);
		fSMTPPort->setEnabled(false);
		fKMailSendImmediate->setEnabled(true);
		fKMail->setChecked(true);
		break;
	case SEND_NONE :
		fSendmailCmd->setEnabled(false);
		fSMTPServer->setEnabled(false);
		fSMTPPort->setEnabled(false);
		fKMailSendImmediate->setEnabled(false);
		fNoSend->setChecked(true);
		break;
	default :
		kdWarning() << k_funcinfo
			<< ": Unknown mode "
			<< (int) m
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
	{
		DEBUGCONDUIT << fname << ": Signature currently "
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

	filename = KFileDialog::getOpenFileName(filename,CSL1("*"));

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname << ": Signature selected "
			<< filename << endl;
	}
#endif

	if (!filename.isEmpty())
	{
		fSignature->setText(filename);
	}
}



PopMailReceivePage::PopMailReceivePage(QWidget *parent) :
	QWidget(parent,"RecvMail")
{
	FUNCTIONSETUP;
	QLabel *currentLabel;
	QGridLayout *grid=new QGridLayout(this,8,3,SPACING);

	methodGroup=new QVButtonGroup(i18n("Retrieve Method"),
		this,"bg");

	fNoReceive=new QRadioButton(i18n("Do &not receive mail"),
		methodGroup);
	fReceivePOP=new QRadioButton(i18n("Use &POP3 server"),
		methodGroup);
	fReceiveUNIX=new QRadioButton(i18n("Use &UNIX mailbox"),
		methodGroup);

	connect(fNoReceive,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fReceivePOP,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	connect(fReceiveUNIX,SIGNAL(clicked()),
		this,SLOT(toggleMode()));
	methodGroup->adjustSize();


	grid->addMultiCellWidget(methodGroup,0,0,0,2);

	currentLabel = new QLabel(i18n("UNIX mailbox:"),this);
	currentLabel->adjustSize();

	fMailbox=new QLineEdit(this);
	fMailbox->resize(200,fMailbox->height());

	fMailboxBrowse=new QPushButton(i18n("Browse..."),this);
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

	currentLabel = new QLabel(i18n("POP server:"), this);
	currentLabel->adjustSize();

	fPopServer = new QLineEdit(this);
	fPopServer->resize(200, fPopServer->height());

	grid->addWidget(currentLabel,3,0);
	grid->addWidget(fPopServer,3,1);

	currentLabel = new QLabel(i18n("POP port:"), this);
	currentLabel->adjustSize();

	fPopPort = new QLineEdit(this);
	fPopPort->resize(200, fPopPort->height());

	grid->addWidget(currentLabel,4,0);
	grid->addWidget(fPopPort,4,1);

	currentLabel = new QLabel(i18n("POP username:"), this);
	currentLabel->adjustSize();

	fPopUser = new QLineEdit(this);
	fPopUser->resize(200, fPopUser->height());

	grid->addWidget(currentLabel,5,0);
	grid->addWidget(fPopUser,5,1);

	fLeaveMail = new QCheckBox(i18n("&Leave mail on server"), this);
	fLeaveMail->adjustSize();

	grid->addWidget(fLeaveMail,6,1);

	currentLabel = new QLabel(i18n("POP password:"), this);
	currentLabel->adjustSize();

	fPopPass = new QLineEdit(this);
	fPopPass->setEchoMode(QLineEdit::Password);
	fPopPass->resize(200, fPopPass->height());


	grid->addWidget(currentLabel,7,0);
	grid->addWidget(fPopPass,7,1);


	fStorePass = new QCheckBox(i18n("Save &POP password"), this);
	connect(fStorePass, SIGNAL(clicked()), this, SLOT(togglePopPass()));
	fStorePass->adjustSize();
	togglePopPass();

	grid->addWidget(fStorePass,8,1);

}

void PopMailReceivePage::readSettings(KConfig &config)
{
	FUNCTIONSETUP;

	QString defaultMailbox;
	char *u=getenv("USER");
	if (u==0L)
	{
		u=getenv("HOME");
		if (u==0L)
		{
			defaultMailbox=CSL1("mbox");
		}
		else
		{
			defaultMailbox=QString::fromLocal8Bit(u)+CSL1("mbox");
		}
	}
	else
	{
		defaultMailbox=CSL1("/var/spool/mail/")+QString::fromLocal8Bit(u);
	}

	fMailbox->setText(config.readEntry("UNIX Mailbox",defaultMailbox));
	fPopServer->setText(config.readEntry("PopServer", CSL1("pop")));
	fPopPort->setText(config.readEntry("PopPort", CSL1("110")));
	fPopUser->setText(config.readEntry("PopUser", CSL1("$USER")));
	fLeaveMail->setChecked(config.readNumEntry("LeaveMail", 1));
	fPopPass->setText(config.readEntry("PopPass"));
	fPopPass->setEnabled(config.readNumEntry("StorePass", 0));
	fStorePass->setChecked(config.readNumEntry("StorePass", 0));
	setMode(RetrievalMode(
		config.readNumEntry(PopMailConduitFactory::syncIncoming,RECV_NONE)));
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
		chmod(KGlobal::dirs()->findResource("config", CSL1("kpilotrc"))
			.latin1(), 0600);
		config.writeEntry("PopPass", fPopPass->text());
	}
	else
	{
		config.writeEntry("PopPass",QString::null);
	}

	config.writeEntry(PopMailConduitFactory::syncIncoming, (int)getMode());
	config.sync();

	return 0;
}

/* slot */ void PopMailReceivePage::toggleMode()
{
	if (fNoReceive->isChecked()) setMode(RECV_NONE);
	if (fReceivePOP->isChecked()) setMode(RECV_POP);
	if (fReceiveUNIX->isChecked()) setMode(RECV_UNIX);
}

void PopMailReceivePage::setMode(RetrievalMode m)
{
	FUNCTIONSETUP;

	switch(m)
	{
	case RECV_NONE :
		fMailbox->setEnabled(false);
		fPopServer->setEnabled(false);
		fPopPort->setEnabled(false);
		fPopUser->setEnabled(false);
		fLeaveMail->setEnabled(false);
		fStorePass->setEnabled(false);
		fPopPass->setEnabled(false);
		fNoReceive->setChecked(true);
		break;
	case RECV_POP :
		fMailbox->setEnabled(false);
		fPopServer->setEnabled(true);
		fPopPort->setEnabled(true);
		fPopUser->setEnabled(true);
		fLeaveMail->setEnabled(true);
		fStorePass->setEnabled(true);
		togglePopPass();
		fReceivePOP->setChecked(true);
		break;
	case RECV_UNIX :
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
		kdWarning() << k_funcinfo
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
	{
		DEBUGCONDUIT << fname << ": Mailbox currently "
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

	filename = KFileDialog::getOpenFileName(filename,CSL1("*"));

#ifdef DEBUG
	{
		DEBUGCONDUIT << fname << ": Mailbox selected "
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
#endif


PopMailWidgetConfig::PopMailWidgetConfig(QWidget *p,const char *n) :
	ConduitConfigBase(p,n),
	fConfigWidget(new PopMailWidget(p,"PopMailWidget"))
{
	FUNCTIONSETUP;
	fConduitName = i18n("Popmail");
	UIDialog::addAboutPage(fConfigWidget->fTabWidget,PopMailConduitFactory::about());
	fWidget=fConfigWidget;

#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fStorePass,SIGNAL(toggled(bool)));
	CM(fPopPass,SIGNAL(textChanged(const QString &)));
	CM(fRecvMode,SIGNAL(activated(int)));
	CM(fSendMode,SIGNAL(activated(int)));
#undef CM

	connect(fConfigWidget->fStorePass,SIGNAL(toggled(bool)),
		fConfigWidget->fPopPass,SLOT(setEnabled(bool)));
	connect(fConfigWidget->fRecvMode,SIGNAL(activated(int)),
		this,SLOT(toggleRecvMode(int)));
	connect(fConfigWidget->fSendMode,SIGNAL(activated(int)),
		this,SLOT(toggleSendMode(int)));
}

void PopMailWidgetConfig::commit()
{
	FUNCTIONSETUP;
	KConfig*fConfig=MailConduitSettings::self()->config();
	KConfigGroupSaver s(fConfig,PopMailConduitFactory::group());
#define WR(a,b,c) fConfig->writeEntry(c,fConfigWidget->a->b);
	WR(fSendMode,currentItem(),PopMailConduitFactory::syncIncoming());
	WR(fEmailFrom,text(),"EmailAddress");
	WR(fSignature,url(),"Signature");
	WR(fLeaveMail,isChecked(),"LeaveMail");
#undef WR
}

void PopMailWidgetConfig::load()
{
	FUNCTIONSETUP;
	KConfig*fConfig=MailConduitSettings::self()->config();
	KConfigGroupSaver s(fConfig,PopMailConduitFactory::group());
#define RD(a,b,c,d,e) fConfigWidget->a->b(fConfig->read##c##Entry(d,e))
	RD(fSendMode,setCurrentItem,Num,PopMailConduitFactory::syncIncoming(),(int)NoSend);
	RD(fEmailFrom,setText,,"EmailAddress",QString::null);
	RD(fSignature,setURL,,"Signature",CSL1("$HOME/.signature"));
	RD(fLeaveMail,setChecked,Bool,"LeaveMail",true);
#undef RD

	/**
	*
	* Disable sending through anything but KMail,
	* and totally disable receiving.
	*/
	fConfigWidget->fSendMode->setCurrentItem(3);
	fConfigWidget->fRecvMode->setCurrentItem(0);
	
	toggleSendMode(fConfigWidget->fSendMode->currentItem());
	toggleRecvMode(fConfigWidget->fRecvMode->currentItem());
}


/* slot */ void PopMailWidgetConfig::toggleRecvMode(int i)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Got mode " << i << endl;
#endif

#define E(a,b) fConfigWidget->a->setEnabled(b)
	switch(i)
	{
	case RecvPOP :
		E(fPopPass,true);
		E(fStorePass,true);
		E(fPopServer,true);
		E(fPopUser,true);
		E(fLeaveMail,true);
		E(fMailbox,false);
		break;
	case RecvMBOX :
		E(fPopPass,false);
		E(fStorePass,false);
		E(fPopServer,false);
		E(fPopUser,false);
		E(fLeaveMail,false);
		E(fMailbox,true);
		break;
	case NoRecv : /* FALLTHRU */
	default :
		E(fPopPass,false);
		E(fStorePass,false);
		E(fPopServer,false);
		E(fPopUser,false);
		E(fLeaveMail,false);
		E(fMailbox,false);
		break;
	}
#undef E
}

/* slot */ void PopMailWidgetConfig::toggleSendMode(int i)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Got mode " << i << endl;
#endif

#define E(a,b) fConfigWidget->a->setEnabled(b)
	switch(i)
	{
	case SendKMail :
		E(fEmailFrom,true);
		E(fSignature,true);
		E(fSMTPServer,false);
		E(fSendmailCmd,false);
		break;
	case SendSMTP :
		E(fEmailFrom,true);
		E(fSignature,true);
		E(fSMTPServer,true);
		E(fSendmailCmd,false);
		break;
	case SendSendmail :
		E(fEmailFrom,true);
		E(fSignature,true);
		E(fSMTPServer,false);
		E(fSendmailCmd,true);
		break;
	case NoSend : /* FALLTHRU */
	default :
		E(fEmailFrom,false);
		E(fSignature,false);
		E(fSMTPServer,false);
		E(fSendmailCmd,false);
		break;
	}
#undef E
}



