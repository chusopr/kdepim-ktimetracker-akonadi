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

#ifndef _HARDDISKPARTITION_H_
#define _HARDDISKPARTITION_H_

namespace Nepomuk {
}

#include "dataobject.h"
#include <nepomuk/nepomuk_export.h>

namespace Nepomuk {

/**
 * A partition on a hard disk 
 */
    class NEPOMUK_EXPORT HardDiskPartition : public DataObject
    {
    public:
        /**
         * Create a new empty and invalid HardDiskPartition instance
         */
        HardDiskPartition();
        /**
         * Default copy constructor
         */
        HardDiskPartition( const HardDiskPartition& );
        HardDiskPartition( const Resource& );
        /**
         * Create a new HardDiskPartition instance representing the resource
         * referenced by \a uriOrIdentifier.
         */
        HardDiskPartition( const QString& uriOrIdentifier );
        /**
         * Create a new HardDiskPartition instance representing the resource
         * referenced by \a uri.
         */
        HardDiskPartition( const QUrl& uri );
        ~HardDiskPartition();

        HardDiskPartition& operator=( const HardDiskPartition& );

            /**
             * Retrieve a list of all available HardDiskPartition resources. 
             * This list consists of all resource of type HardDiskPartition 
             * that are stored in the local Nepomuk meta data storage and any 
             * changes made locally. Be aware that in some cases this list can 
             * get very big. Then it might be better to use libKNep directly. 
             */
            static QList<HardDiskPartition> allHardDiskPartitions();


        /**
         * \return The URI of the resource type that is used in HardDiskPartition instances.
         */
        static QString resourceTypeUri();

    protected:
       HardDiskPartition( const QString& uri, const QUrl& type );
       HardDiskPartition( const QUrl& uri, const QUrl& type );
   };
}

#endif
