/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
    Copyright (c) 2010 Eduardo Madeira Fleury <efleury@gmail.com>

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

import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

Dialog {
  id: clockWidget
  property alias okEnabled: clockWidgetOk.enabled

  property alias hours: hourSelector.value
  property alias minutes: minuteSelector.value

  property bool blockSignalEmission: false

  signal timeChanged( int hour, int minute )

  content: [
    Item {
      anchors.fill: parent


      KPIM.Clock {
        id: myClock
        anchors {
          left: parent.left
          top: parent.top
          bottom: amPmSwitch.top

          topMargin: 25
          bottomMargin: 25
        }

        onHoursChanged: {
          if ( clockWidget.blockSignalEmission )
            return;
          // ### TODO: instead of calling function just set value
          // was supposed to work
          hourSelector.setValue(amPmSwitch.on ? myClock.hours + 12 : myClock.hours);
        }

        onMinutesChanged: {
          // ### TODO: instead of calling function just set value
          // was supposed to work
          minuteSelector.setValue(myClock.minutes);
        }
      }

      KPIM.Switch {
        id: amPmSwitch
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: myClock.horizontalCenter

        onOnChanged: {
          if ( on ) { // pm selected
            if ( hourSelector.value < 12 ) {
              hourSelector.setValue( hourSelector.value + 12 );
            }
          } else { // am selected
            if ( hourSelector.value >= 12 ) {
              hourSelector.setValue( hourSelector.value - 12 );
            }
          }
        }
      }

      Column {
        spacing: 5
        anchors {
          top: parent.top
          left: myClock.right
          right: parent.right

          topMargin: 100
          leftMargin: 60
        }

        KPIM.VerticalSelector {
          id: hourSelector
          height: 100
          model: 24

          onValueChanged: {
            clockWidget.blockSignalEmission = true;
            myClock.hours = value >= 12 ? value - 12 : value;
            amPmSwitch.setOn( value >= 12 );
            clockWidgetOk.enabled = true;
            clockWidget.blockSignalEmission = false;
          }
          onSelected: {
            minuteSelector.state = "unselected";
          }
          Component.onCompleted: {
              // ### TODO: instead of calling function just set value
              // was supposed to work
              hourSelector.setValue(myClock.hours);
          }
        }

        KPIM.VerticalSelector {
          id: minuteSelector
          height: 100
          model: 60

          onValueChanged: {
            myClock.minutes = value;
            clockWidgetOk.enabled = true;
          }
          onSelected: {
            hourSelector.state = "unselected";
          }
          Component.onCompleted: {
              // ### TODO: instead of calling function just set value
              // was supposed to work
              minuteSelector.setValue(myClock.minutes);
          }
        }

      }
      Row {
        spacing: 5
        anchors{
          bottom: parent.bottom
          right: parent.right
        }
        KPIM.Button2 {
          id: clockWidgetCancel
          buttonText: KDE.i18n( "Cancel" );
          width: 100
          onClicked: {
            clockWidget.collapse()
            myClock.clearSelection()
            //### + reset widget
          }
        }
        KPIM.Button2 {
          id: clockWidgetOk
          enabled: false
          buttonText: KDE.i18n( "Ok" );
          width: 100
          onClicked: {
            clockWidget.collapse()
            myClock.clearSelection()
            timeChanged(hourSelector.value, minuteSelector.value);
          }
        }
      }
    }
  ]
}
