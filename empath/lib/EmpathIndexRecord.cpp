/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathIndexRecord.h"
#endif

// Qt includes
#include <qdatetime.h>
#include <qdatastream.h>

// Local includes
#include "Empath.h"
#include "EmpathIndexRecord.h"

EmpathIndexRecord::EmpathIndexRecord()
    :   id_                 (QString::null),
        subject_            (""),
        sender_             (""),
        date_               (""),
        status_             (RMM::MessageStatus(0)),
        size_               (0),
        messageId_          (""),
        parentMessageId_    (""),
        tagged_             (false)
{
}
        
EmpathIndexRecord::EmpathIndexRecord(const EmpathIndexRecord & i)
    :   id_                 (i.id_),
        subject_            (i.subject_),
        sender_             (i.sender_),
        date_               (i.date_),
        status_             (i.status_),
        size_               (i.size_),
        messageId_          (i.messageId_),
        parentMessageId_    (i.parentMessageId_),
        tagged_             (i.tagged_)
{
}


EmpathIndexRecord::EmpathIndexRecord(const QString & id, RMM::RMessage & m)
    :   id_                 (id),
        subject_            (m.envelope().subject().asString()),
        sender_             (m.envelope().firstSender()),
        date_               (m.envelope().date()),
        status_             (m.status()),
        size_               (m.size()),
        messageId_          (m.envelope().messageID()),
        parentMessageId_    (m.envelope().parentMessageId()),
        tagged_             (false)
{
}

EmpathIndexRecord::EmpathIndexRecord(
        const QString &     id,
        const QString &     subject,
        RMM::RMailbox &     sender,
        RMM::RDateTime &    date,
        RMM::MessageStatus  status,
        Q_UINT32            size,
        RMM::RMessageID &   messageId,
        RMM::RMessageID &   parentMessageId)
    :
        id_                 (id),
        subject_            (subject),
        sender_             (sender),
        date_               (date),
        status_             (status),
        size_               (size),
        messageId_          (messageId),
        parentMessageId_    (parentMessageId),
        tagged_             (false)
{
}

EmpathIndexRecord::~EmpathIndexRecord()
{
}

    bool
EmpathIndexRecord::hasParent()
{
    return !parentMessageId_.asString().isEmpty();
}

    QString
EmpathIndexRecord::niceDate(bool twelveHour)
{
    QDateTime now(QDateTime::currentDateTime());
    QDateTime then(date_.qdt());
    
    // Use difference between times to work out how old a message is, and see
    // if we can represent it in a more concise fashion.
    // FIXME: This is only for locale en_GB

    QString dts;
    
    // If the dates differ, then print the day of week..
    if (then.daysTo(now) != 0) {
        dts += then.date().dayName(then.date().dayOfWeek()) + " ";
    // Print the day of month.
        dts += QString().setNum(then.date().day()) + " ";
    }

    // If the months differ, print month name.
    if (then.date().month() != now.date().month())
        dts += then.date().monthName(then.date().month()) + " ";
        
    // If the message is from a different year, add that too.
    if (then.date().year() != now.date().year())
        dts += QString().setNum(then.date().year()) + " ";

    // If the day is the same, print the time of the message. 
    if (then.date().daysTo(now.date()) == 0) {
        
        int hour = then.time().hour();
        
        if (twelveHour) {

            if (hour > 12)
                dts += QString().setNum(hour - 12);
            else
                dts += QString().setNum(hour);

        } else {
        
            // Add leading nought to hour for twatty 24hr time.
            if (hour < 10) dts += "0";
            dts += QString().setNum(hour);
        }

        dts += ":";
        int m = then.time().minute();
        
        // Add leading nought to minute.
        if (m < 10) dts += "0";
        dts += QString().setNum(m);
        
        if (twelveHour) {
            if (hour > 12) dts += " PM";
            else dts += " AM";
        }
    }

    return dts;
}

    QDataStream &
operator << (QDataStream & s, EmpathIndexRecord & rec)
{
    s   << rec.id_
        << (unsigned short)rec.tagged_
        << rec.subject_
        << rec.sender_
        << rec.date_
        << (unsigned int)rec.status_
        << rec.size_
        << rec.messageId_
        << rec.parentMessageId_;

    return s;
}

    QDataStream &
operator >> (QDataStream & s, EmpathIndexRecord & rec)
{
    unsigned int i;
    unsigned short j;

    s   >> rec.id_
        >> j
        >> rec.subject_
        >> rec.sender_
        >> rec.date_
        >> i
        >> rec.size_
        >> rec.messageId_
        >> rec.parentMessageId_;

    rec.status_ = (RMM::MessageStatus)i;
    rec.tagged_ = (bool)j;
    
    return s;
}

    void
EmpathIndexRecord::setStatus(RMM::MessageStatus s)
{
    status_ = s;
}

// vim:ts=4:sw=4:tw=78
