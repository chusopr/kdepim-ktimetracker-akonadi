// 	$Id$	

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kbuttonbox.h>
#include <kabapi.h>

#include "koevent.h"
#include "koprefs.h"

#include "koeditorgeneralevent.h"
#include "koeditorgeneralevent.moc"

KOEditorGeneralEvent::KOEditorGeneralEvent(int spacing,QWidget* parent,
                                           const char* name) :
  QWidget( parent, name)
{
  mSpacing = spacing;

//  alarmProgram = "";
  initTimeBox();
  initAlarmBox();
  initMisc();

  initLayout();

  QWidget::setTabOrder(summaryEdit, startDateEdit);
  QWidget::setTabOrder(startDateEdit, startTimeEdit);
  QWidget::setTabOrder(startTimeEdit, endDateEdit);
  QWidget::setTabOrder(endDateEdit, endTimeEdit);
  QWidget::setTabOrder(endTimeEdit, noTimeButton);
  QWidget::setTabOrder(noTimeButton, recursButton);
  QWidget::setTabOrder(recursButton, alarmButton);
  QWidget::setTabOrder(alarmButton, freeTimeCombo);
  QWidget::setTabOrder(freeTimeCombo, descriptionEdit);
  QWidget::setTabOrder(descriptionEdit, categoriesButton);
  QWidget::setTabOrder(categoriesButton, privateButton);

  summaryEdit->setFocus();

  // time widgets on General and Recurrence tab are synchronized
  connect(startTimeEdit, SIGNAL(timeChanged(QTime)),
	  this, SLOT(startTimeChanged(QTime)));
  connect(endTimeEdit, SIGNAL(timeChanged(QTime)),
	  this, SLOT(endTimeChanged(QTime)));

  // date widgets on General and Recurrence tab are synchronized
  connect(startDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(startDateChanged(QDate)));
  connect(endDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(endDateChanged(QDate)));

  // recursion on/off
  connect(this,SIGNAL(recursChanged(bool)),SLOT(recurStuffEnable(bool)));
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

void KOEditorGeneralEvent::initTimeBox()
{
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Appointment Time "),this, "User_2" );

  QFrame *timeBoxFrame = new QFrame(timeGroupBox,"TimeBoxFrame");

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,2,3);
  layoutTimeBox->setSpacing(mSpacing);


  startDateLabel = new QLabel( timeBoxFrame );
  startDateLabel->setText( i18n("Start Date:") );
  layoutTimeBox->addWidget(startDateLabel,0,0);
  
  startDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startDateEdit,0,1);

  startTimeLabel = new QLabel( timeBoxFrame, "Label_2" );
  startTimeLabel->setText( i18n("Start Time:") );
  layoutTimeBox->addWidget(startTimeLabel,0,2);
  
  startTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startTimeEdit,0,3);


  noTimeButton = new QCheckBox(timeBoxFrame, "CheckBox_1" );
  noTimeButton->setText( i18n("No time associated") );
  layoutTimeBox->addWidget(noTimeButton,0,5);

  connect(noTimeButton, SIGNAL(toggled(bool)),SLOT(timeStuffDisable(bool)));
  connect(noTimeButton, SIGNAL(toggled(bool)),SLOT(alarmStuffDisable(bool)));
  connect(noTimeButton, SIGNAL(toggled(bool)),SIGNAL(allDayChanged(bool)));

  
  endDateLabel = new QLabel( timeBoxFrame, "Label_3" );
  endDateLabel->setText( i18n("End Date:") );
  layoutTimeBox->addWidget(endDateLabel,1,0);

  endDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(endDateEdit,1,1);

  endTimeLabel = new QLabel( timeBoxFrame, "Label_3" );
  endTimeLabel->setText( i18n("End Time:") );
  layoutTimeBox->addWidget(endTimeLabel,1,2);

  endTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(endTimeEdit,1,3);


  recursButton = new QCheckBox(timeBoxFrame);
  recursButton->setText(i18n("Recurring event"));
  layoutTimeBox->addWidget(recursButton,1,5);

  QObject::connect(recursButton,SIGNAL(toggled(bool)),
                   SIGNAL(recursChanged(bool)));

  // add stretch space between date/time edits and checkboxes
  layoutTimeBox->setColStretch(4,1);
}

void KOEditorGeneralEvent::initMisc()
{
  summaryLabel = new QLabel( this, "Label_1" );
  summaryLabel->setText( i18n("Summary:") );
//  summaryLabel->setAlignment( 289 );
//  summaryLabel->setMargin( -1 );

  summaryEdit = new QLineEdit( this, "LineEdit_1" );

  freeTimeLabel = new QLabel( this, "Label_6" );
  freeTimeLabel->setText( i18n("Show Time As:") );
//  freeTimeLabel->setAlignment( 289 );
//  freeTimeLabel->setMargin( -1 );

  freeTimeCombo = new QComboBox( false, this, "ComboBox_1" );
//  freeTimeCombo->setSizeLimit( 10 );  // that's the default value anyway
  freeTimeCombo->insertItem( i18n("Busy") );
  freeTimeCombo->insertItem( i18n("Free") );

  descriptionEdit = new QMultiLineEdit( this, "MultiLineEdit_1" );
  descriptionEdit->insertLine( "" );
  descriptionEdit->setReadOnly( false );
  descriptionEdit->setOverwriteMode( false );

  ownerLabel = new QLabel( this, "Label_7" );
  ownerLabel->setText( i18n("Owner:") );
//  ownerLabel->setAlignment( 289 );
//  ownerLabel->setMargin( -1 );

  privateButton = new QCheckBox( this, "CheckBox_3" );
  privateButton->setText( i18n("Private") );

  categoriesButton = new QPushButton( this, "PushButton_6" );
  categoriesButton->setText( i18n("Categories...") );
  connect(categoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));

  categoriesLabel = new QLabel( this, "LineEdit_7" );
  categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
}

void KOEditorGeneralEvent::initAlarmBox()
{
  QPixmap pixmap;

  alarmBell = new QLabel(this);
  alarmBell->setPixmap(UserIcon("bell"));

  alarmButton = new QCheckBox( this, "CheckBox_2" );
  alarmButton->setText( i18n("Reminder:") );

  alarmTimeEdit = new KRestrictedLine( this, "alarmTimeEdit",
				       "1234567890");
  alarmTimeEdit->setText("");

  alarmIncrCombo = new QComboBox(false, this);
  alarmIncrCombo->insertItem(i18n("minute(s)"));
  alarmIncrCombo->insertItem(i18n("hour(s)"));
  alarmIncrCombo->insertItem(i18n("day(s)"));
  alarmIncrCombo->setMinimumHeight(20);

  alarmSoundButton = new QPushButton( this, "PushButton_4" );
  pixmap = UserIcon("playsound");
  //  alarmSoundButton->setText( i18n("WAV") );
  alarmSoundButton->setPixmap(pixmap);
  alarmSoundButton->setToggleButton(true);
  QToolTip::add(alarmSoundButton, "No sound set");

  alarmProgramButton = new QPushButton( this, "PushButton_5" );
  pixmap = UserIcon("runprog");
  //  alarmProgramButton->setText( i18n("PROG") );
  alarmProgramButton->setPixmap(pixmap);
  alarmProgramButton->setToggleButton(true);
  QToolTip::add(alarmProgramButton, "No program set");

  connect(alarmButton, SIGNAL(toggled(bool)),
	  this, SLOT(alarmStuffEnable(bool)));

  connect(alarmSoundButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmSound()));
  connect(alarmProgramButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmProgram()));
}

void KOEditorGeneralEvent::initLayout()
{
  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->setSpacing(mSpacing);
  
  layoutTop->addWidget(ownerLabel);

  QBoxLayout *layoutSummary = new QHBoxLayout;
  layoutTop->addLayout(layoutSummary);
  layoutSummary->addWidget(summaryLabel);
  layoutSummary->addWidget(summaryEdit);

  layoutTop->addWidget(timeGroupBox);

  QBoxLayout *layoutAlarmLine = new QHBoxLayout;
  layoutTop->addLayout(layoutAlarmLine);

  QBoxLayout *layoutAlarmBox = new QHBoxLayout;
  layoutAlarmLine->addLayout(layoutAlarmBox);
  layoutAlarmBox->addWidget(alarmBell);
  layoutAlarmBox->addWidget(alarmButton);
  layoutAlarmBox->addWidget(alarmTimeEdit);
  layoutAlarmBox->addWidget(alarmIncrCombo);
  layoutAlarmBox->addWidget(alarmSoundButton);
  layoutAlarmBox->addWidget(alarmProgramButton);

  layoutAlarmLine->addStretch(1);

  QBoxLayout *layoutFreeTime = new QHBoxLayout;
  layoutAlarmLine->addLayout(layoutFreeTime);
  layoutFreeTime->addStretch(1);
  layoutFreeTime->addWidget(freeTimeLabel);
  layoutFreeTime->addWidget(freeTimeCombo);

  layoutTop->addWidget(descriptionEdit,1);

  QBoxLayout *layoutCategories = new QHBoxLayout;
  layoutTop->addLayout(layoutCategories);
  layoutCategories->addWidget(categoriesButton);
  layoutCategories->addWidget(categoriesLabel,1);
  layoutCategories->addWidget(privateButton);
}

void KOEditorGeneralEvent::pickAlarmSound()
{
  QString prefix = KGlobal::dirs()->findResourceDir("appdata", "sounds/alert.wav"); 
  if (!alarmSoundButton->isOn()) {
    alarmSound = "";
    QToolTip::remove(alarmSoundButton);
    QToolTip::add(alarmSoundButton, "No sound set");
  } else {
    QString fileName(QFileDialog::getOpenFileName(prefix.data(),
						  "*.wav", this));
    if (!fileName.isEmpty()) {
      alarmSound = fileName;
      QToolTip::remove(alarmSoundButton);
      QString dispStr = "Playing \"";
      dispStr += fileName.data();
      dispStr += "\"";
      QToolTip::add(alarmSoundButton, dispStr.data());
    }
  }
  if (alarmSound.isEmpty())
    alarmSoundButton->setOn(false);
}

void KOEditorGeneralEvent::pickAlarmProgram()
{
  if (!alarmProgramButton->isOn()) {
    alarmProgram = "";
    QToolTip::remove(alarmProgramButton);
    QToolTip::add(alarmProgramButton, "No program set");
  } else {
    QString fileName(QFileDialog::getOpenFileName(QString::null, "*", this));
    if (!fileName.isEmpty()) {
      alarmProgram = fileName;
      QToolTip::remove(alarmProgramButton);
      QString dispStr = "Running \"";
      dispStr += fileName.data();
      dispStr += "\"";
      QToolTip::add(alarmProgramButton, dispStr.data());
    }
  }
  if (alarmProgram.isEmpty())
    alarmProgramButton->setOn(false);
}

void KOEditorGeneralEvent::setEnabled(bool enabled)
{
  noTimeButton->setEnabled(enabled);
  recursButton->setEnabled(enabled);

  summaryEdit->setEnabled(enabled);
  startDateEdit->setEnabled(enabled);
  endDateEdit->setEnabled(enabled);

  startTimeEdit->setEnabled(enabled);
  endTimeEdit->setEnabled(enabled);

  alarmButton->setEnabled(enabled);
  alarmTimeEdit->setEnabled(enabled);
  alarmSoundButton->setEnabled(enabled);
  alarmProgramButton->setEnabled(enabled);

  descriptionEdit->setEnabled(enabled);
  freeTimeCombo->setEnabled(enabled);
  privateButton->setEnabled(enabled);
  categoriesButton->setEnabled(enabled);
  categoriesLabel->setEnabled(enabled);
}

void KOEditorGeneralEvent::timeStuffDisable(bool disable)
{
  if (disable) {
    startTimeLabel->hide();
    endTimeLabel->hide();
    startTimeEdit->hide();
    endTimeEdit->hide();
  } else {
    startTimeLabel->show();
    endTimeLabel->show();
    startTimeEdit->show();
    endTimeEdit->show();
  }
}

void KOEditorGeneralEvent::alarmStuffEnable(bool enable)
{
  alarmTimeEdit->setEnabled(enable);
  alarmSoundButton->setEnabled(enable);
  alarmProgramButton->setEnabled(enable);
}

void KOEditorGeneralEvent::alarmStuffDisable(bool disable)
{
  alarmTimeEdit->setEnabled(!disable);
  alarmSoundButton->setEnabled(!disable);
  alarmProgramButton->setEnabled(!disable);
}

void KOEditorGeneralEvent::recurStuffEnable(bool enable)
{
  if (enable) {
    startDateLabel->hide();
    endDateLabel->hide();
    startDateEdit->hide();
    endDateEdit->hide();
  } else {
    startDateLabel->show();
    endDateLabel->show();
    startDateEdit->show();
    endDateEdit->show();
  }
}

void KOEditorGeneralEvent::setDateTimes(QDateTime start, QDateTime end)
{
  qDebug("KOEditorGeneralEvent::setDateTimes(): Start DateTime: %s",
         start.toString().latin1());

  startDateEdit->setDate(start.date());
  startTimeEdit->setTime(start.time());
  endDateEdit->setDate(end.date());
  endTimeEdit->setTime(end.time());

  currStartDateTime = start;
  currEndDateTime = end;
}

void KOEditorGeneralEvent::setCategories(QString str)
{
  categoriesLabel->setText(str);
}

void KOEditorGeneralEvent::startTimeChanged(QTime newtime)
{
  qDebug("KOEditorGeneralEvent::startTimeChanged() %s",
         newtime.toString().latin1());

  int secsep = currStartDateTime.secsTo(currEndDateTime);
  
  currStartDateTime.setTime(newtime);

  // adjust end time so that the event has the same duration as before.
  currEndDateTime = currStartDateTime.addSecs(secsep);
  endTimeEdit->setTime(currEndDateTime.time());
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::endTimeChanged(QTime newtime)
{
  qDebug("KOEditorGeneralEvent::endTimeChanged %s",
         newtime.toString().latin1());

  QDateTime newdt(currEndDateTime.date(), newtime);

  if(newdt < currStartDateTime) {
    // oops, can't let that happen.
    newdt = currStartDateTime;
    endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::startDateChanged(QDate newdate)
{
  int daysep = currStartDateTime.daysTo(currEndDateTime);
  
  currStartDateTime.setDate(newdate);
  
  // adjust end date so that the event has the same duration as before
  currEndDateTime.setDate(currStartDateTime.date().addDays(daysep));
  endDateEdit->setDate(currEndDateTime.date());

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::endDateChanged(QDate newdate)
{
  QDateTime newdt(newdate, currEndDateTime.time());

  if(newdt < currStartDateTime) {
    // oops, we can't let that happen.
    newdt = currStartDateTime;
    endDateEdit->setDate(newdt.date());
    endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::setDefaults(QDateTime from,QDateTime to,bool allDay)
{
  ownerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->mName);

  noTimeButton->setChecked(allDay);
  timeStuffDisable(allDay);
  alarmStuffDisable(allDay);

  setDateTimes(from,to);

  recursButton->setChecked(false);
//  recurStuffEnable(false);

  QString alarmText(QString::number(KOPrefs::instance()->mAlarmTime));
  int pos = alarmText.find(' ');
  if (pos >= 0)
    alarmText.truncate(pos);
  alarmTimeEdit->setText(alarmText.data());
  alarmStuffEnable(false);
}

void KOEditorGeneralEvent::readEvent(KOEvent *event)
{
  QString tmpStr;
  QDateTime tmpDT;
  int i;

  summaryEdit->setText(event->getSummary());
  descriptionEdit->setText(event->getDescription());

  // organizer information
  ownerLabel->setText(i18n("Owner: ") + event->getOrganizer());

  // the rest is for the events only
  noTimeButton->setChecked(event->doesFloat());
  timeStuffDisable(event->doesFloat());
  alarmStuffDisable(event->doesFloat());

  setDateTimes(event->getDtStart(),event->getDtEnd());

  recursButton->setChecked(event->doesRecur());
  recurStuffEnable(event->doesRecur());

  privateButton->setChecked((event->getSecrecy() > 0) ? true : false);

  // set up alarm stuff
  alarmButton->setChecked(event->getAlarmRepeatCount());
  if (alarmButton->isChecked()) {
    alarmStuffEnable(true);
    tmpDT = event->getAlarmTime();
    i = tmpDT.secsTo(currStartDateTime);
    i = i / 60; // make minutes
    if (i % 60 == 0) { // divides evenly into hours?
      i = i / 60;
      alarmIncrCombo->setCurrentItem(1);
    }
    if (i % 24 == 0) { // divides evenly into days?
      i = i / 24;
      alarmIncrCombo->setCurrentItem(2);
    }

    alarmTimeEdit->setText(QString::number(i));

    if (!event->getProgramAlarmFile().isEmpty()) {
      alarmProgram = event->getProgramAlarmFile();
      alarmProgramButton->setOn(true);
      QString dispStr = "Running \"";
      dispStr += alarmProgram.data();
      dispStr += "\"";
      QToolTip::add(alarmProgramButton, dispStr.data());
    }
    if (!event->getAudioAlarmFile().isEmpty()) {
      alarmSound = event->getAudioAlarmFile();
      alarmSoundButton->setOn(true);
      QString dispStr = "Playing \"";
      dispStr += alarmSound.data();
      dispStr += "\"";
      QToolTip::add(alarmSoundButton, dispStr.data());
    }
  } else {
    alarmStuffEnable(false);
  }

  if (event->getTransparency() > 0)
    freeTimeCombo->setCurrentItem(1);
  // else it is implicitly 0 (i.e. busy)

  setCategories(event->getCategoriesStr());
}

void KOEditorGeneralEvent::writeEvent(KOEvent *event)
{
  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;
  bool ok;
  int j;

  event->setSummary(summaryEdit->text());
  event->setDescription(descriptionEdit->text());
  event->setCategories(categoriesLabel->text());
  event->setSecrecy(privateButton->isChecked() ? 1 : 0);

  if (noTimeButton->isChecked()) {
    event->setFloats(true);
    // need to change this.
    tmpDate = startDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);

    tmpDate = endDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);
  } else {
    event->setFloats(false);
    
    // set date/time end
    tmpDate = endDateEdit->getDate();
    tmpTime = endTimeEdit->getTime(ok);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);

    // set date/time start
    tmpDate = startDateEdit->getDate();
    tmpTime = startTimeEdit->getTime(ok);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);    
  } // check for float

  // alarm stuff
  if (alarmButton->isChecked()) {
    event->setAlarmRepeatCount(1);
    tmpStr = alarmTimeEdit->text();
    j = tmpStr.toInt() * -60;
    if (alarmIncrCombo->currentItem() == 1)
      j = j * 60;
    else if (alarmIncrCombo->currentItem() == 2)
      j = j * (60 * 24);

    tmpDT = event->getDtStart();
    tmpDT = tmpDT.addSecs(j);
    event->setAlarmTime(tmpDT);
    if (!alarmProgram.isEmpty() && alarmProgramButton->isOn())
      event->setProgramAlarmFile(alarmProgram);
    else
      event->setProgramAlarmFile("");
    if (!alarmSound.isEmpty() && alarmSoundButton->isOn())
      event->setAudioAlarmFile(alarmSound);
    else
      event->setAudioAlarmFile("");
  } else {
    event->setAlarmRepeatCount(0);
    event->setProgramAlarmFile("");
    event->setAudioAlarmFile("");
  }
  
  // note, that if on the details tab the "Transparency" option is implemented,
  // we will have to change this to suit.
  event->setTransparency(freeTimeCombo->currentItem());
}
