/*
    Copyright (C) 2007 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <kcmdlineargs.h>

#include <messagecomposer/kmeditor.h>

#include <QtCore/QFile>

using namespace Message;

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "testkmeditor", 0, ki18n("KMeditorTest"), "1.0" , ki18n("kmeditor test app"));
    KApplication app;
    KMeditor *edit = new KMeditor();
    edit->setAcceptRichText(false);
    edit->resize( 600, 600 );
    edit->show();
    return app.exec();
}
