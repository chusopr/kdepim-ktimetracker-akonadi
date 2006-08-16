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

#include <QtDBus/QDBusConnection>
#include <QtGui/QGridLayout>

#include "agentwidget.h"
#include "profilewidget.h"

#include "mainwidget.h"

MainWidget::MainWidget( QWidget *parent )
  : QWidget( parent )
{
  mAgentManager = new org::kde::Akonadi::AgentManager( "org.kde.Akonadi.AgentManager", "/", QDBus::sessionBus(), this );

  QGridLayout *layout = new QGridLayout( this );

  ProfileWidget *profileWidget = new ProfileWidget( this );
  layout->addWidget( profileWidget, 0, 0 );

  AgentWidget *agentWidget = new AgentWidget( this );
  layout->addWidget( agentWidget, 0, 1 );
}
