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

import Qt 4.7 as QML
import org.kde 4.5

/** Window bar containing the task switcher buttom for full screen apps. */
QML.BorderImage {
  source: KDE.locate( "data", "mobileui/top.png" );
  border { left: 110; right: 1; top: 0; bottom: 0 }
  horizontalTileMode: QML.BorderImage.Repeat
  verticalTileMode: QML.BorderImage.Repeat
  anchors.top: parent.top
  anchors.left: parent.left
  anchors.right: parent.right

  QML.MouseArea {
    anchors.top: parent.top
    anchors.left: parent.left
    width: 80
    height: 40
    onClicked: {
      window.triggerTaskSwitcher();
    }
  }
}
