// $Id$

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qfont.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kstddirs.h>

#include "koprefs.h"

#include "kooptionsdialog.h"
#include "kooptionsdialog.moc"

//koConfig koconf;

KOOptionsDialog::KOOptionsDialog(QWidget *parent, char *name, bool modal) :
  KDialogBase(TreeList, i18n("Preferences"),Ok|Apply|Cancel|Default,Ok,parent,
              name,modal)
{
  setupMainTab();
  setupTimeTab();
  setupFontsTab();
  setupColorsTab();
  setupViewsTab();
  setupPrinterTab();

  readConfig();
  
  QObject::connect(this,SIGNAL(defaultClicked()),SLOT(setDefaults()));
  QObject::connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}


KOOptionsDialog::~KOOptionsDialog()
{
}


void KOOptionsDialog::setupMainTab()
{
  QFrame *topFrame = addPage(i18n("Personal"));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  topLayout->addWidget(new QLabel(i18n("Your name:"),topFrame),0,0);
  mNameEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mNameEdit,0,1);

  topLayout->addWidget(new QLabel(i18n("Email address:"),topFrame),1,0);
  mEmailEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mEmailEdit,1,1);
  
  topLayout->addWidget(new QLabel(i18n("Additional:"),topFrame),2,0);
  mAdditionalEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mAdditionalEdit,2,1);
  
  mAutoSaveCheck = new QCheckBox(i18n("Auto-save Calendar"),topFrame);
  topLayout->addMultiCellWidget(mAutoSaveCheck,3,3,0,1);
  mConfirmCheck = new QCheckBox(i18n("Confirm Deletes"),topFrame);
  topLayout->addMultiCellWidget(mConfirmCheck,4,4,0,1);
  
  const char *holidayList[] = { "(none)",
				"australia", "austria", "bavarian",
				"belgium", "canada", "catalan", "czechia", 
				"denmark", "dutch", "finnish", "french",
				"frswiss", "german", "hungary", "iceland",
				"italy", "japan",
				"norway", "portugal", "poland",
                                "quebec", "romania", "spain",
				"swedish", "thailand", "uk", "us", 0L };

  topLayout->addWidget(new QLabel(i18n("Holidays:"),topFrame),5,0);
  mHolidayCombo = new QComboBox(topFrame);
  mHolidayCombo->insertStrList(holidayList);
  topLayout->addWidget(mHolidayCombo,5,1);

  topLayout->setRowStretch(6,1);
}


void KOOptionsDialog::setupTimeTab()
{
  QFrame *topFrame = addPage(i18n("Time & Date"));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  mTimeFormatGroup = new QButtonGroup(1,Horizontal,i18n("Time Format:"),
                                      topFrame);
  (void)new QRadioButton(i18n("24 hour"),mTimeFormatGroup);
  (void)new QRadioButton(i18n("AM / PM"),mTimeFormatGroup);
  topLayout->addMultiCellWidget(mTimeFormatGroup,0,0,0,1);

  mDateFormatGroup = new QButtonGroup(1,Horizontal,i18n("Date Format:"),topFrame);
  (void)new QRadioButton(i18n("Month/Day/Year (01/31/2000)"),mDateFormatGroup);
  (void)new QRadioButton(i18n("Month-Day-Year (01-31-2000)"),mDateFormatGroup);
  topLayout->addMultiCellWidget(mDateFormatGroup,1,1,0,1);

  const char *tzList[] = { "-1200", "-1130", "-1100", "-1030", "-1000",
                           "-0930", "-0900", "-0830", "-0800", "-0730",
                           "-0700", "-0630", "-0600", "-0530", "-0500",
                           "-0430", "-0400", "-0330", "-0300", "-0230",
                           "-0200", "-0130", "-0100", "-0030", "+0000",
                           "+0030", "+0100", "+0130", "+0200", "+0230",
                           "+0300", "+0330", "+0400", "+0430", "+0500",
                           "+0530", "+0600", "+0630", "+0700", "+0730",
                           "+0800", "+0830", "+0900", "+0930", "+1000",
                           "+1030", "+1100", "+1130", "+1200", "+1230",
                           "+1300", "+1330", "+1400", 0L };

  topLayout->addWidget(new QLabel(i18n("TimeZone:"),topFrame),2,0);
  mTimeZoneCombo = new QComboBox(topFrame);
  mTimeZoneCombo->insertStrList(tzList);
  topLayout->addWidget(mTimeZoneCombo,2,1);

  topLayout->addWidget(new QLabel(i18n("Default Appointment Time:"),
                       topFrame),3,0);
  mStartTimeSpin = new QSpinBox(0,23,1,topFrame);
  mStartTimeSpin->setSuffix(":00");
  topLayout->addWidget(mStartTimeSpin,3,1);

  QStringList alarmList;
  
  alarmList << i18n("1 minute") << i18n("5 minutes") << i18n("10 minutes")
            << i18n("15 minutes") << i18n("30 minutes");

  topLayout->addWidget(new QLabel(i18n("Default Alarm Time:"),topFrame),
                       4,0);
  mAlarmTimeCombo = new QComboBox(topFrame);
  mAlarmTimeCombo->insertStringList(alarmList);
  topLayout->addWidget(mAlarmTimeCombo,4,1);
  
  mWeekstartCheck = new QCheckBox(i18n("Week Starts on Monday"),topFrame);
  topLayout->addWidget(mWeekstartCheck,5,0);
  
  topLayout->setRowStretch(6,1);
}


void KOOptionsDialog::setupViewsTab()
{
  QFrame *topFrame = addPage(i18n("Views"));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  topLayout->addWidget(new QLabel(i18n("Day Begins At:"),
                       topFrame),0,0);
  mDayBeginsSpin = new QSpinBox(0,23,1,topFrame);
  mDayBeginsSpin->setSuffix(":00");
  topLayout->addWidget(mDayBeginsSpin,0,1);

  QGroupBox *hourSizeGroup = new QGroupBox(1,Horizontal,
                                           i18n("Hour size in schedule view"),
                                           topFrame);
  mHourSizeSlider = new QSlider(0,100,5,10,Horizontal,hourSizeGroup);
  topLayout->addMultiCellWidget(hourSizeGroup,1,1,0,1);
  
  mDailyRecurCheck =
      new QCheckBox(i18n("Show events that recur daily in Date Navigator"),
                    topFrame);
  topLayout->addWidget(mDailyRecurCheck,2,0);
  
  topLayout->setRowStretch(3,1);
}


void KOOptionsDialog::setupFontsTab()
{
  QFrame *topFrame = addPage(i18n("Fonts"));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  mTimeBarFont = new QLabel("12:34",topFrame);
  mTimeBarFont->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  topLayout->addWidget(mTimeBarFont,0,0);

  QPushButton *buttonTimeBar = new QPushButton(i18n("Time Bar Font"),topFrame);
  topLayout->addWidget(buttonTimeBar,0,1);
  connect(buttonTimeBar,SIGNAL(clicked()),SLOT(selectTimeBarFont()));

  topLayout->setRowStretch(1,1);
  
// Disabled because font settings are not used. Has to be reimplemented.
#if 0
	QVBoxLayout *layout = new QVBoxLayout( fontsFrame, 4 );

	KPropFont *fontdlg = new KPropFont( fontsFrame, i18n("Appointment"), 
					    i18n("List view font"), 50, 
					    font(), "List Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("Appointment"), 
				 i18n("Schedule view font"), 50, 
				 font(), "Schedule Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("Appointment"), 
				 i18n("Month view font"), 50, 
				 font(), "Month Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("12345"), i18n("Time bar font"),
				 50, font(), "TimeBar Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("Things to do"),
				 i18n("To-Do list font"), 50,
				 font(), "Todo Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

//	layout->addStretch();	
#endif
}

void KOOptionsDialog::selectTimeBarFont()
{
  QFont theFont(mTimeBarFont->font());
  QString theText(mTimeBarFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mTimeBarFont->setFont(theFont);
}


void KOOptionsDialog::setupColorsTab()
{
  QFrame *topFrame = addPage(i18n("Colors"));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  mHolidayColor = new QFrame(topFrame);
  mHolidayColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mHolidayColor,0,0);

  QPushButton *buttonHolidayColor = new QPushButton(i18n("Holiday Color"),
                                                    topFrame);
  topLayout->addWidget(buttonHolidayColor,0,1);
  connect(buttonHolidayColor,SIGNAL(clicked()),SLOT(selectHolidayColor()));

  mHighlightColor = new QFrame(topFrame);
  mHighlightColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mHighlightColor,1,0);

  QPushButton *buttonHighlightColor = new QPushButton(i18n("Highlight Color"),
                                                    topFrame);
  topLayout->addWidget(buttonHighlightColor,1,1);
  connect(buttonHighlightColor,SIGNAL(clicked()),SLOT(selectHighlightColor()));

  topLayout->setRowStretch(2,1);
  

// Disabled because color settings are not used. Has to be reimplemented.
#if 0
//	KPropColor *color;
	KPropGroup *group;
	
	QVBoxLayout *layout = new QVBoxLayout( colorsFrame, 4 );
/*
	QPushButton *button = new QPushButton( "Set System Default Colors", colorsFrame );
	button->setFixedSize( button->sizeHint() );
	connect( button, SIGNAL( clicked() ), SLOT( setColorDefaults() ) );
	layout->addWidget( button,0,AlignLeft );
*/
	QCheckBox *checkbox = new KPropCheck( colorsFrame, i18n("Use system default colors"), "DefaultColors", "Colors");  
	connectConfig( checkbox );
	connect( checkbox, SIGNAL( toggled(bool) ), SLOT( toggleSystemColors(bool) ) );
	layout->addWidget( checkbox );
	
	layout->addWidget( group = new KPropGroup( colorsFrame, i18n("Appointment & Checklist items")) );
	group->addWidget( color1 = new KPropColor( group, i18n("Background"), 50, 
			koconf.windowColor, "AptBackground", "Colors" ) );
	connectConfig( color1 );
	group->addWidget( color2 = new KPropColor( group, i18n("Text"), 50, 
			koconf.textColor, "AptText", "Colors" ) );
	connectConfig( color2 );
	group->addWidget( color3 = new KPropColor( group, i18n("Handle Selected"), 50, 
						   koconf.activeTitleColor, "AptSelected", "Colors" ) );
	connectConfig( color3 );
	group->addWidget( color4 = new KPropColor( group, i18n("Handle Unselected"), 50, 
			koconf.inactiveTitleColor, "AptUnselected", "Colors" ) );
	connectConfig( color4 );
	group->addWidget( color5 = new KPropColor( group, i18n("Handle Active Apt."), 50, 
			koconf.activeTextColor, "AptActive", "Colors" ) );
	connectConfig( color5 );

	layout->addWidget( group = new KPropGroup( colorsFrame, "Other" ) );
	group->addWidget( color6 = new KPropColor( group, i18n("Sheet background"), 50, 
			koconf.backgroundColor.dark(120), "SheetBackground", "Colors" ) );
	connectConfig( color6 );
	group->addWidget( color7 = new KPropColor( group, i18n("Calendar Background"), 50, 
			koconf.windowColor, "CalBackground", "Colors" ) );
	connectConfig( color7 );
	group->addWidget( color8 = new KPropColor( group, i18n("Calendar Date Text"), 50, 
			koconf.textColor, "CalText", "Colors" ) );
	connectConfig( color8 );
	group->addWidget( color9 = new KPropColor( group, i18n("Calendar Date Selected"), 50, 
			koconf.selectColor, "CalSelected", "Colors" ) );
	connectConfig( color9 );


/*	
	KPropColor *colorProp = new KPropColor( colorsFrame[A, 
						i18n("Appointment"), 50, 
						"List Color", "Colors" );
	connectConfig(colorProp);
	layout->addWidget(colorProp);

	colorProp = new KPropColor(colorsFrame, 
				   i18n("Today"), 50,
				   "Today Color", "Colors");
	connectConfig(colorProp);
	layout->addWidget(colorProp);

	colorProp = new KPropColor(colorsFrame, 
				   i18n("Holidays"), 50, 
				   "Holiday Color", "Colors");
	connectConfig(colorProp);
	layout->addWidget(colorProp);
*/
// 	layout->addStretch();
#endif
}

void KOOptionsDialog::selectHolidayColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mHolidayColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectHighlightColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mHighlightColor->setBackgroundColor(myColor);
  }
}


void KOOptionsDialog::setupPrinterTab()
{
  mPrinterTab = addPage(i18n("Printing"));
  
  QGridLayout *topLayout = new QGridLayout(mPrinterTab,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  topLayout->addWidget(new QLabel(i18n("Printer Name:"),mPrinterTab),0,0);
  mPrinterCombo = new QComboBox(mPrinterTab);
  topLayout->addWidget(mPrinterCombo,0,1);

  QString prName;
  QFile printcap("/etc/printcap");

  if ( printcap.open(IO_ReadOnly) ) {
    QTextStream t (&printcap);
    while ( !t.eof() ){
      QString whole_line = t.readLine();
      whole_line = whole_line.stripWhiteSpace();
      if (whole_line.left(1) != "#" &&
	  whole_line.left(1) != ":" &&
	  whole_line != "") { // ignore certain lines
	QString pname(whole_line.mid(whole_line.find("|")+1,
				     whole_line.length()));
	if (pname.find("|") != -1) { // If there is no 2nd bar.XCXC
	  // Try to account for poorly formatted lines
	  prName = pname.left(pname.find("|"));
	} else {
	  prName = pname.left(pname.find(":"));
	}
	mPrinterCombo->insertItem(prName);
      }
    }
    printcap.close();
  }

  mPaperSizeGroup = new QButtonGroup(1,Horizontal,i18n("Paper Size:"),
                                     mPrinterTab);
  (void)new QRadioButton(i18n("A4"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("B5"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Letter"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Legal"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Executive"),mPaperSizeGroup);
  topLayout->addMultiCellWidget(mPaperSizeGroup,1,1,0,1);

  mPaperOrientationGroup = new QButtonGroup(1,Horizontal,
                                            i18n("Paper Orientation:"),
                                            mPrinterTab);
  (void)new QRadioButton(i18n("Portrait"),mPaperOrientationGroup);
  (void)new QRadioButton(i18n("Landscape"),mPaperOrientationGroup);
  topLayout->addMultiCellWidget(mPaperOrientationGroup,1,1,0,1);

  topLayout->addWidget(new QLabel(i18n("Preview Program:"),mPrinterTab),2,0);
  mPrintPreviewEdit = new QLineEdit(mPrinterTab);
  topLayout->addWidget(mPrintPreviewEdit,2,1);
}


void KOOptionsDialog::showPrinterTab()
{
  showPage(pageIndex(mPrinterTab));
}


void KOOptionsDialog::setColorDefaults()
{
#if 0
  KConfig config(locate("config", "korganizerrc")); 
	config.setGroup( "Colors" );

	color1->setLabelColor( koconf.windowColor);
	color2->setLabelColor( koconf.textColor);
	color3->setLabelColor( koconf.selectColor);
	color4->setLabelColor( koconf.inactiveTextColor);
	color5->setLabelColor( koconf.activeTextColor);
	color6->setLabelColor( koconf.backgroundColor.dark(120));
	color7->setLabelColor( koconf.windowColor);
	color8->setLabelColor( koconf.textColor);
	color9->setLabelColor( koconf.selectColor);
#endif
}

void KOOptionsDialog::applyColorDefaults()
{
#if 0
	emit setConfig();
#endif
}

void KOOptionsDialog::toggleSystemColors( bool syscol )
{
	if( syscol )
		setColorDefaults();
//	else
//		emit config();
}


void KOOptionsDialog::setDefaults()
{
  KOPrefs::instance()->setDefaults();
  readConfig();
}


void KOOptionsDialog::setCombo(QComboBox *combo,const QString & text)
{
  int i;
  for(i=0;i<combo->count();++i) {
    if (combo->text(i) == text) {
      combo->setCurrentItem(i);
      break;
    }
  }
}

void KOOptionsDialog::readConfig()
{
  mAutoSaveCheck->setChecked(KOPrefs::instance()->mAutoSave);
  mConfirmCheck->setChecked(KOPrefs::instance()->mConfirm);

  mNameEdit->setText(KOPrefs::instance()->mName);
  mEmailEdit->setText(KOPrefs::instance()->mEmail);
  mAdditionalEdit->setText(KOPrefs::instance()->mAdditional);

  setCombo(mHolidayCombo,KOPrefs::instance()->mHoliday);
  
  mTimeFormatGroup->setButton(KOPrefs::instance()->mTimeFormat);
  mDateFormatGroup->setButton(KOPrefs::instance()->mDateFormat);
  setCombo(mTimeZoneCombo,KOPrefs::instance()->mHoliday);

  mStartTimeSpin->setValue(KOPrefs::instance()->mStartTime);
  mAlarmTimeCombo->setCurrentItem(KOPrefs::instance()->mAlarmTime);
  mWeekstartCheck->setChecked(KOPrefs::instance()->mWeekstart);

  mDayBeginsSpin->setValue(KOPrefs::instance()->mDayBegins);
  mHourSizeSlider->setValue(KOPrefs::instance()->mHourSize);
  mDailyRecurCheck->setChecked(KOPrefs::instance()->mDailyRecur);

  mTimeBarFont->setFont(KOPrefs::instance()->mTimeBarFont);

  mHolidayColor->setBackgroundColor(KOPrefs::instance()->mHolidayColor);
  mHighlightColor->setBackgroundColor(KOPrefs::instance()->mHighlightColor);

  setCombo(mPrinterCombo,KOPrefs::instance()->mPrinter);

  mPaperSizeGroup->setButton(KOPrefs::instance()->mPaperSize);
  mPaperOrientationGroup->setButton(KOPrefs::instance()->mPaperOrientation);
  mPrintPreviewEdit->setText(KOPrefs::instance()->mPrintPreview);
}


void KOOptionsDialog::writeConfig()
{
  qDebug("KOOptionsDialog::writeConfig()");

  KOPrefs::instance()->mAutoSave = mAutoSaveCheck->isChecked();
  KOPrefs::instance()->mConfirm = mConfirmCheck->isChecked();

  KOPrefs::instance()->mName = mNameEdit->text();
  KOPrefs::instance()->mEmail = mEmailEdit->text();
  KOPrefs::instance()->mAdditional = mAdditionalEdit->text();
  KOPrefs::instance()->mHoliday = mHolidayCombo->currentText();

  KOPrefs::instance()->mTimeFormat = 
      mTimeFormatGroup->id(mTimeFormatGroup->selected());
  KOPrefs::instance()->mDateFormat = 
      mDateFormatGroup->id(mDateFormatGroup->selected());
  KOPrefs::instance()->mTimeZone = mTimeZoneCombo->currentText();
  KOPrefs::instance()->mStartTime = mStartTimeSpin->value();
  KOPrefs::instance()->mAlarmTime = mAlarmTimeCombo->currentItem();
  KOPrefs::instance()->mWeekstart = mWeekstartCheck->isChecked();

  KOPrefs::instance()->mDayBegins = mDayBeginsSpin->value();
  KOPrefs::instance()->mHourSize = mHourSizeSlider->value();
  KOPrefs::instance()->mDailyRecur = mDailyRecurCheck->isChecked();

  KOPrefs::instance()->mTimeBarFont = mTimeBarFont->font();

  KOPrefs::instance()->mHolidayColor = mHolidayColor->backgroundColor();
  KOPrefs::instance()->mHighlightColor = mHighlightColor->backgroundColor();

  KOPrefs::instance()->mPrinter = mPrinterCombo->currentText();
  KOPrefs::instance()->mPaperSize = 
      mPaperSizeGroup->id(mPaperSizeGroup->selected());
  KOPrefs::instance()->mPaperOrientation =
      mPaperOrientationGroup->id(mPaperOrientationGroup->selected());
  KOPrefs::instance()->mPrintPreview = mPrintPreviewEdit->text();
  
  KOPrefs::instance()->writeConfig();
}


void KOOptionsDialog::slotApply()
{
  writeConfig();
  emit configChanged();
}


void KOOptionsDialog::slotOk()
{
  slotApply();
  accept();
}

