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
  if( layoutStyle() == GuiHandler::Grid ) {
    QGridLayout *l = new QGridLayout();
    return l;
  }
  else {
    QVBoxLayout *l = new QVBoxLayout();
    l->addStretch( 1 );
    return l;
  }
}

void GuiHandler::addWidget( QLayout *l, QWidget *w ) const
{
  if( layoutStyle() == GuiHandler::Grid ) {
    QGridLayout *gl = dynamic_cast< QGridLayout *>( l );
    if( !gl )
      return;

    gl->addWidget( w, gl->rowCount(), 0, 1, 2 );
  }
  else {
    QVBoxLayout *vbl = dynamic_cast< QVBoxLayout *>( l );
    if( !vbl )
      return;

    vbl->insertWidget( l->count() - 1, w );
  }
}

void GuiHandler::addElement( QLayout *l, QWidget *label, QWidget *widget, GuiElement::Properties *prop ) const
{
  if( layoutStyle() == GuiHandler::Grid ) {
    QGridLayout *gl = dynamic_cast< QGridLayout *>( l );
    if( !gl )
      return;

    int row = gl->rowCount();
    if( label ) {
      gl->addWidget( label, row, 0, prop->alignment() );
      gl->addWidget( widget, row, 1, prop->alignment() );
    }
    else {
      gl->addWidget( widget, row, 0, 1, 2, prop->alignment() );
    }
  }
  else {
    QVBoxLayout *vbl = dynamic_cast< QVBoxLayout *>( l );
    if( !vbl )
      return;

    QBoxLayout *newLayout;
    newLayout = new QVBoxLayout();

    newLayout->addWidget( label, prop->alignment() );
    newLayout->addWidget( widget, prop->alignment() );

    vbl->insertLayout( l->count() - 1, newLayout );
  }
}
