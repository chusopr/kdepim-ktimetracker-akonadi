/*
    This file is part of KNode.
    Copyright (c) 2003      Laurent Montel  <montel@kde.org>,
    Based on the work of Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "knode_part.h"
#include "knglobals.h"
#include "knmainwidget.h"
#include "aboutdata.h"
#include "kncollectionview.h"

#include <kparts/genericfactory.h>
#include <kparts/statusbarextension.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <ksqueezedtextlabel.h>

#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>


typedef KParts::GenericFactory< KNodePart > KNodeFactory;
K_EXPORT_COMPONENT_FACTORY( libknodepart, KNodeFactory )

KNodePart::KNodePart( QWidget *parentWidget, QObject *parent, const QStringList &)
  : KParts::ReadOnlyPart( parent ),
  mParentWidget( parentWidget )
{
  kDebug(5003) << "KNodePart()" << endl;
  kDebug(5003) << "  InstanceName: " << KGlobal::mainComponent().componentName() << endl;

  setComponentData( KNodeFactory::componentData() );

  kDebug(5003) << "KNodePart()..." << endl;
  kDebug(5003) << "  InstanceName: " << KGlobal::mainComponent().componentName() << endl;

  KGlobal::locale()->insertCatalog("libkdepim");
  KGlobal::locale()->insertCatalog("libkpgp");
#ifdef __GNUC__
#warning Port me!
#endif
//   kapp->dcopClient()->suspend(); // Don't handle DCOP requests yet
  KIconLoader::global()->addAppDir("knode");
  knGlobals.setComponentData( KNodeFactory::componentData() );

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget(parentWidget);
  canvas->setFocusPolicy( Qt::ClickFocus );
  setWidget(canvas);

  mainWidget = new KNMainWidget( this, canvas );
  QVBoxLayout *topLayout = new QVBoxLayout(canvas);
  topLayout->addWidget(mainWidget);
  mainWidget->setFocusPolicy( Qt::ClickFocus );

#ifdef __GNUC__
#warning Port me!
#endif
//   kapp->dcopClient()->resume(); // Ok. We are ready for DCOP requests.

  KParts::StatusBarExtension* statusBar = new KParts::StatusBarExtension(this);
  statusBar->addStatusBarItem(mainWidget->statusBarLabelFilter(), 10, false);
  statusBar->addStatusBarItem(mainWidget->statusBarLabelGroup(), 15, false);

  setXMLFile( "knodeui.rc" );
}

KNodePart::~KNodePart()
{
  mainWidget->prepareShutdown();
}

KAboutData *KNodePart::createAboutData()
{
  return new KNode::AboutData();
}

bool KNodePart::openFile()
{
  kDebug(5003) << "KNodePart:openFile()" << endl;

  mainWidget->show();
  return true;
}

void KNodePart::guiActivateEvent(KParts::GUIActivateEvent *e)
{
  kDebug(5003) << "KNodePart::guiActivateEvent" << endl;
  KParts::ReadOnlyPart::guiActivateEvent(e);
}


QWidget* KNodePart::parentWidget() const
{
  return mParentWidget;
}



#include "knode_part.moc"

