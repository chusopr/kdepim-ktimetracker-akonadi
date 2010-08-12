/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

QML.Rectangle {
  id : _topLevel
  color : "#00000000"
  property int actionItemHeight : 70
  property int actionItemWidth : 200
  property int actionItemSpacing : 0
  property int bottomMargin
  anchors.bottomMargin : bottomMargin

  property alias model : myList.model

  property alias customActions : actionColumn.content

  signal triggered(string triggeredName)

  QML.Component {
    id: highlightBar
    QML.Rectangle {
      width: myList.width
      height: 30
      color: "#FFFF88"
      y: myList.currentItem.y
    }
  }

  QML.ListView {
    id : myList
    anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
    width: parent.width - actionColumn.width
    focus: true
    clip: true
    highlight: highlightBar
    delegate : QML.Component {
      QML.Item {
        id: _delegateTopLevel
        height: _topLevel.actionItemHeight
        width: myList.width

        QML.Text {
          anchors.fill: parent
          text : model.display
          horizontalAlignment: QML.Text.AlignHCenter
          verticalAlignment: QML.Text.AlignVCenter
        }

        QML.MouseArea {
          anchors.fill: parent
          onClicked: { _delegateTopLevel.QML.ListView.view.currentIndex = model.index; }
        }
      }
    }

    onCurrentIndexChanged : {
      application.setAgentInstanceListSelectedRow( currentIndex )
    }
  }

  ActionMenuContainer {
    id : actionColumn
    width : _topLevel.actionItemWidth
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.right : parent.right
    actionItemWidth : width
    actionItemHeight : _topLevel.actionItemHeight
  }

  onActionItemSpacingChanged : {
    myColumn.refresh();
  }

  onActionItemHeightChanged : {
    myColumn.refresh();
  }
}
