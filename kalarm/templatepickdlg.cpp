/*
 *  templatepickdlg.cpp  -  dialogue to choose an alarm template
 *  Program:  kalarm
 *  (C) 2004 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of this program with any edition of the Qt library by
 *  Trolltech AS, Norway (or with modified versions of Qt that use the same
 *  license as Qt), and distribute linked combinations including the two.
 *  You must obey the GNU General Public License in all respects for all of
 *  the code used other than Qt.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not obligated to
 *  do so. If you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "kalarm.h"

#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kdebug.h>

#include "kalarmapp.h"
#include "functions.h"
#include "templatelistview.h"
#include "templatepickdlg.moc"

static const char TMPL_PICK_DIALOG_NAME[] = "TemplatePickDialog";


TemplatePickDlg::TemplatePickDlg(QWidget* parent, const char* name)
	: KDialogBase(KDialogBase::Plain, i18n("Choose an Alarm Template"), Ok|Cancel, Ok, parent, name)
{
	QWidget* topWidget = plainPage();
	QBoxLayout* topLayout = new QVBoxLayout(topWidget);
	topLayout->setSpacing(spacingHint());

	// Display the list of templates, but exclude command alarms if in kiosk mode.
	bool includeCmdAlarms = !theApp()->noShellAccess();
	mTemplateList = new TemplateListView(includeCmdAlarms, i18n("Select a template to base the new alarm on."), topWidget, "list");
	mTemplateList->setSelectionMode(QListView::Single);
	mTemplateList->refresh();      // populate the template list
	connect(mTemplateList, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	connect(mTemplateList, SIGNAL(executed(QListViewItem*)), SLOT(slotOk()));
	topLayout->addWidget(mTemplateList);

	slotSelectionChanged();        // enable or disable the OK button

	resize(KAlarm::readConfigWindowSize(TMPL_PICK_DIALOG_NAME, minimumSize()));
}

/******************************************************************************
* Return the currently selected alarm template, or 0 if none.
*/
const KAEvent* TemplatePickDlg::selectedTemplate() const
{
	return mTemplateList->selectedEvent();
}

/******************************************************************************
* Called when the template selection changes.
* Enable/disable the OK button depending on whether anything is selected.
*/
void TemplatePickDlg::slotSelectionChanged()
{
	enableButtonOK(mTemplateList->selectedItem());
}

/******************************************************************************
*  Called when the dialog's size has changed.
*  Records the new size in the config file.
*/
void TemplatePickDlg::resizeEvent(QResizeEvent* re)
{
	if (isVisible())
		KAlarm::writeConfigWindowSize(TMPL_PICK_DIALOG_NAME, re->size());
	KDialog::resizeEvent(re);
}
