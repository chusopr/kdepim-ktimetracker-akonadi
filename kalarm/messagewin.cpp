/*
 *  messagewin.cpp  -  displays an alarm message
 *  Program:  kalarm
 *
 *  (C) 2001 by David Jarvie  software@astrojar.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *
 */

#include "kalarm.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include <kstddirs.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kwin.h>
#include <kprocess.h>
#include <kio/netaccess.h>
#include <knotifyclient.h>
#include <kaudioplayer.h>
#include <kdebug.h>

#include "kalarmapp.h"
#include "prefsettings.h"
#include "datetime.h"
#include "messagewin.h"
#include "messagewin.moc"


static const int MAX_LINE_LENGTH = 80;    // maximum width (in characters) to try to display


/******************************************************************************
*  Construct the message window.
*/
MessageWin::MessageWin(const MessageEvent& event, bool reschedule_event)
	: KMainWindow(0L, "MessageWin", WStyle_StaysOnTop | WDestructiveClose | WGroupLeader),
	  message(event.messageIsFileName() ? event.fileName() : event.message()),
	  font(theApp()->generalSettings()->messageFont()),
	  colour(event.colour()),
	  dateTime(event.dateTime()),
	  eventID(event.VUID()),
	  repeats(event.repeatCount()),
	  flags(event.flags()),
	  audioFile(event.alarm()->audioFile()),
	  beep(event.beep()),
	  file(event.messageIsFileName()),
	  rescheduleEvent(reschedule_event),
	  shown(false),
	  deferDlgShown(false)
{
	kdDebug() << "MessageWin::MessageWin(event)" << endl;
	setAutoSaveSettings(QString::fromLatin1("MessageWindow"));     // save window sizes etc.
	QSize size = initView();
	if (file  &&  !fileError)
		size = theApp()->readConfigWindowSize("FileMessage", size);
	resize(size);
}

/******************************************************************************
*  Construct the message window for restoration by session management.
*/
MessageWin::MessageWin()
	: KMainWindow(0L, "MessageWin", WStyle_StaysOnTop | WDestructiveClose),
	  shown(true)
{
	kdDebug() << "MessageWin::MessageWin()" << endl;
}

MessageWin::~MessageWin()
{
}

/******************************************************************************
*  Construct the message window.
*/
QSize MessageWin::initView()
{
	fileError = false;
	setCaption(i18n("Message"));
	QWidget* topWidget = new QWidget(this, "MessageWinTop");
	setCentralWidget(topWidget);
	topWidget->setBackgroundColor(colour);
	QVBoxLayout* topLayout = new QVBoxLayout(topWidget, KDialog::marginHint(), KDialog::spacingHint());

	if (dateTime.isValid())
	{
		// Alarm date/time
		QLabel* label = new QLabel(topWidget);
		label->setText(KGlobal::locale()->formatDateTime(dateTime));
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		label->setFixedSize(label->sizeHint());
		topLayout->addWidget(label);
		QWhatsThis::add(label,
		      i18n("The scheduled date/time for the message\n"
		           "(as opposed to the actual time of display)."));
	}

	if (file)
	{
		// Display the file name
		QLabel* label = new QLabel(topWidget);
		label->setText(message);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		label->setFixedSize(label->sizeHint());
		topLayout->addWidget(label);
		QWhatsThis::add(label, i18n("The file whose contents are displayed below"));

		// Display contents of file
		bool opened = false;
		bool dir = false;
		QString tmpFile;
		KURL url(message);
		if (KIO::NetAccess::download(url, tmpFile))
		{
			QFile qfile(tmpFile);
			QFileInfo info(qfile);
			if (!(dir = info.isDir())
			&&  qfile.open(IO_ReadOnly|IO_Translate))
			{
				opened = true;
				QTextView* view = new QTextView(this, "fileContents");
				topLayout->addWidget(view);
				QFontMetrics fm = view->fontMetrics();
				QString line;
				int n;
				for (n = 0;  qfile.readLine(line, 4096) > 0;  ++n)
				{
					int nl = line.find('\n');
					if (nl >= 0)
						line = line.left(nl);
					view->append(line);
				}
				qfile.close();
				view->setMinimumSize(view->sizeHint());

				// Set the default size to square, max 20 lines.
				// Note that after the first file has been displayed, this size
				// is overridden by the user-set default stored in the config file.
				// So there is no need to calculate an accurate size.
				int h = fm.lineSpacing() * (n <= 20 ? n : 20) + 2*view->frameWidth();
				view->resize(QSize(h, h).expandedTo(view->sizeHint()));
			}
			KIO::NetAccess::removeTempFile(tmpFile);
		}
		if (!opened)
		{
			// File couldn't be opened
			bool exists = KIO::NetAccess::exists(url);
			QLabel* label = new QLabel(topWidget);
			label->setText(dir ? i18n("Error: File is a directory") : exists ? i18n("Error opening file !!") : i18n("Error: File not found !!"));
			label->setPalette(QPalette(colour, colour));
			label->setFixedSize(label->sizeHint());
			topLayout->addWidget(label);
			fileError = true;
		}
	}
	else
	{
		// Message label
		QLabel* label = new QLabel(topWidget);
		label->setText(message);
		label->setFont(font);
		label->setPalette(QPalette(colour, colour));
		label->setFixedSize(label->sizeHint());
		int spacing = label->fontMetrics().lineSpacing()/2 - KDialog::spacingHint();
		topLayout->addSpacing(spacing);
		topLayout->addWidget(label);
		topLayout->addSpacing(spacing);
	}

	QGridLayout* grid = new QGridLayout(1, 4);
	topLayout->addLayout(grid);
	grid->setColStretch(0, 1);     // keep the buttons right-adjusted in the window

	// OK button
	QPushButton* okButton = new QPushButton(i18n("&OK"), topWidget);
	okButton->setDefault(true);
	connect(okButton, SIGNAL(clicked()), SLOT(close()));
	grid->addWidget(okButton, 0, 1, AlignHCenter);
	QWhatsThis::add(okButton, i18n("Acknowledge the alarm"));

	// Defer button
	deferButton = new QPushButton(i18n("&Defer..."), topWidget);
	connect(deferButton, SIGNAL(clicked()), SLOT(slotShowDefer()));
	grid->addWidget(deferButton, 0, 2, AlignHCenter);
	QWhatsThis::add(deferButton,
	      i18n("Defer the alarm until later.\n"
	           "You will be prompted to specify when\n"
	           "the alarm should be redisplayed"));

	// KAlarm button
	KIconLoader iconLoader;
	QPixmap pixmap = iconLoader.loadIcon(PROGRAM_NAME, KIcon::MainToolbar);
	QPushButton* button = new QPushButton(topWidget);
	button->setPixmap(pixmap);
	button->setFixedSize(button->sizeHint());
	connect(button, SIGNAL(clicked()), SLOT(slotKAlarm()));
	grid->addWidget(button, 0, 3, AlignHCenter);
	QWhatsThis::add(button, i18n("Activate KAlarm"));

	// Set the button sizes
	button = new QPushButton(i18n("ABCDEF"), topWidget);
	QSize minbutsize = button->sizeHint();
	delete button;
	minbutsize = minbutsize.expandedTo(okButton->sizeHint());
	minbutsize = minbutsize.expandedTo(deferButton->sizeHint());
	okButton->setFixedSize(minbutsize);
	deferButton->setFixedSize(minbutsize);

	topLayout->activate();
	QSize size(minbutsize.width()*3, topLayout->sizeHint().height());
	setMinimumSize(size);

//KWin::setType(winId(), NET::Dialog);     // display Help button in the title bar
	KWin::setState(winId(), NET::Modal | NET::Sticky | NET::StaysOnTop);
	KWin::setOnAllDesktops(winId(), true);
	return size;
}

/******************************************************************************
* Save settings to the session managed config file, for restoration
* when the program is restored.
*/
void MessageWin::saveProperties(KConfig* config)
{
	config->writeEntry("Message", message);
	config->writeEntry("Font", font);
	config->writeEntry("Colour", colour);
	if (dateTime.isValid())
		config->writeEntry("Time", dateTime);
}

/******************************************************************************
* Read settings from the session managed config file.
* This function is automatically called whenever the app is being
* restored. Read in whatever was saved in saveProperties().
*/
void MessageWin::readProperties(KConfig* config)
{
	message  = config->readEntry("Message");
	font     = config->readFontEntry("Font");
	colour   = config->readColorEntry("Colour");
	QDateTime invalidDateTime;
	dateTime = config->readDateTimeEntry("Time", &invalidDateTime);
	initView();
}

/******************************************************************************
*  Called when the window is shown.
*  The first time, output any required audio notification, and delete the event
*  from the calendar file.
*/
void MessageWin::showEvent(QShowEvent* se)
{
	KMainWindow::showEvent(se);
	if (!shown)
	{
		if (beep)
		{
			// Beep using two methods, in case the sound card/speakers are switched off or not working
			KNotifyClient::beep();     // beep through the sound card & speakers
			QApplication::beep();      // beep through the internal speaker
		}
		if (!audioFile.isEmpty())
			KAudioPlayer::play(audioFile.latin1());
		if (rescheduleEvent)
			theApp()->rescheduleMessage(eventID);
		shown = true;
	}
}

/******************************************************************************
*  Called when the window's size has changed (before it is painted).
*/
void MessageWin::resizeEvent(QResizeEvent* re)
{
	if (file  &&  !fileError  &&  !deferDlgShown)
		theApp()->writeConfigWindowSize("FileMessage", re->size());
	KMainWindow::resizeEvent(re);
}

/******************************************************************************
*  Called when the Defer... button is clicked.
*  Displays the defer message dialog.
*/
void MessageWin::slotShowDefer()
{
	if (!deferDlgShown)
	{
		delete deferButton;
		QWidget* deferDlg = new QWidget(this);
		QVBoxLayout* wlayout = new QVBoxLayout(deferDlg, KDialog::spacingHint());
		QGridLayout* grid = new QGridLayout(2, 1, KDialog::spacingHint());
		wlayout->addLayout(grid);
		deferTime = new AlarmTimeWidget(true, deferDlg, "deferTime");
//		deferTime = new AlarmTimeWidget(true, this, "deferTime");
		deferTime->setDateTime(QDateTime::currentDateTime());
		connect(deferTime, SIGNAL(deferred()), SLOT(slotDefer()));
		grid->addWidget(deferTime, 0, 0);
		if (repeats)
		{
			QLabel* warn = new QLabel(deferDlg);
			warn->setText(i18n("Note: deferring this alarm will also defer its repetitions"));
			warn->setFixedSize(warn->sizeHint());
			grid->addWidget(warn, 1, 0);
		}

		QSize s(deferDlg->sizeHint());
//		QSize s(deferTime->sizeHint());
		if (s.width() > width())
			resize(s.width(), height());     // this ensures that the background colour extends to edge
		else if (width() > s.width())
			s.setWidth(width());

		// Ensure that the defer dialog doesn't disappear past the bottom of the screen
		int maxHeight = KApplication::desktop()->height() - s.height();
		QRect rect = frameGeometry();
		if (rect.bottom() > maxHeight)
		{
			// Move the window upwards if possible, and resize if necessary
			int bottomShift = rect.bottom() - maxHeight;
			int topShift    = bottomShift;
			if (topShift > rect.top())
				topShift = rect.top();
			rect = geometry();
			rect.setTop(rect.top() - topShift);
			rect.setBottom(rect.bottom() - bottomShift);
			setGeometry(rect);
		}

		if (layout())
			layout()->setEnabled(false);
		deferDlg->setGeometry(0, height(), s.width(), s.height());
//		deferTime->setGeometry(0, height(), s.width(), s.height());
		setFixedSize(s.width(), height() + s.height());
		deferDlg->show();
//		deferTime->show();
		deferDlgShown = true;
	}
}

/******************************************************************************
*  Called when the Defer button is clicked to defer the alarm.
*/
void MessageWin::slotDefer()
{
	QDateTime dateTime;
	if (deferTime->getDateTime(dateTime))
	{
		// Get the event being deferred. It will only still exist if repetitions are outstanding.
		const MessageEvent* event = theApp()->getCalendar().getEvent(eventID);
		if (event)
		{
			// It's a repeated alarm which may still exist in the calendar file
			MessageEvent* newEvent = new MessageEvent(*event);   // event instance will belong to the calendar
			newEvent->set(dateTime, (flags & MessageEvent::LATE_CANCEL));
			theApp()->modifyMessage(const_cast<MessageEvent*>(event), newEvent, 0L);
		}
		else
		{
			// The event doesn't exist any more, so create a new one
			MessageEvent* event = new MessageEvent(dateTime, flags, colour, message, file);
			theApp()->addMessage(event, 0L);
		}
		close();
	}
}

/******************************************************************************
*  Called when the KAlarm button is clicked.
*  Displays the KAlarm main window.
*/
void MessageWin::slotKAlarm()
{
	KProcess proc;
	proc << PROGRAM_NAME;
	proc.start(KProcess::DontCare);
}
