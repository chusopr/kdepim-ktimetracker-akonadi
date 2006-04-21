/*
    This file is part of kdepim.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kmailchanges.h"
#include "kolabconfig.h"

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kemailsettings.h>
#include <identitymanager.h>
#include <identity.h>
#include <kdebug.h>

static const char* s_folderContentsType[] = {
  I18N_NOOP( "Calendar" ),
  I18N_NOOP( "Contacts" ),
  I18N_NOOP( "Notes" ),
  I18N_NOOP( "Tasks" ),
  I18N_NOOP( "Journal" ) };


class CreateImapAccount : public KConfigPropagator::Change
{
protected:
    CreateImapAccount( const QString& title )
        : KConfigPropagator::Change( title )
    {
    }
    
    void constructUserDomainAndEmail()
    {
      mDefaultDomain = KolabConfig::self()->server();
      mUser = KolabConfig::self()->user();
      int pos = mUser.find( "@" );
      // with kolab the userid _is_ the full email
      if ( pos > 0 ) {
        // The user typed in a full email address. Assume it's correct
        mEmail = mUser;
        const QString h = mUser.mid( pos+1 );
        if ( !h.isEmpty() )
          // The user did type in a domain on the email address. Use that
          mDefaultDomain = h;
      }
      else
        // Construct the email address. And use it for the username also
        mUser = mEmail = mUser+"@"+KolabConfig::self()->server();
    }

    // This sucks as this is a 1:1 copy from KMAccount::encryptStr()
    static QString encryptStr(const QString& aStr)
    {
      QString result;
      for (uint i = 0; i < aStr.length(); i++)
        result += (aStr[i].unicode() < 0x20) ? aStr[i] :
          QChar(0x1001F - aStr[i].unicode());
      return result;

    }

    QString mUser;
    QString mEmail;
    QString mDefaultDomain;
};

class CreateDisconnectedImapAccount : public CreateImapAccount
{
  public:
    CreateDisconnectedImapAccount()
      : CreateImapAccount( i18n("Create Disconnected IMAP Account for KMail") )
    {
    }

    void apply()
    {
      constructUserDomainAndEmail();

      KConfig c( "kmailrc" );
      c.setGroup( "General" );
      c.writeEntry( "Default domain", mDefaultDomain );
      uint accCnt = c.readNumEntry( "accounts", 0 );
      c.writeEntry( "accounts", accCnt+1 );
      uint transCnt = c.readNumEntry( "transports", 0 );
      c.writeEntry( "transports", transCnt+1 );

      c.setGroup( QString("Account %1").arg(accCnt+1) );
      int uid = kapp->random();
      c.writeEntry( "Folder", uid );
      c.writeEntry( "Id", uid );
      c.writeEntry( "Type", "cachedimap");
      c.writeEntry( "auth", "*");
      c.writeEntry( "Name", "Kolab Server" );
      c.writeEntry( "host", KolabConfig::self()->server() );

      c.writeEntry( "login",mUser );

      if ( KolabConfig::self()->savePassword() ) {
        c.writeEntry( "pass", encryptStr(KolabConfig::self()->password()) );
        c.writeEntry( "store-passwd", true );
      }
      c.writeEntry( "port", "993" );
      c.writeEntry( "use-ssl", true );

      c.writeEntry( "sieve-support", "true" );

      c.setGroup( QString("Folder-%1").arg( uid ) );
      c.writeEntry( "isOpen", true );

      c.setGroup( QString("Transport %1").arg(transCnt+1) );
      c.writeEntry( "name", "Kolab Server" );
      c.writeEntry( "host", KolabConfig::self()->server() );
      c.writeEntry( "type", "smtp" );
      c.writeEntry( "port", "465" );
      c.writeEntry( "encryption", "SSL" );
      c.writeEntry( "auth", true );
      c.writeEntry( "authtype", "PLAIN" );
      c.writeEntry( "user", mEmail );
      c.writeEntry( "pass", encryptStr(KolabConfig::self()->password()) );
      c.writeEntry( "storepass", "true" );

      // Write email in "default kcontrol settings", used by IdentityManager
      // if it has to create a default identity.
      KEMailSettings es;
      es.setSetting( KEMailSettings::RealName, KolabConfig::self()->realName() );
      es.setSetting( KEMailSettings::EmailAddress, mEmail );

      KPIM::IdentityManager identityManager;
      if ( !identityManager.allEmails().contains( mEmail ) ) {
        // Not sure how to name the identity. First one is "Default", next one "Kolab", but then...
        // let's use the server name after that.
        QString accountName = i18n( "Kolab" );
        const QStringList identities = identityManager.identities();
        if ( identities.find( accountName ) != identities.end() ) {
          accountName = KolabConfig::self()->server();
          int i = 2;
          // And if there's already one, number them
          while ( identities.find( accountName ) != identities.end() ) {
            accountName = KolabConfig::self()->server() + " " + QString::number( i++ );
          }
        }

        KPIM::Identity& identity = identityManager.newFromScratch( accountName );
        identity.setFullName( KolabConfig::self()->realName() );
        identity.setEmailAddr( mEmail );
        identityManager.commit();
      }

      // This needs to be done here, since it reference just just generated id
      c.setGroup( "IMAP Resource" );
      c.writeEntry("TheIMAPResourceAccount", uid);
      c.writeEntry("TheIMAPResourceFolderParent", QString(".%1.directory/INBOX").arg( uid ));
   }

};

class CreateOnlineImapAccount : public CreateImapAccount
{
  public:
    CreateOnlineImapAccount()
      : CreateImapAccount( i18n("Create Online IMAP Account for KMail") )
    {
    }

    void apply()
    {
      constructUserDomainAndEmail();

      KConfig c( "kmailrc" );
      c.setGroup( "General" );
      uint accCnt = c.readNumEntry( "accounts", 0 );
      c.writeEntry( "accounts", accCnt+1 );

      c.setGroup( QString("Account %1").arg(accCnt+1) );
      int uid = kapp->random();
      c.writeEntry( "Folder", uid );
      c.writeEntry( "Id", uid );
      c.writeEntry( "Type", "imap");
      c.writeEntry( "auth", "*");
      c.writeEntry( "Name", "Kolab Server Mail" );
      c.writeEntry( "host", KolabConfig::self()->server() );

      c.writeEntry( "login", mUser );

      if ( KolabConfig::self()->savePassword() ) {
        c.writeEntry( "pass", encryptStr(KolabConfig::self()->password()) );
        c.writeEntry( "store-passwd", true );
      }
      c.writeEntry( "port", "993" );
      c.writeEntry( "use-ssl", true );

      c.writeEntry( "sieve-support", "true" );

      // locally unsubscribe the default folders
      c.writeEntry( "locally-subscribed-folders", true );
      QString groupwareFolders = QString("/INBOX/%1/,/INBOX/%2/,/INBOX/%3/,/INBOX/%4/,/INBOX/%5/")
          .arg( s_folderContentsType[0] ).arg( s_folderContentsType[1] )
          .arg( s_folderContentsType[2] ).arg( s_folderContentsType[3] )
          .arg( s_folderContentsType[4] );
      c.writeEntry( "locallyUnsubscribedFolders", groupwareFolders );

      c.setGroup( QString("Folder-%1").arg( uid ) );
      c.writeEntry( "isOpen", true );
    }
};


void createKMailChanges( KConfigPropagator::Change::List& changes )
{
  KConfigPropagator::ChangeConfig *c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "Enabled";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "AutoAccept";
  c->value = "false";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "AutoDeclConflict";
  c->value = "false";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "LegacyMangleFromToHeaders";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "LegacyBodyInvites";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "Enabled";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "TheIMAPResourceEnabled";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "TheIMAPResourceStorageFormat";
  c->value = "XML";
  changes.append( c );

  if ( KolabConfig::self()->useOnlineForNonGroupware() ) {
      c = new KConfigPropagator::ChangeConfig;
      c->file = "kmailrc";
      c->group = "IMAP Resource";
      c->name = "ShowOnlyGroupwareFoldersForGroupwareAccount";
      c->value = "true";
      changes.append( c );
      changes.append( new CreateOnlineImapAccount );
  }

  changes.append( new CreateDisconnectedImapAccount );
}

