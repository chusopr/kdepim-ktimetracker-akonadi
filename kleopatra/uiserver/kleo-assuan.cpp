/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/kleo-assuan.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "kleo-assuan.h"

#include <QString>

#include <gpg-error.h>

Kleo::assuan_exception::assuan_exception( gpg_error_t e, const QString & msg )
            : std::runtime_error( make_message( e, msg.toLocal8Bit().data() ) ), m_error( e ), m_message( msg.toLocal8Bit() ) 
{}

Kleo::assuan_exception::assuan_exception( gpg_error_t e, const char* msg )
            : std::runtime_error( make_message( e, msg ) ), m_error( e ), m_message( msg ) 
{}


// static
std::string Kleo::assuan_exception::make_message( gpg_error_t err, const std::string & msg ) {    
    char buf[1024];
    buf[0] = '\0';
    gpg_strerror_r( err, buf, sizeof buf );
    buf[sizeof buf - 1] = '\0';
    char result_buf[1024];
    snprintf( result_buf, sizeof result_buf, "%lu %s: %s",
            (unsigned long)err, gpg_strsource( err ), buf  );
    result_buf[sizeof result_buf - 1] = '\0';
    std::string result = result_buf;
    result += " - ";
    result += msg;
    return result;
}
