// kfileio.cpp
// Author: Stefan Taferner <taferner@kde.org>
// License: GPL

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmessagebox.h>
#include <kdebug.h>

#include <assert.h>
#include <QDir>
#include <QByteArray>

#include <klocale.h>
#include <kstdguiitem.h>

#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <sys/stat.h>
#include <sys/types.h>

#include <kdepimmacros.h>

namespace KPIM {

//-----------------------------------------------------------------------------
static void msgDialog(const QString &msg)
{
  KMessageBox::sorry(0, msg, i18n("File I/O Error"));
}


//-----------------------------------------------------------------------------
KDE_EXPORT
QByteArray kFileToByteArray( const QString & aFileName, bool aEnsureNL, bool aVerbose )
{
  QByteArray result;
  QFileInfo info(aFileName);
  unsigned int readLen;
  unsigned int len = info.size();
  QFile file(aFileName);

  //assert(aFileName!=0);
  if( aFileName.isEmpty() )
    return "";

  if (!info.exists())
  {
    if (aVerbose)
      msgDialog(i18n("The specified file does not exist:\n%1", aFileName));
    return QByteArray();
  }
  if (info.isDir())
  {
    if (aVerbose)
      msgDialog(i18n("This is a folder and not a file:\n%1", aFileName));
    return QByteArray();
  }
  if (!info.isReadable())
  {
    if (aVerbose)
      msgDialog(i18n("You do not have read permissions "
				   "to the file:\n%1", aFileName));
    return QByteArray();
  }
  if (len <= 0) return QByteArray();

  if (!file.open(QIODevice::Unbuffered|QIODevice::ReadOnly))
  {
    if (aVerbose) switch(file.error())
    {
    case QFile::ReadError:
      msgDialog(i18n("Could not read file:\n%1", aFileName));
      break;
    case QFile::OpenError:
      msgDialog(i18n("Could not open file:\n%1", aFileName));
      break;
    default:
      msgDialog(i18n("Error while reading file:\n%1", aFileName));
    }
    return QByteArray();
  }

  result.resize( len + int( aEnsureNL ) );
  readLen = file.read( result.data(), len );
  if ( aEnsureNL ) {
    if ( result[readLen-1] != '\n' ) {
      result[readLen++] = '\n';
      len++;
    }
    else
      result.truncate( len );
  }

  if (readLen < len)
  {
    QString msg = i18n("Could only read %1 bytes of %2.",
                   readLen, len);
    msgDialog(msg);
    result.truncate( readLen );
  }

  return result;
}

//-----------------------------------------------------------------------------
KDE_EXPORT
bool kByteArrayToFile( const QByteArray & aBuffer, const QString & aFileName,
                       bool aAskIfExists, bool aBackup, bool aVerbose )
{
  // TODO: use KSaveFile
  QFile file(aFileName);

  //assert(aFileName!=0);
  if(aFileName.isEmpty())
    return false;

  if (file.exists())
  {
    if (aAskIfExists)
    {
      QString str;
      str = i18n("File %1 exists.\nDo you want to replace it?",
		   aFileName);
      const int rc = KMessageBox::warningContinueCancel(0,
	   str, i18n("Save to File"), KGuiItem(i18n("&Replace")));
      if (rc != KMessageBox::Continue) return false;
    }
    if (aBackup)
    {
      // make a backup copy
      // TODO: use KSaveFile::backupFile()
      QString bakName = aFileName;
      bakName += '~';
      QFile::remove(bakName);
      if( !QDir::current().rename(aFileName, bakName) )
      {
	// failed to rename file
	if (!aVerbose) return false;
	const int rc = KMessageBox::warningContinueCancel(0,
	     i18n("Failed to make a backup copy of %1.\nContinue anyway?",
	      aFileName),
             i18n("Save to File"), KStdGuiItem::save() );
	if (rc != KMessageBox::Continue) return false;
      }
    }
  }

  if (!file.open(QIODevice::Unbuffered|QIODevice::WriteOnly|QIODevice::Truncate))
  {
    if (aVerbose) switch(file.error())
    {
    case QFile::WriteError:
      msgDialog(i18n("Could not write to file:\n%1", aFileName));
      break;
    case QFile::OpenError:
      msgDialog(i18n("Could not open file for writing:\n%1",
		 aFileName));
      break;
    default:
      msgDialog(i18n("Error while writing file:\n%1", aFileName));
    }
    return false;
  }

  const int writeLen = file.write( aBuffer.data(), aBuffer.size() );

  if ( writeLen < 0 ) {
    if (aVerbose)
      msgDialog(i18n("Could not write to file:\n%1", aFileName));
    return false;
  }
  else if ( writeLen < aBuffer.size() ) {
    QString msg = i18n("Could only write %1 bytes of %2.",
                    writeLen ,
                    aBuffer.size() );
    if (aVerbose)
      msgDialog(msg);
    return false;
  }

  return true;
}


QString checkAndCorrectPermissionsIfPossible( const QString &toCheck,
  const bool recursive, const bool wantItReadable,
  const bool wantItWritable )
{
  // First we have to find out which type the toCheck is. This can be
  // a directory (follow if recursive) or a file (check permissions).
  // Symlinks are followed as expected.
  QFileInfo fiToCheck(toCheck);
  fiToCheck.setCaching(false);
  QByteArray toCheckEnc = QFile::encodeName( toCheck );
  QString error;
  struct stat statbuffer;

  if ( !fiToCheck.exists() ) {
    error.append( i18n("%1 does not exist",
                   toCheck) + "\n");
  }

  // check the access bit of a folder.
  if ( fiToCheck.isDir() ) {
    if ( stat( toCheckEnc,&statbuffer ) != 0 ) {
      kDebug() << "wantItA: Can't read perms of " << toCheck << endl;
    }
    QDir g( toCheck );
    if ( !g.isReadable() ) {
      if ( chmod( toCheckEnc, statbuffer.st_mode + S_IXUSR ) != 0 ) {
        error.append( i18n("%1 is not accessible and that is "
                           "unchangeable.", toCheck) + "\n");
      } else {
        kDebug() << "Changed access bit for " << toCheck << endl;
      }
    }
  }

  // For each file or folder  we can check if the file is readable
  // and writable, as requested.
  if ( fiToCheck.isFile() || fiToCheck.isDir() ) {

    if ( !fiToCheck.isReadable() && wantItReadable ) {
      // Get the current permissions. No need to do anything with an
      // error, it will het added to errors anyhow, later on.
      if ( stat(toCheckEnc,&statbuffer) != 0 ) {
        kDebug() << "wantItR: Can't read perms of " << toCheck << endl;
      }

      // Lets try changing it.
      if ( chmod( toCheckEnc, statbuffer.st_mode + S_IRUSR ) != 0 ) {
        error.append( i18n("%1 is not readable and that is unchangeable.",
                            toCheck) + "\n");
      } else {
        kDebug() << "Changed the read bit for " << toCheck << endl;
      }
    }

    if ( !fiToCheck.isWritable() && wantItWritable ) {
      // Gets the current persmissions. Needed because it can be changed
      // curing previous operation.
      if (stat(toCheckEnc,&statbuffer) != 0) {
        kDebug() << "wantItW: Can't read perms of " << toCheck << endl;
      }

      // Lets try changing it.
      if ( chmod (toCheckEnc, statbuffer.st_mode + S_IWUSR ) != 0 ) {
        error.append( i18n("%1 is not writable and that is unchangeable.",
                            toCheck) + "\n");
      } else {
        kDebug() << "Changed the write bit for " << toCheck << endl;
      }
    }
  }

  // If it is a folder and recursive is true, then we check the contents of
  // the folder.
  if ( fiToCheck.isDir() && recursive ){
    QDir g(toCheck);
    // First check if the folder is readable for us. If not, we get
    // some ugly crashes.
    if ( !g.isReadable() ){
      error.append(i18n("Folder %1 is inaccessible.", toCheck) + "\n");
    } else {
      foreach(QFileInfo fi, g.entryInfoList())
      {
        QString newToCheck = toCheck + '/' + fi.fileName();
        QFileInfo fiNewToCheck(newToCheck);
        if ( fi.fileName() != "." && fi.fileName() != ".." ) {
          error.append ( checkAndCorrectPermissionsIfPossible( newToCheck,
                                recursive, wantItReadable, wantItWritable) );
        }
      }
    }
  }
  return error;
}

bool checkAndCorrectPermissionsIfPossibleWithErrorHandling( QWidget *parent,
  const QString &toCheck, const bool recursive, const bool wantItReadable,
  const bool wantItWritable )
{
  QString error = checkAndCorrectPermissionsIfPossible(toCheck, recursive,
                                           wantItReadable, wantItWritable);
  // There is no KMessageBox with Retry, Cancel and Details.
  // so, I can't provide a functionality to recheck. So it now
  // it is just a warning.
  if ( !error.isEmpty() ) {
    kDebug() << "checkPermissions found:" << error << endl;
    KMessageBox::detailedSorry(parent,
                               i18n("Some files or folders do not have "
                               "the right permissions, please correct them "
                               "manually."),
                               error, i18n("Permissions Check"), false);
    return false;
  } else {
    return true;
  }
}

}
