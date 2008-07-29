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

#ifndef _EXECUTABLE_H_
#define _EXECUTABLE_H_

namespace Nepomuk {
}

#include "informationelement.h"
#include <nepomuk/nepomuk_export.h>

namespace Nepomuk {

/**
 * An executable file. 
 */
    class NEPOMUK_EXPORT Executable : public InformationElement
    {
    public:
        /**
         * Create a new empty and invalid Executable instance
         */
        Executable();
        /**
         * Default copy constructor
         */
        Executable( const Executable& );
        Executable( const Resource& );
        /**
         * Create a new Executable instance representing the resource
         * referenced by \a uriOrIdentifier.
         */
        Executable( const QString& uriOrIdentifier );
        /**
         * Create a new Executable instance representing the resource
         * referenced by \a uri.
         */
        Executable( const QUrl& uri );
        ~Executable();

        Executable& operator=( const Executable& );

            /**
             * Retrieve a list of all available Executable resources. This 
             * list consists of all resource of type Executable that are stored 
             * in the local Nepomuk meta data storage and any changes made locally. 
             * Be aware that in some cases this list can get very big. Then it 
             * might be better to use libKNep directly. 
             */
            static QList<Executable> allExecutables();


        /**
         * \return The URI of the resource type that is used in Executable instances.
         */
        static QString resourceTypeUri();

    protected:
       Executable( const QString& uri, const QUrl& type );
       Executable( const QUrl& uri, const QUrl& type );
   };
}

#endif
