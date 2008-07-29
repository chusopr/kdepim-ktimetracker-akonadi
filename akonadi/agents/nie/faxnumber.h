/*
 *
 * $Id: $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

/*
 * This file has been generated by the Nepomuk Resource class generator.
 * DO NOT EDIT THIS FILE.
 * ANY CHANGES WILL BE LOST.
 */

#ifndef _FAXNUMBER_H_
#define _FAXNUMBER_H_

namespace Nepomuk {
}

#include "phonenumber.h"
#include <nepomuk/nepomuk_export.h>

namespace Nepomuk {

/**
 * A fax number. Inspired by the (TYPE=fax) parameter of the TEL 
 * property as defined in RFC 2426 sec 3.3.1. 
 */
    class NEPOMUK_EXPORT FaxNumber : public PhoneNumber
    {
    public:
        /**
         * Create a new empty and invalid FaxNumber instance
         */
        FaxNumber();
        /**
         * Default copy constructor
         */
        FaxNumber( const FaxNumber& );
        FaxNumber( const Resource& );
        /**
         * Create a new FaxNumber instance representing the resource
         * referenced by \a uriOrIdentifier.
         */
        FaxNumber( const QString& uriOrIdentifier );
        /**
         * Create a new FaxNumber instance representing the resource
         * referenced by \a uri.
         */
        FaxNumber( const QUrl& uri );
        ~FaxNumber();

        FaxNumber& operator=( const FaxNumber& );

            /**
             * Retrieve a list of all available FaxNumber resources. This 
             * list consists of all resource of type FaxNumber that are stored 
             * in the local Nepomuk meta data storage and any changes made locally. 
             * Be aware that in some cases this list can get very big. Then it 
             * might be better to use libKNep directly. 
             */
            static QList<FaxNumber> allFaxNumbers();


        /**
         * \return The URI of the resource type that is used in FaxNumber instances.
         */
        static QString resourceTypeUri();

    protected:
       FaxNumber( const QString& uri, const QUrl& type );
       FaxNumber( const QUrl& uri, const QUrl& type );
   };
}

#endif
