/*
    Copyright (c) 2010 Montel Laurent <montel@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

// TODO: i18n??
var page = Dialog.addPage( "mailboxwizard.ui", "Personal Settings" );

function validateInput()
{
  if ( page.widget().mailboxPath.text == "" ) {
    page.setValid( false );
  } else {
    page.setValid( true );
  }
}

function setup()
{
  var mboxRes = SetupManager.createResource( "akonadi_mbox_resource" );
  mboxRes.setOption( "Path", page.widget().mailboxPath.text );

  SetupManager.execute();
}
connect( page.widget().mailboxPath, "textChanged(QString)", this, "validateInput()" );
connect( page, "pageLeftNext()", this, "setup()" );
validateInput();
