/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "storesmsjob.h"

StoreSMSJob::StoreSMSJob( SMS* sms, Device *device, 
                          kmobiletoolsGammu_engine* parent, const char* name )
                        : GammuJob( device, parent, name ) {
    m_sms = sms;
}

StoreSMSJob::StoreSMSJob( const QString &number, const QString &text, Device *device, 
                          kmobiletoolsGammu_engine* parent, const char* name )
                        : GammuJob( device, parent, name ) {
    m_sms = new SMS();
    m_sms->setText( text );
    m_sms->setNumbers( QStringList( number ) );
}


void StoreSMSJob::run() {
    device()->storeSMS( m_sms );
}
