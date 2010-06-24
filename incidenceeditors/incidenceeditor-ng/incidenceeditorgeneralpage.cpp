/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "incidenceeditorgeneralpage.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

#include "incidencegeneraleditor.h"
#include "incidencedatetimeeditor.h"
#include "incidencedescriptioneditor.h"
#include "incidenceattachmenteditor.h"

#include <KLocale>

using namespace IncidenceEditorsNG;

IncidenceEditorGeneralPage::IncidenceEditorGeneralPage( QWidget *parent )
  : CombinedIncidenceEditor( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( 1 );

  IncidenceGeneralEditor *ieGeneral = new IncidenceGeneralEditor( this );
  layout->addWidget( ieGeneral );

  QFrame *seperatorLine = new QFrame( this );
  seperatorLine->setFrameShape( QFrame::HLine );
  seperatorLine->setFrameShadow( QFrame::Sunken );

  layout->addWidget( seperatorLine );
  IncidenceDateTimeEditor *ieDateTime = new IncidenceDateTimeEditor( this );
  layout->addWidget( ieDateTime );

  seperatorLine = new QFrame( this );
  seperatorLine->setFrameShape( QFrame::HLine );
  seperatorLine->setFrameShadow( QFrame::Sunken );
  layout->addWidget( seperatorLine );

  IncidenceDescriptionEditor *ieDescription = new IncidenceDescriptionEditor( this );
  layout->addWidget( ieDescription, 4 );

  connect( this, SIGNAL(dirtyStatusChanged(bool)),
           SLOT(updateDirtyLabel(bool)));
  
  // Combine the various editors with this page.
  combine( ieGeneral );
  combine( ieDateTime );
  combine( ieDescription );
}

#include "moc_incidenceeditorgeneralpage.cpp"
