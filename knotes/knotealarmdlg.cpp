/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 2005, Michael Brade <brade@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 In addition, as a special exception, the copyright holders give
 permission to link the code of this program with any edition of
 the Qt library by Trolltech AS, Norway (or with modified versions
 of Qt that use the same license as Qt), and distribute linked
 combinations including the two.  You must obey the GNU General
 Public License in all respects for all of the code used other than
 Qt.  If you modify this file, you may extend this exception to
 your version of the file, but you are not obligated to do so.  If
 you do not wish to do so, delete this exception statement from
 your version.
*******************************************************************/

#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qvbox.h>

#include <klocale.h>

#include <libkdepim/kdateedit.h>
#include <libkdepim/ktimeedit.h>

#include "knotealarmdlg.h"


KNoteAlarmDlg::KNoteAlarmDlg( const QString& caption, QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, caption, Ok|Cancel, Ok )
{
    //enableButtonSeparator( true );

    QVBox *page = makeVBoxMainWidget();
    QGroupBox *buttons = new QGroupBox( 3, Vertical, i18n("Scheduled Alarm"), page );
    QButtonGroup *group = new QButtonGroup( page );
    group->hide();

    QRadioButton *none = new QRadioButton( i18n("No alarm"), buttons );
    group->insert( none );

    QHBox *at = new QHBox( buttons );
    QRadioButton *label_at = new QRadioButton( i18n("Alarm at:"), at );
    group->insert( label_at );
    KDateEdit *at_date = new KDateEdit( at );
    KTimeEdit *at_time = new KTimeEdit( at );
    at->setStretchFactor( at_date, 1 );

    QHBox *in = new QHBox( buttons );
    QRadioButton *label_in = new QRadioButton( i18n("Alarm in:"), in );
    group->insert( label_in );
    KTimeEdit *in_time = new KTimeEdit( in );
    QLabel *in_min = new QLabel( i18n("hours/minutes"), in );
}


KNoteAlarmDlg::~KNoteAlarmDlg()
{
}


#include "knotealarmdlg.moc"
