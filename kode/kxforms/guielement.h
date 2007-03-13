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
#ifndef KXFORMS_GUIELEMENT_H
#define KXFORMS_GUIELEMENT_H

#include "reference.h"

#include <QList>
#include <QDomElement>
#include <QLabel>

namespace KXForms {

class Manager;

class GuiElement : public QObject
{
  public:
    typedef QList<GuiElement *> List;

    GuiElement( QWidget *parent, Manager *m );
    virtual ~GuiElement();

    virtual void parseElement( const QDomElement & ) {}

    void setRef( const Reference & );
    Reference ref() const;

    QDomElement context() const;

    QDomElement createElement( const Reference & );

    void loadData( const QDomElement &context );

    virtual QWidget *widget() { return mWidget; }
    virtual QWidget *labelWidget() { return mLabel; }

    virtual void loadData() = 0;
    virtual void saveData() = 0;

  protected:
    QWidget *mParent;
    QLabel *mLabel;
    QWidget *mWidget;
    Manager *mManager;

  private:
    Reference mRef;
    QDomElement mContext;
};

}

#endif
