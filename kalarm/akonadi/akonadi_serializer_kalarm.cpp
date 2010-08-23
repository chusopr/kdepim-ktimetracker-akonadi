/*
 *  akonadi_serializer_kalarm.cpp  -  Akonadi resource serializer for KAlarm
 *  Program:  kalarm
 *  Copyright © 2009,2010 by David Jarvie <djarvie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 */

#include "akonadi_serializer_kalarm.h"
#include "kacalendar.h"
#include "kaevent.h"

#include <akonadi/item.h>
#include <kdebug.h>

#include <QtCore/qplugin.h>


using namespace Akonadi;

bool SerializerPluginKAlarm::deserialize(Item& item, const QByteArray& label, QIODevice& data, int version)
{
    Q_UNUSED(version);

    if (label != Item::FullPayload)
        return false;

    KCalCore::Incidence::Ptr i = mFormat.fromString(QString::fromUtf8(data.readAll()));
    if (!i)
    {
        kWarning(5263) << "Failed to parse incidence!";
        data.seek(0);
        kWarning(5263) << QString::fromUtf8(data.readAll());
        return false;
    }
    if (i->type() != KCalCore::Incidence::TypeEvent)
    {
        kWarning(5263) << "Incidence with uid" << i->uid() << "is not an Event!";
        data.seek(0);
        return false;
    }
    KAEvent event(i.staticCast<KCalCore::Event>());
    QString mime = KAlarm::CalEvent::mimeType(event.category());
    if (mime.isEmpty())
    {
        kWarning(5263) << "Event with uid" << event.id() << "contains no usable alarms!";
        data.seek(0);
        return false;
    }

    item.setMimeType(mime);
    item.setPayload<KAEvent>(event);
    return true;
}

void SerializerPluginKAlarm::serialize(const Item& item, const QByteArray& label, QIODevice& data, int& version)
{
    Q_UNUSED(version);

    if (label != Item::FullPayload || !item.hasPayload<KAEvent>())
        return;
    KAEvent e = item.payload<KAEvent>();
    KCalCore::Event::Ptr kcalEvent(new KCalCore::Event);
    e.updateKCalEvent(kcalEvent, KAEvent::UID_SET);
    QByteArray head = "BEGIN:VCALENDAR\nPRODID:";
    head += KAlarm::Calendar::icalProductId();
    head += "\nVERSION:2.0\nX-KDE-KALARM-VERSION:";
    head += KAEvent::currentCalendarVersionString();
    head += '\n';
    data.write(head);
    data.write(mFormat.toString(kcalEvent.staticCast<KCalCore::Incidence>()).toUtf8());
    data.write("\nEND:VCALENDAR");
}

Q_EXPORT_PLUGIN2(akonadi_serializer_kalarm, SerializerPluginKAlarm)

#include "akonadi_serializer_kalarm.moc"

// vim: et sw=4:
