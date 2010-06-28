/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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

#include "desctab.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeView>

#include "entitytreewidget.h"
#include <akonadi/entitytreemodel.h>
#include <kdescendantsproxymodel.h>

#include <akonadi_next/modeltest.h>
#include <QTimer>

DescTabWidget::DescTabWidget(QWidget* parent, Qt::WindowFlags f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);
  layout->addWidget(splitter);

  m_etw = new EntityTreeWidget(splitter);
  m_etw->init();
  QTreeView *descView = new QTreeView(splitter);

  QTimer::singleShot(5000, this, SLOT(connectProxy()));

}

void DescTabWidget::connectProxy()
{

  m_etw->dumpTree();

  KDescendantsProxyModel *descProxy = new KDescendantsProxyModel(this);

  kDebug() << descProxy;
  descProxy->setSourceModel(m_etw->model());

  new ModelTest(descProxy, this);

//   descView->setModel(descProxy);
}
