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

#ifdef __GNUG__
# pragma interface "RMM_MailboxList.h"
#endif

#ifndef RMM_RMAILBOXLIST_H
#define RMM_RMAILBOXLIST_H

#include <qstring.h>

#include <RMM_Mailbox.h>
#include <RMM_Defines.h>

namespace RMM {

/**
 * @short Simple encapsulation of a list of RMailbox
 * Simple encapsulation of a list of RMailbox, which is also an RHeaderBody.
 */
class RMailboxList : public RHeaderBody {

#include "generated/RMailboxList_generated.h"

    public:
        
        unsigned int count();
        RMailbox::Ptr at(int);
        void append(RMailbox);
        
    private:
        
        RMailbox::List list_;
};

}

#endif //RMAILBOXLIST_H
// vim:ts=4:sw=4:tw=78
