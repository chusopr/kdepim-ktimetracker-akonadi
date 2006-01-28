/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef LIBSYNDICATION_RDF_RESOURCEWRAPPER_H
#define LIBSYNDICATION_RDF_RESOURCEWRAPPER_H

#include <kdepimmacros.h>

#include <ksharedptr.h>

namespace LibSyndication {
namespace RDF {

class Resource;
typedef KSharedPtr<Resource> ResourcePtr;

/**
 * A wrapper for RDF resources. Base class for convenience wrappers
 * such as @see Document, @see Item etc.
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT ResourceWrapper
{
    public:
        ResourceWrapper();
        ResourceWrapper(const ResourceWrapper& other);
        ResourceWrapper(ResourcePtr resource);
        virtual ~ResourceWrapper();

        ResourceWrapper& operator=(const ResourceWrapper& other);
        bool operator==(const ResourceWrapper& other) const;
        
        /**
         * returns the wrapped resource.
         */
        ResourcePtr resource() const;

        /**
         * returns whether the wrapped resource is a null resource
         * @return @c true if isNull() is true for the wrapped resource,
         * @c false otherwise
         */
        bool isNull() const;

    private:

        class ResourceWrapperPrivate;
        KSharedPtr<ResourceWrapperPrivate> d;
};

} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_RESOURCEWRAPPER_H
