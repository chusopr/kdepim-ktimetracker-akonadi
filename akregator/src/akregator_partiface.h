/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORPARTIFACE_H
#define AKREGATORPARTIFACE_H

#include <dcopobject.h>

namespace Akregator {

    class aKregatorPartIface : virtual public DCOPObject
    {
        K_DCOP
        k_dcop:
            virtual void fetchFeedUrl(const QString&) = 0;
            virtual void saveSettings() = 0;
    };

}

#endif
