/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "incidenceeditorgeneralpage.h"

#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

#include "incidencegeneraleditor.h"
#include "incidencedescriptioneditor.h"
#include "incidenceattachmenteditor.h"

#include <KLocale>

using namespace IncidenceEditorsNG;

IncidenceEditorGeneralPage::IncidenceEditorGeneralPage( QWidget *parent )
  : CombinedIncidenceEditor( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( 0 );

  IncidenceGeneralEditor *ieGeneral = new IncidenceGeneralEditor( this );
  layout->addWidget( ieGeneral );

  IncidenceDescriptionEditor *ieDescription = new IncidenceDescriptionEditor( this );
  layout->addWidget( ieDescription );

  IncidenceAttachmentEditor *ieAttachment = new IncidenceAttachmentEditor( this );
  layout->addWidget( ieAttachment );
  
//   QSpacerItem *verticalSpacer =
//     new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
//   layout->addItem( verticalSpacer );

  mDirtyLabel = new QLabel( i18n( "Clean!" ), this );
  layout->addWidget( mDirtyLabel );

  connect( this, SIGNAL(dirtyStatusChanged(bool)),
           SLOT(updateDirtyLabel(bool)));
  
  // Combine the various editors with this page.
  combine( ieGeneral );
  combine( ieDescription );
  combine( ieAttachment );
}

void IncidenceEditorGeneralPage::updateDirtyLabel( bool isDirty )
{
  if ( isDirty )
    mDirtyLabel->setText( i18n( "Dirty!" ) );
  else
    mDirtyLabel->setText( i18n( "Clean!" ) );
}

#include "moc_incidenceeditorgeneralpage.cpp"
