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

#ifndef _WEBSITE_H_
#define _WEBSITE_H_

namespace Nepomuk {
}

#include "informationelement.h"
#include <nepomuk/nepomuk_export.h>

namespace Nepomuk {

/**
 * A website, usually a container for remote resources, that may 
 * be interpreted as HTMLDocuments, images or other types of content. 
 */
    class NEPOMUK_EXPORT Website : public InformationElement
    {
    public:
        /**
         * Create a new empty and invalid Website instance
         */
        Website();
        /**
         * Default copy constructor
         */
        Website( const Website& );
        Website( const Resource& );
        /**
         * Create a new Website instance representing the resource
         * referenced by \a uriOrIdentifier.
         */
        Website( const QString& uriOrIdentifier );
        /**
         * Create a new Website instance representing the resource
         * referenced by \a uri.
         */
        Website( const QUrl& uri );
        ~Website();

        Website& operator=( const Website& );

            /**
             * Retrieve a list of all available Website resources. This list 
             * consists of all resource of type Website that are stored in the 
             * local Nepomuk meta data storage and any changes made locally. 
             * Be aware that in some cases this list can get very big. Then it 
             * might be better to use libKNep directly. 
             */
            static QList<Website> allWebsites();


        /**
         * \return The URI of the resource type that is used in Website instances.
         */
        static QString resourceTypeUri();

    protected:
       Website( const QString& uri, const QUrl& type );
       Website( const QUrl& uri, const QUrl& type );
   };
}

#endif
