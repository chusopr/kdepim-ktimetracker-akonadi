/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "agentwidget.h"

#include <akonadi/agenttypedialog.h>
#include <akonadi/agentinstancewidget.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/control.h>

#include <KLocale>

#include <QtGui/QGridLayout>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>

using namespace Akonadi;

AgentWidget::AgentWidget( QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this );

  mWidget = new Akonadi::AgentInstanceWidget( this );
  connect( mWidget, SIGNAL( doubleClicked( const Akonadi::AgentInstance& ) ), SLOT( configureAgent() ) );

  layout->addWidget( mWidget, 0, 0, 1, 6 );

  QPushButton *button = new QPushButton( "Add...", this );
  connect( button, SIGNAL( clicked() ), this, SLOT( addAgent() ) );
  layout->addWidget( button, 1, 1 );

  button = new QPushButton( "Remove", this );
  connect( button, SIGNAL( clicked() ), this, SLOT( removeAgent() ) );
  layout->addWidget( button, 1, 2 );

  button = new QPushButton( "Configure...", this );
  connect( button, SIGNAL( clicked() ), this, SLOT( configureAgent() ) );
  layout->addWidget( button, 1, 3 );

  QMenu *syncMenu = new QMenu( this );
  syncMenu->addAction( i18n("Synchronize All"), this, SLOT(synchronizeAgent()) );
  syncMenu->addAction( i18n("Synchronize Collection Tree"), this, SLOT(synchronizeTree()) );
  button = new QPushButton( "Synchronize", this );
  button->setMenu( syncMenu );
  connect( button, SIGNAL( clicked() ), this, SLOT( synchronizeAgent() ) );
  layout->addWidget( button, 1, 4 );

  button = new QPushButton( "Toggle Online/Offline", this );
  connect( button, SIGNAL(clicked()), SLOT(toggleOnline()) );
  layout->addWidget( button, 1, 5 );

  Control::widgetNeedsAkonadi( this );
}

void AgentWidget::addAgent()
{
  Akonadi::AgentTypeDialog dlg( this );
  if ( dlg.exec() ) {
    const AgentType agentType = dlg.agentType();

    if ( agentType.isValid() ) {
      AgentInstanceCreateJob *job = new AgentInstanceCreateJob( agentType, this );
      job->configure( this );
      job->start(); // TODO: check result
    }
  }
}

void AgentWidget::removeAgent()
{
  const AgentInstance agent = mWidget->currentAgentInstance();
  if ( agent.isValid() )
    AgentManager::self()->removeInstance( agent );
}

void AgentWidget::configureAgent()
{
  AgentInstance agent = mWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.configure( this );
}

void AgentWidget::synchronizeAgent()
{
  AgentInstance agent = mWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.synchronize();
}

void AgentWidget::toggleOnline()
{
  AgentInstance agent = mWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.setIsOnline( !agent.isOnline() );
}

void AgentWidget::synchronizeTree()
{
  AgentInstance agent = mWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.synchronizeCollectionTree();
}

#include "agentwidget.moc"
