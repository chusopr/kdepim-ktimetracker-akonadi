/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

import Qt 4.7 as QML
import org.kde.pim.mobileui 4.5 as KPIM


QML.Item {
  height : parent.height
  width : parent.width
  property string name
  property string argument : ""
  property string title : ""

  property string category

  signal triggered(string triggeredName)
  signal pressAndHold()

  onVisibleChanged :
  {
    if (!visible)
      height = -actionItemSpacing
    else
      height = actionItemHeight
  }

  KPIM.Action {
    height : parent.height
    width : parent.width
    action : {
      application.setActionTitle(name, title)
      application.getAction(name, argument);
    }
    hidable : false

    onLongPressed: {
      pressAndHold();
    }
    }
     

/*
  QML.Rectangle {
    height : parent.height
    width : parent.width
    color : "yellow"
    QML.Text {
      height : parent.height

      id : itemText
    }
    QML.MouseArea {
      anchors.fill : parent
      onClicked : { console.log("TRIG"); triggered(itemText.text) }
    }
  } */
}