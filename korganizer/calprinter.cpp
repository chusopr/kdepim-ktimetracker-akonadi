/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qvbuttongroup.h>
#include <qwidgetstack.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>

#include <kprinter.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>

#include "koprefsdialog.h"

#include "calprinter.h"
#ifndef KORG_NOPRINTER
#include "calprinter.moc"

#include "calprintplugins.h"

CalPrinter::CalPrinter(QWidget *parent, Calendar *calendar)
  : QObject(0L, "CalPrinter")
{
  mCalendar = calendar;
  mParent = parent;
  mPrinter = new KPrinter;
  mPrinter->setOrientation(KPrinter::Landscape);
  mConfig=new KSimpleConfig("korganizer_printing.rc");
  init(mPrinter, calendar);
}

CalPrinter::~CalPrinter()
{
  delete mPrinter;
  delete mPrintDialog;
}

void CalPrinter::init(KPrinter *printer, Calendar *calendar)
{
  mPrintPlugins.setAutoDelete(true);
  mPrintPlugins.append( new CalPrintDay( printer, calendar, mConfig ) );
  mPrintPlugins.append( new CalPrintWeek( printer, calendar, mConfig ) );
  mPrintPlugins.append( new CalPrintMonth( printer, calendar, mConfig ) );
  mPrintPlugins.append( new CalPrintTodos( printer, calendar, mConfig ) );

  // TODO_RK: Add a plugin interface here
  mPrintDialog=new CalPrintDialog(mPrintPlugins, mPrinter, mParent);

  CalPrintBase*plug = mPrintPlugins.first();
  while (plug) {
    connect( this, SIGNAL( setDateRangeSignal( const QDate&, const QDate& ) ),
      plug, SLOT( setDateRange( const QDate&, const QDate& ) ) );
    connect( this, SIGNAL( updateConfigSignal() ),
      plug, SLOT( loadConfig() ) );
    connect( this, SIGNAL( writeConfigSignal() ),
      plug, SLOT( saveConfig() ) );
    connect( mPrintDialog, SIGNAL(okClicked()), plug, SLOT(readSettingsWidget()) );

    plug->setSettingsWidget();

    plug=mPrintPlugins.next();
  }
}

void CalPrinter::setupPrinter()
{
  KMessageBox::sorry( mParent, i18n("Not implemented.") );
#if 0
  KOPrefsDialog *optionsDlg = new KOPrefsDialog(mParent);
  optionsDlg->readConfig();
  optionsDlg->showPrinterTab();
  connect(optionsDlg, SIGNAL(configChanged()),
          mParent, SLOT(updateConfig()));
  optionsDlg->show();
#endif
}

void CalPrinter::setDateRange(const QDate&fd, const QDate&td)
{
  emit setDateRangeSignal(fd, td);
}

void CalPrinter::preview(int type, const QDate &fd, const QDate &td)
{
  mPrintDialog->setPreview(true);
  mPrintDialog->setPrintType(type);
  emit setDateRangeSignal(fd,td);

  if (mPrintDialog->exec() == QDialog::Accepted) {
    doPreview( mPrintDialog->selectedPlugin() );
  }
}

void CalPrinter::print(int type, const QDate &fd, const QDate &td)
{
  mPrintDialog->setPreview(false);
  mPrintDialog->setPrintType(type);
  emit setDateRangeSignal(fd,td);

  if (mPrintDialog->exec() == QDialog::Accepted) {
    doPrint( mPrintDialog->selectedPlugin() );
  }
}

void CalPrinter::forcePrint(int type, const QDate &fd, const QDate &td, bool preview)
{
  if (type<0) return;
  emit setDateRangeSignal(fd,td);

  if (preview)
    mPrinter->setPreviewOnly(true);
  else
    if (!mPrinter->setup(mParent, i18n("Print Calendar"))) return;

  CalPrintBase* selectedStyle = mPrintPlugins.at(type);
  if (selectedStyle) selectedStyle->doPrint();

  if (preview)
    mPrinter->setPreviewOnly(false);
}

void CalPrinter::doPreview(CalPrintBase*selectedStyle)
{
  mPrinter->setPreviewOnly(true);
  selectedStyle->doPrint();
  // restore previous settings that were used before the preview.
  mPrinter->setPreviewOnly(false);
}

void CalPrinter::doPrint(CalPrintBase*selectedStyle)
{
  // FIXME: add a better caption to the Printingdialog
  if (!mPrinter->setup(mParent, i18n("Print Calendar"))) return;

  selectedStyle->doPrint();
}

///////////////////////////////////////////////////////////////////////////////

void CalPrinter::updateConfig()
{
}



/****************************************************************************/

CalPrintDialog::CalPrintDialog(QPtrList<CalPrintBase> plugins, KPrinter *p,
        QWidget *parent, const char *name)
  : KDialogBase( parent, name, /*modal*/true, i18n("Print"),
        KDialogBase::Ok|KDialogBase::Cancel),
    mPrinter(p), mPrintPlugins(plugins)
{
  QWidget *page = new QWidget( this );
  setMainWidget(page);
  QGridLayout *topLayout = new QGridLayout( page );

  mPrinterLabel = new QLabel("", page);
  topLayout->addMultiCellWidget(mPrinterLabel, 0, 0, 0, 2);

  QPushButton*setupButton = new QPushButton(i18n("&Setup printer..."), page);
  topLayout->addWidget(setupButton, 0, 3);
  connect(setupButton, SIGNAL(clicked()), this, SLOT(setupPrinter()) );

  mTypeGroup = new QVButtonGroup(i18n("View Type"), page);
  // use the minimal width possible = max width of the radio buttons, not extensible
  mTypeGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4,
    (QSizePolicy::SizeType)5, 0, 0,
    mTypeGroup->sizePolicy().hasHeightForWidth() ) );

  mConfigArea = new QWidgetStack(page);
  connect( mTypeGroup, SIGNAL(clicked(int)),
    this, SLOT(setPrintType(int)) );

  CalPrintBase*plug=mPrintPlugins.first();
  QRadioButton *rButt;
  while (plug) {
    rButt = new QRadioButton(plug->description(), mTypeGroup);
    rButt->setMinimumHeight( rButt->sizeHint().height()-5 );

    mConfigArea->addWidget(plug->configWidget(mConfigArea));
    connect( this, SIGNAL(applySettings()), plug, SLOT(readSettingsWidget()) );
    connect( this, SIGNAL(doSettings()), plug, SLOT(setSettingsWidget()) );

    plug=mPrintPlugins.next();
  }

  topLayout->addMultiCellWidget(mTypeGroup, 1,2, 0,0 );
  topLayout->addMultiCellWidget(mConfigArea, 1,1, 1,3 );

  QLabel*orientationLabel=new QLabel(i18n("&Orientation:"), page);
  topLayout->addWidget(orientationLabel, 2, 1);

  mOrientationSelection=new QComboBox(page);
  mOrientationSelection->insertItem(i18n("Use default setting of printer"));
  mOrientationSelection->insertItem(i18n("Portrait"));
  mOrientationSelection->insertItem(i18n("Landscape"));
  topLayout->addMultiCellWidget( mOrientationSelection, 2,2, 2,3 );
  orientationLabel->setBuddy(mOrientationSelection);
}

CalPrintDialog::~CalPrintDialog()
{
}

void CalPrintDialog::setupPrinter()
{
  if (mPrinter->setup(this, i18n("Setup printer"))) {
    setPrinterLabel();
  }
}

void CalPrintDialog::setPreview(bool preview)
{
  setButtonOKText( preview ? i18n("&Preview") : i18n("&Print...") );
  mPreviewText = preview ? i18n("<qt>Preview for printer <b>%1</b></qt>")
      : i18n( "<qt>Printing on printer <b>%1</b></qt>");
  setPrinterLabel();
}

void CalPrintDialog::setPrinterLabel()
{
  QString printerName(mPrinter->printerName());
  if (printerName.isEmpty())
    mPrinterLabel->setText(mPreviewText.arg(i18n("[Unconfigured]")) );
  else
    mPrinterLabel->setText(mPreviewText.arg( printerName ));
}

void CalPrintDialog::setPrintType(int i)
{
  mTypeGroup->setButton(i);
  mConfigArea->raiseWidget(i+1);
}

CalPrintBase* CalPrintDialog::selectedPlugin()
{
  int pos=mTypeGroup->id( mTypeGroup->selected() );
  if (pos<0) return 0;
  CalPrintBase* retval = mPrintPlugins.at( pos );
  return retval;
}

void CalPrintDialog::slotOk()
{
  int orientSel=mOrientationSelection->currentItem();
  if (orientSel==1) {
    // TODO: Set Portrait mode
  } else if (orientSel==2) {
    // TODO: Set Landscape  mode
  }
  KDialogBase::slotOk();
}

#endif
