#ifndef KABC_SIMPLEFORMAT_H
#define KABC_SIMPLEFORMAT_H

#include <qstring.h>

#include "format.h"

namespace KABC {

class AddressBook;

class SimpleFormat : public Format {
  public:
    bool load( AddressBook *, const QString &fileName );
    bool save( AddressBook *, const QString &fileName );
};

}

#endif
