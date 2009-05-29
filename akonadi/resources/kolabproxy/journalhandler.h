/*
    Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Andras Mantia <andras@kdab.net>

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

#ifndef JOURNALHANDLER_H
#define JOURNALHANDLER_H

#include "incidencehandler.h"
#include <kcal/journal.h>
#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<KCal::Journal> JournalPtr;

/**
	@author Andras Mantia <amantia@kde.org>
*/
class JournalHandler : public IncidenceHandler {
public:
  JournalHandler();
  virtual ~JournalHandler();

  virtual QStringList contentMimeTypes();

private:
  virtual QByteArray incidenceToXml(KCal::Incidence *incidence);
  virtual KCal::Incidence* incidenceFromKolab(MessagePtr data);
  KCal::Journal *journalFromKolab(MessagePtr data);
};

#endif
