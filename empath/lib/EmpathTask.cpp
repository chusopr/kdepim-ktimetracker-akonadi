/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

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


// KDE includes
#include <kapp.h>

// Local includes
#include "Empath.h"
#include "EmpathTask.h"
#include "EmpathDefines.h"

EmpathTask::EmpathTask(const QString & name)
    :    QObject(),
        name_(name),
        max_(0),
        pos_(0),
        done_(false),
        waitInterval_(1),
        waitCount_(0)
{
    startTime_ = QTime::currentTime();

    QObject::connect(
        this,   SIGNAL(newTask(EmpathTask *)),
        empath, SLOT(s_newTask(EmpathTask *)));

    startTimer(50);
}

EmpathTask::~EmpathTask()
{
    emit(finished());
}

    void
EmpathTask::setMax(int i)
{
    max_ = i;
    emit(maxChanged(i));
}

    void
EmpathTask::setPos(int i)
{
    pos_ = i;
    emit(posChanged(i));
}

    void
EmpathTask::doneOne()
{
    emit(addOne());
}

    void
EmpathTask::done()
{
    killTimers();
    done_ = true;
    emit(finished());
}

    void
EmpathTask::timerEvent(QTimerEvent *)
{
    killTimers();
    emit(newTask(this));
}

// vim:ts=4:sw=4:tw=78
