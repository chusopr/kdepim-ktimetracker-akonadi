/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 2003, Daniel Martin <daniel.martin@pirack.com>
               2003, Michael Brade <brade@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
*******************************************************************/

#include <qtimer.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qcstring.h>

#include <kextsock.h>
#include <ksockaddr.h>
#include <kglobal.h>
#include <klocale.h>

#include "knotesnetrecv.h"

// Maximum note size in chars we are going to accept,
// to prevent "note floods".
#define MAXBUFFER 4096

// Maximum time we are going to wait between data receptions,
// to prevent memory and connection floods. In milliseconds.
#define MAXTIME 10000

// Small buffer's size
#define SBSIZE 512


KNotesNetworkReceiver::KNotesNetworkReceiver( KExtendedSocket *s )
  : QObject(),
    m_buffer( new QByteArray() ), m_sock( s )
{
    m_noteHeader = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(), true, true );

    // We forge a note beginning with the remote address, to help the user
    // guess who wrote it.
    m_notePrefix = i18n("From: %1\n").arg( m_sock->peerAddress()->nodeName() );

    // Setup the timer
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()), SLOT(slotReceptionTimeout()) );

    // Setup the communications
    connect( m_sock, SIGNAL(readyRead()), SLOT(slotDataAvailable()) );
    connect( m_sock, SIGNAL(closed( int )), SLOT(slotConnectionClosed( int )) );
    m_sock->enableRead( true );

    // Go!
    m_timer->start( MAXTIME, true );
}

KNotesNetworkReceiver::~KNotesNetworkReceiver()
{
    // Timer frees itself when its parent dies.
    m_timer->stop();
    delete m_buffer;
    delete m_sock;
}

void KNotesNetworkReceiver::slotDataAvailable()
{
    char smallBuffer[SBSIZE];
    int smallBufferLen;

    do
    {
        // Append to "big buffer", only if we have some space left.
        int curLen = m_buffer->count();

        smallBufferLen = m_sock->readBlock( smallBuffer, SBSIZE );
        if ( smallBufferLen > MAXBUFFER - curLen )
            smallBufferLen = ( MAXBUFFER - curLen );   // Limit max transfer over buffer, to avoid overflow.
        if ( smallBufferLen > 0 )
        {
            m_buffer->resize( curLen + smallBufferLen );
            memcpy( m_buffer->data() + curLen, smallBuffer, smallBufferLen );
        }
    }
    while ( smallBufferLen == SBSIZE );

    // If we are overflowing, close connection.
    if ( m_buffer->count() == MAXBUFFER )
        m_sock->closeNow();
    else
        m_timer->changeInterval( MAXTIME );
}

void KNotesNetworkReceiver::slotConnectionClosed( int /*state*/ )
{
    if ( m_timer->isActive() )
    {
        QString noteText = QString( *m_buffer ).stripWhiteSpace();

        // Remove first line with (possibly) fake Id.
        noteText = m_notePrefix + noteText.mid( noteText.find( QRegExp("[\r\n]") ) );

        // this also means we can't receive empty notes
        if ( noteText.length() > m_notePrefix.length() )
            emit sigNoteReceived( m_noteHeader, noteText );
    }

    delete this;
}

void KNotesNetworkReceiver::slotReceptionTimeout()
{
    m_sock->closeNow();
}

#include "knotesnetrecv.moc"
