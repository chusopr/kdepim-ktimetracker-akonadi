/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

import Qt 4.6

Item {
  property string title
  property int handlePosition
  property alias contentData: contentArea.data

  Rectangle {
    id: background
    x: -12
    y: handlePosition
    gradient: Gradient {
      GradientStop { position: 0.0; color: "lightgrey" }
      GradientStop { position: 0.5; color: "grey" }
    }
    effect: DropShadow {
      blurRadius: 8
      offset.x: 4
      offset.y: 4
    }
    radius: 12

//     height: titleLabel.width
//   //   width: titleLabel.height
    width: 64
     height: 160

    Text {
      id: titleLabel
      anchors.left: parent.right
      anchors.top: parent.top
      width: parent.height
      text: parent.parent.title
      transformOrigin: "TopLeft"
      rotation: 90
      horizontalAlignment: "AlignHCenter"
      verticalAlignment: "AlignTop"
      font.bold: true
      font.pixelSize: 36
    }

    MouseArea {
      anchors.fill: parent
      drag.target: parent
      drag.axis: "XAxis"
      drag.minimumX: -12
      drag.maximumX: 12
    }

    Item {
      id: contentArea
      visible: false
      anchors.fill: parent // TODO border, only visible space, etc
    }

    states: [
      State {
        name: "expandedState"
        when: background.x > 0
        PropertyChanges {
          target: background
          height: parent.parent.height - 40
          width: parent.parent.width - 40
          y: 20
        }
        PropertyChanges { target: titleLabel; visible: false }
        PropertyChanges { target: contentArea; visible: true }
      }
    ]

  }

}
