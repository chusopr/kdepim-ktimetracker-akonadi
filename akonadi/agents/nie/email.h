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

#ifndef _EMAIL_H_
#define _EMAIL_H_

namespace Nepomuk {
        class Contact;
}

#include "message.h"
#include <nepomuk/nepomuk_export.h>

namespace Nepomuk {

/**
 * An email. 
 */
    class NEPOMUK_EXPORT Email : public Message
    {
    public:
        /**
         * Create a new empty and invalid Email instance
         */
        Email();
        /**
         * Default copy constructor
         */
        Email( const Email& );
        Email( const Resource& );
        /**
         * Create a new Email instance representing the resource
         * referenced by \a uriOrIdentifier.
         */
        Email( const QString& uriOrIdentifier );
        /**
         * Create a new Email instance representing the resource
         * referenced by \a uri.
         */
        Email( const QUrl& uri );
        ~Email();

        Email& operator=( const Email& );

            /**
             * Get property 'to'. The primary intended recipient of an email. 
             */
            QList<Contact> tos() const;

            /**
             * Set property 'to'. The primary intended recipient of an email. 
             */
            void setTos( const QList<Contact>& value );

            /**
             * Add a value to property 'to'. The primary intended recipient 
             * of an email. 
             */
            void addTo( const Contact& value );

            /**
             * \return The URI of the property 'to'. 
             */
            static QUrl toUri();

            /**
             * Get property 'bcc'. A Contact that is to receive a bcc of the email. 
             * A Bcc (blind carbon copy) is a copy of an email message sent to 
             * a recipient whose email address does not appear in the message. 
             */
            QList<Contact> bccs() const;

            /**
             * Set property 'bcc'. A Contact that is to receive a bcc of the email. 
             * A Bcc (blind carbon copy) is a copy of an email message sent to 
             * a recipient whose email address does not appear in the message. 
             */
            void setBccs( const QList<Contact>& value );

            /**
             * Add a value to property 'bcc'. A Contact that is to receive a bcc 
             * of the email. A Bcc (blind carbon copy) is a copy of an email message 
             * sent to a recipient whose email address does not appear in the 
             * message. 
             */
            void addBcc( const Contact& value );

            /**
             * \return The URI of the property 'bcc'. 
             */
            static QUrl bccUri();

            /**
             * Get property 'cc'. A Contact that is to receive a cc of the email. 
             * A cc (carbon copy) is a copy of an email message whose recipient 
             * appears on the recipient list, so that all other recipients 
             * are aware of it. 
             */
            QList<Contact> ccs() const;

            /**
             * Set property 'cc'. A Contact that is to receive a cc of the email. 
             * A cc (carbon copy) is a copy of an email message whose recipient 
             * appears on the recipient list, so that all other recipients 
             * are aware of it. 
             */
            void setCcs( const QList<Contact>& value );

            /**
             * Add a value to property 'cc'. A Contact that is to receive a cc 
             * of the email. A cc (carbon copy) is a copy of an email message whose 
             * recipient appears on the recipient list, so that all other recipients 
             * are aware of it. 
             */
            void addCc( const Contact& value );

            /**
             * \return The URI of the property 'cc'. 
             */
            static QUrl ccUri();

            /**
             * Get property 'contentMimeType'. Key used to store the MIME 
             * type of the content of an object when it is different from the 
             * object's main MIME type. This value can be used, for example, 
             * to model an e-mail message whose mime type is"message/rfc822", 
             * but whose content has type "text/html". If not specified, the 
             * MIME type of the content defaults to the value specified by the 
             * 'mimeType' property. 
             */
            QStringList contentMimeTypes() const;

            /**
             * Set property 'contentMimeType'. Key used to store the MIME 
             * type of the content of an object when it is different from the 
             * object's main MIME type. This value can be used, for example, 
             * to model an e-mail message whose mime type is"message/rfc822", 
             * but whose content has type "text/html". If not specified, the 
             * MIME type of the content defaults to the value specified by the 
             * 'mimeType' property. 
             */
            void setContentMimeTypes( const QStringList& value );

            /**
             * Add a value to property 'contentMimeType'. Key used to store 
             * the MIME type of the content of an object when it is different 
             * from the object's main MIME type. This value can be used, for 
             * example, to model an e-mail message whose mime type is"message/rfc822", 
             * but whose content has type "text/html". If not specified, the 
             * MIME type of the content defaults to the value specified by the 
             * 'mimeType' property. 
             */
            void addContentMimeType( const QString& value );

            /**
             * \return The URI of the property 'contentMimeType'. 
             */
            static QUrl contentMimeTypeUri();

            /**
             * Retrieve a list of all available Email resources. This list 
             * consists of all resource of type Email that are stored in the 
             * local Nepomuk meta data storage and any changes made locally. 
             * Be aware that in some cases this list can get very big. Then it 
             * might be better to use libKNep directly. 
             */
            static QList<Email> allEmails();


        /**
         * \return The URI of the resource type that is used in Email instances.
         */
        static QString resourceTypeUri();

    protected:
       Email( const QString& uri, const QUrl& type );
       Email( const QUrl& uri, const QUrl& type );
   };
}

#endif
