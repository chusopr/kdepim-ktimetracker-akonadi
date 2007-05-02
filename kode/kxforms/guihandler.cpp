/*
    This file is part of KXForms.

    Copyright (c) 2005,2006 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "guihandler.h"

#include "manager.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>

using namespace KXForms;

GuiHandler::GuiHandler( Manager *m )
  : mManager( m ), mLayoutStyle( Grid )
{
  mManager->setGuiHandler( this );
}

GuiHandler::~GuiHandler()
{
}

Manager *GuiHandler::manager() const
{
  return mManager;
}


QLayout *GuiHandler::getTopLayout() const
{
  return new QGridLayout();
}

void GuiHandler::addWidget( QLayout *l, QWidget *w ) const
{
  if( layoutStyle() == GuiHandler::Grid ) {
    QGridLayout *gl = dynamic_cast< QGridLayout *>( l );
    if( !gl )
      return;

    gl->addWidget( w, gl->rowCount(), 0, 1, 3 );
  }
  else {
    QVBoxLayout *vbl = dynamic_cast< QVBoxLayout *>( l );
    if( !vbl )
      return;

    vbl->insertWidget( l->count() - 1, w );
  }
}

void GuiHandler::addElement( QLayout *l, QWidget *label, QWidget *widget, 
            GuiElement::Properties *prop, bool indented ) const
{
  Q_UNUSED( prop );
  if( layoutStyle() == GuiHandler::Grid ) {
    QGridLayout *gl = dynamic_cast< QGridLayout *>( l );
    if( !gl )
      return;

    int row = gl->rowCount();
    gl->setColumnStretch( 2, 1 );
    if( label ) {
      if( indented ) {
        gl->addWidget( label, row, 0, 1, 2, Qt::AlignRight | Qt::AlignTop );
        gl->addWidget( widget, row, 2 );
      }
      else {
        gl->addWidget( label, row, 0, Qt::AlignRight | Qt::AlignTop );
        gl->addWidget( widget, row, 1, 1, 2 );
      }
    }
    else {
      gl->addWidget( widget, row, 0, 1, 3 );
    }
  }
  else {
    QVBoxLayout *vbl = dynamic_cast< QVBoxLayout *>( l );
    if( !vbl )
      return;

    QBoxLayout *newLayout;
    newLayout = new QVBoxLayout();

    if( indented ) {
      QBoxLayout *labelLayout = new QHBoxLayout();
      labelLayout->addSpacing( 40 );
      labelLayout->addWidget( label, Qt::AlignRight | Qt::AlignTop );
      QBoxLayout *widgetLayout = new QHBoxLayout();
      widgetLayout->addSpacing( 40 );
      widgetLayout->addWidget( widget, Qt::AlignRight | Qt::AlignTop );
      newLayout->addLayout( labelLayout );
      newLayout->addLayout( widgetLayout );
    } else {
      newLayout->addWidget( label, Qt::AlignRight | Qt::AlignTop );
      newLayout->addWidget( widget );
    }

    vbl->insertLayout( l->count() - 1, newLayout );
  }
}

void GuiHandler::addElement( QLayout *l, QWidget *label, QWidget *widget, int x, int y, int width, int height, 
            GuiElement::Properties *prop, bool indented ) const
{
  Q_UNUSED( prop );

  if( layoutStyle() == GuiHandler::Grid ) {
    QGridLayout *gl = dynamic_cast< QGridLayout *>( l );
    if( !gl )
      return;

    gl->setColumnStretch( (2*x)+1, 1 );
    if( label ) {
      qDebug() << "Adding label at: (" << y << "," << 2*x << ")";
      gl->addWidget( label, y, (2*x), height, 1, Qt::AlignRight | Qt::AlignTop );
      qDebug() << "Adding widget at: (" << y << "," << (2*x)+1 << ") (" << height << "," << (2*width)-1  << ")";
      if( indented ) {
        QHBoxLayout *hbl = new QHBoxLayout();
        hbl->addSpacing( 40 );
        hbl->addWidget( widget );
        gl->addLayout( hbl, y, (2*x)+1, height, (2*width)-1 );
      }
      else
        gl->addWidget( widget, y, (2*x)+1, height, (2*width)-1 );
    }
    else {
      if( indented ) {
        QHBoxLayout *hbl = new QHBoxLayout();
        hbl->addSpacing( 40 );
        hbl->addWidget( widget );
        gl->addLayout( hbl, y, (2*x), height, 2*width );
      }
        gl->addWidget( widget, y, (2*x), height, 2*width );
    }
  }
  else {
    QGridLayout *gl = dynamic_cast< QGridLayout *>( l );
    if( !gl )
      return;

    gl->setColumnStretch( x, 1 );
    if( label ) {
      qDebug() << "Adding label at: (" << 2*y << "," << x << ")";
      gl->addWidget( label, 2*y, x, 1, width, Qt::AlignLeft | Qt::AlignTop );
      qDebug() << "Adding widget at: (" << (2*y)+1 << "," << x << ") (" << (2*height)-1 << "," << width  << ")";
      if( indented ) {
        QHBoxLayout *hbl = new QHBoxLayout();
        hbl->addSpacing( 40 );
        hbl->addWidget( widget );
        gl->addLayout( hbl, (2*y)+1, x, (2*height)-1, width );
      }
      else
        gl->addWidget( widget, (2*y)+1, x, (2*height)-1, width );
    }
    else {
      if( indented ) {
        QHBoxLayout *hbl = new QHBoxLayout();
        hbl->addSpacing( 40 );
        hbl->addWidget( widget );
        gl->addLayout( hbl, 2*y, x, 2*height, width );
      }
        gl->addWidget( widget, 2*y, x, 2*height, width );
    }
  }
}
