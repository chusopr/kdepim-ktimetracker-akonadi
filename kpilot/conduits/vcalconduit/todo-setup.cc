/* todo-setup.cc			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
**
** This file is part of the todo conduit, a conduit for KPilot that
** synchronises the Pilot's todo application with the outside world,
** which currently means KOrganizer.
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

static const char *todo_setup_id="$Id$";

#include "options.h"

#include <qdir.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>

#include "kpilotConfig.h"
#include "todo-conduit.h"

#include "todo-setup.h"
#include "todo-setup.moc"


const QString TodoSetup::TodoGroup("todoOptions");

TodoSetup::TodoSetup(QWidget *parent)
  : setupDialog(parent,TodoGroup,TodoConduit::version())
{
  KConfig& config=KPilotConfig::getConfig(TodoGroup);
  addPage(new TodoSetupPage(this,config));
  addPage(new setupInfoPage(this));
  setupDialog::setupWidget();
  (void) todo_setup_id;
}


int TodoSetupPage::commitChanges(KConfig& config)
{
  config.writeEntry("CalFile", fCalendarFile->text());
  config.writeEntry("FirstTime", 
                    fPromptFirstTime->isChecked() ? "true" : "false");
  config.writeEntry("DeleteOnPilot",
                    fDeleteOnPilot->isChecked() ? "true" : "false");

  return 0;
}


void TodoSetupPage::slotBrowse()
{
  QString fileName = KFileDialog::getOpenFileName(0, "*.vcs *ics");
  if(fileName.isNull()) return;
  fCalendarFile->setText(fileName);
}

TodoSetupPage::TodoSetupPage(setupDialog *parent,KConfig& config) :
    setupDialogPage(i18n("ToDo File"),parent)
{
  grid = new QGridLayout(this, 2, 3, SPACING);

  fCalFileLabel = new QLabel(i18n("Calendar File:"),
			    this);
  fCalFileLabel->adjustSize();
  grid->addWidget(fCalFileLabel, 0, 0);
  
  fCalendarFile = new QLineEdit(this);
  fCalendarFile->setText(config.readEntry("CalFile", ""));
  fCalendarFile->resize(200, fCalendarFile->height());
  grid->addWidget(fCalendarFile, 0, 1);

  fBrowseButton = new QPushButton(i18n("Browse"), this);
  fBrowseButton->adjustSize();
  connect(fBrowseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  grid->addWidget(fBrowseButton, 0, 2);

  fPromptFirstTime =
    new QCheckBox(i18n("&Prompt before changing data."), this);
  fPromptFirstTime->adjustSize();
  fPromptFirstTime->setChecked(config.readBoolEntry("FirstTime", TRUE));
  grid->addWidget(fPromptFirstTime, 1, 1);

  fDeleteOnPilot = 
    new QCheckBox(i18n("Delete locally deleted records on pilot"),
		  this);
  fDeleteOnPilot->adjustSize();
  fDeleteOnPilot->setChecked(config.readBoolEntry("DeleteOnPilot", true));
  grid->addWidget(fDeleteOnPilot, 2, 1);
}

TodoSetupPage::~TodoSetupPage()
{
  delete fCalendarFile;
  delete fPromptFirstTime;
  delete fDeleteOnPilot;
  delete fBrowseButton;
  delete fCalFileLabel;
  delete grid;
}


// $Log$
// Revision 1.2  2001/06/05 22:58:40  adridg
// General rewrite, cleanup thx. Philipp Hullmann
//
// Revision 1.1  2001/04/16 13:36:20  adridg
// Moved todoconduit
//
// Revision 1.13  2001/04/01 17:32:05  adridg
// Fiddling around with date properties
//
// Revision 1.12  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.11  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.10  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
