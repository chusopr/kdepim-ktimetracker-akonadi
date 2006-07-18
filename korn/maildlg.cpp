#include "maildlg.h"
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QDesktopWidget>
#include<kdebug.h>
#include<klocale.h>
#include<qapplication.h>
#include "mailsubject.h"
#include <QProgressDialog>
#include "maildrop.h"

KornMailDlg::KornMailDlg( QWidget *parent )
   : KDialog( parent ),
   _progress( 0 )
{
	setCaption( i18n("Mail Details") );
	setButtons( KDialog::User1|KDialog::Close );
	setButtonGuiItem( User1, KGuiItem(i18n("&Full Message")) );
	setModal( true );
	showButtonSeparator( true );
	QWidget * page = new QWidget( this );
	setMainWidget(page);
	QVBoxLayout * topLayout = new QVBoxLayout( page );
	topLayout->setMargin( 0 );
	topLayout->setSpacing( spacingHint() );
	_kTextEdit = new KTextEdit(page);
	topLayout->addWidget(_kTextEdit, 10);
	_kTextEdit->setReadOnly(true);
	connect(this, SIGNAL(user1Clicked()), this, SLOT(showFullMessage()));
	setInitialSize(QSize(QApplication::desktop()->width()*9/10, QApplication::desktop()->height()/2));
}

KornMailDlg::~KornMailDlg()
{
}

void KornMailDlg::loadMailCanceled()
{
	_loadMailCanceled = true;
}


void KornMailDlg::showFullMessage()
{
	_loadMailCanceled = false;
	
	// create progress dialog
	_progress = new QProgressDialog(i18n("Loading full mail. Please wait..."), "&Cancel", 0, 1000, this);
	_progress->setValue(1);
	qApp->processEvents();

	// connect the mailbox with the progress dialog in case it supports progress bars
	connect(_mailDrop, SIGNAL(readMailTotalSteps(int)), _progress, SLOT(setMaximum(int)));
	connect(_mailDrop, SIGNAL(readMailProgress(int)), _progress, SLOT(setValue(int)));
	qApp->processEvents();

	// connect the mailbox's cancel button
	connect(_progress, SIGNAL(canceled()), this, SLOT(loadMailCanceled()));
	
	connect(_mailDrop, SIGNAL(readMailReady(QString*)), this, SLOT(readMailReady(QString*)));

	// now load the mail fully
	if( _mailDrop->synchrone() )
	{
		QString mail = _mailDrop->readMail(_mailSubject->getId(), &_loadMailCanceled);
		readMailReady( &mail );
	}
	else
		_mailDrop->readMail(_mailSubject->getId(), &_loadMailCanceled);
}

void KornMailDlg::setMailSubject( KornMailSubject * mailSubject )
{
	_mailSubject = mailSubject;
	_mailDrop = mailSubject->getMailDrop();

	// show mail
	_kTextEdit->setPlainText(_mailSubject->getHeader());

	// disable "Full Message" button if mail is already loaded fully
	enableButton(User1, !_mailSubject->isHeaderFullMessage() && _mailDrop->canReadMail());
}

void KornMailDlg::readMailReady( QString* mail )
{
	deleteProgress();

	// if loading was not canceled and did not fail
	if ( mail->length() > 0)
	{
		// store full mail in KornMailSubject instance (so that it has not to be loaded again next time)
		_mailSubject->setHeader(*mail, true);

		// show fully loaded mail
		_kTextEdit->setPlainText(*mail);

		// disable "Full Message" button
		enableButton(User1, false);
	}
}

void KornMailDlg::deleteProgress()
{
	_progress->setValue(_progress->maximum());
	_progress->hide();
	
	disconnect( _mailDrop, SIGNAL(readMailReady(QString*)), this, SLOT(readMailReady(QString*)));
	
	delete _progress;
	_progress = 0;
}

#include "maildlg.moc"
