/***************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser                             *
*   geiseri@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef KDEDEVELOPERAPI_H
#define KDEDEVELOPERAPI_H

#include <bloginterface.h>

/**
Implementation for kdedevelopersAPI

@author ian reinhart geiser
*/
namespace KXMLRPC
{
	class Server;
};

class kdedevelopersAPI : public blogInterface
{
		Q_OBJECT
	public:
		kdedevelopersAPI( const KURL &server, QObject *parent = 0L, const char *name = 0L );
		~kdedevelopersAPI();

	public slots:
		void initServer();
		void getBlogs();
		void post( const blogPost& post, bool publish = false );
		void editPost( const blogPost& post, bool publish = false );
		void fetchPosts( const QString &blogID, int maxPosts );
		void fetchPost( const QString &postID );
		// void fetchTemplates();
		void deletePost( const QString &postID );

	private slots:
		void userInfoFinished( const QValueList<QVariant> & );
		void listFinished( const QValueList<QVariant> & );
		void blogListFinished( const QValueList<QVariant> & );
		void deleteFinished( const QValueList<QVariant> & );
		void getFinished( const QValueList<QVariant> & );
		void postFinished( const QValueList<QVariant> & );
		void fault( int, const QString& );


	private:
		void dumpBlog( const blogPost &blog );
		KXMLRPC::Server *m_xmlrpcServer;
		bool isValid;
};

#endif
