/* -*- mode: c++; c-basic-offset:4 -*-
    selftest/libkleopatrarccheck.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "libkleopatrarccheck.h"

#include "implementation_p.h"

#include <utils/archivedefinition.h>
#include <utils/checksumdefinition.h>

#include <KLocale>
#include <KMessageBox>

#include <QSettings>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::_detail;
using namespace boost;

namespace {

    class LibKleopatraRcCheck : public SelfTestImplementation {
    public:
        explicit LibKleopatraRcCheck()
            : SelfTestImplementation( i18nc("@title", "Config File 'libkleopatrarc'") )
        {
            runTest();
        }

        void runTest() {

            QStringList errors;
            ArchiveDefinition::getArchiveDefinitions( errors );
            ChecksumDefinition::getChecksumDefinitions( errors );

            m_passed = errors.empty();
            if ( m_passed )
                return;
            m_error = i18n("Errors found");

            m_explaination
                = i18nc( "@info",
                         "<para>Kleopatra detected the following errors in the libkleopatrarc configuration:</para>"
                         "%1", "<ol><li>" + errors.join( "</li><li>" ) + "</li></ol>" );
        }

        ///* reimp */ bool canFixAutomatically() const { return false; }
    };
}

shared_ptr<SelfTest> Kleo::makeLibKleopatraRcSelfTest() {
    return shared_ptr<SelfTest>( new LibKleopatraRcCheck );
}
