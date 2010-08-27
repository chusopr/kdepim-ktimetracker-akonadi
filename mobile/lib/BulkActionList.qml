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
import org.kde.akonadi 4.5
import org.kde.pim.mobileui 4.5 as KPIM

QML.Column {
  id : _top
  property alias selectedItemModel : selectedItem.model
  property alias multipleText : _multipleText.text

  signal backClicked()
  signal triggered(string name)

  property int itemHeight : 65

  onChildrenChanged: {
    var newChild = children[ children.length - 1 ];
    newChild.anchors.left = parent.left;
    newChild.anchors.right = parent.right;
    newChild.height = itemHeight;
  }

  QML.Item {
    anchors.left : parent.left
    anchors.right : parent.right
    height : itemHeight
    QML.ListView {
      id : selectedItem
      anchors.fill : parent
      delegate : CollectionDelegate {
        height : 70
        indentation : 80
      }
      visible : count == 1
    }
    QML.Text {
      id : _multipleText
      anchors.horizontalCenter : parent.horizontalCenter
      y : height / 2
      visible : selectedItem.count != 1
      height : 70
    }
    QML.Image {
      id : topLine
      source : "images/list-line-top.png"
      anchors.right : selectedItem.right
      anchors.top : selectedItem.top
    }
    QML.Image {
      id : topLineFiller
      source : "images/dividing-line-horizontal.png"
      anchors.right : topLine.left
      anchors.bottom : topLine.bottom
      fillMode : Image.TileHorizontally
      width : parent.width - topLine.width
    }
    QML.Image {
      id : bottomLine
      source : "images/dividing-line-horizontal.png"
      anchors.right : selectedItem.right
      anchors.bottom : selectedItem.bottom
      fillMode : QML.Image.TileHorizontally
      width : parent.width
    }
    QML.Image {
      source : "images/dividing-line.png"
      anchors.top : selectedItem.bottom
      anchors.right : parent.right
      height : _top.height - selectedItem.height
      fillMode : QML.Image.TileVertically
    }
    QML.Image {
      id : backIcon
      source : "images/bulk_back_overlay.png"
      anchors.right : parent.right
      anchors.verticalCenter : parent.verticalCenter
      QML.MouseArea {
        id : back_ma
        anchors.fill : parent
        onClicked : _top.backClicked()
      }
    }
  }
  KPIM.Action {
    height : itemHeight
    anchors.left : parent.left
    anchors.right : parent.right
    action : application.getAction("akonadi_item_delete", "")
    onTriggered : {
      _top.triggered("akonadi_item_delete")
    }
  }
  KPIM.Action {
    height : itemHeight
    anchors.left : parent.left
    anchors.right : parent.right
    action : application.getAction("akonadi_item_move_to_menu", "")
    onTriggered : {
      _top.triggered("akonadi_item_move_to_menu")
    }
  }
  KPIM.Action {
    height : itemHeight
    anchors.left : parent.left
    anchors.right : parent.right
    action : application.getAction("akonadi_item_copy_to_menu", "")
    onTriggered : {
      _top.triggered("akonadi_item_copy_to_menu")
    }
  }
}
