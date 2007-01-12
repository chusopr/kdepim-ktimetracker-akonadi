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

#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>

#include "agentwidget.h"
#include "browserwidget.h"
#include "debugwidget.h"
#include "profilewidget.h"

#include "mainwidget.h"

MainWidget::MainWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QTabWidget *tabWidget = new QTabWidget( this );
  layout->addWidget( tabWidget );

  tabWidget->addTab( new AgentWidget( tabWidget ), "Agents" );
  tabWidget->addTab( new ProfileWidget( tabWidget ), "Profiles" );
  tabWidget->addTab( new BrowserWidget( tabWidget ), "Browser" );
  tabWidget->addTab( new DebugWidget( tabWidget ), "Debugger" );
}
