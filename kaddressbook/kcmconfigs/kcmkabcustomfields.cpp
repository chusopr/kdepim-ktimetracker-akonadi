/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qwidgetfactory.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klistview.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>

#include "kabprefs.h"

#include "kcmkabcustomfields.h"

extern "C"
{
  KCModule *create_kabcustomfields( QWidget *parent, const char * ) {
    return new KCMKabCustomFields( parent, "kcmkabcustomfields" );
  }
}

class PageItem : public QCheckListItem
{
  public:
    PageItem( QListView *parent, const QString &path )
      : QCheckListItem( parent, "", QCheckListItem::CheckBox ),
        mPath( path ), mIsActive( false )
    {
      mName = path.mid( path.findRev( '/' ) + 1 );
    }


    virtual QString text( int column ) const
    {
      if ( column == 0 )
        return mName;
      else
        return QString::null;
    }

    QString name() const { return mName; }
    QString path() const { return mPath; }

    QPixmap preview()
    {
      if ( mPreview.isNull() ) {
        QWidget *wdg = QWidgetFactory::create( mPath, 0, 0 );
        if ( wdg ) {
          QPixmap pm = QPixmap::grabWidget( wdg );
          QImage img = pm.convertToImage().smoothScale( 300, 300, QImage::ScaleMin );
          mPreview = img;
        } else
          delete wdg;
      }

      return mPreview;
    }

    void setIsActive( bool isActive ) { mIsActive = isActive; }
    bool isActive() const { return mIsActive; }

  private:
    QString mName;
    QString mPath;
    QPixmap mPreview;
    bool mIsActive;
};

KCMKabCustomFields::KCMKabCustomFields( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  connect( mPageView, SIGNAL( selectionChanged( QListViewItem* ) ),
           this, SLOT( updatePreview( QListViewItem* ) ) );
  connect( mPageView, SIGNAL( clicked( QListViewItem* ) ),
           this, SLOT( itemClicked( QListViewItem* ) ) );

  connect( mDesignerButton, SIGNAL( clicked() ),
           this, SLOT( startDesigner() ) );

  load();
}

void KCMKabCustomFields::load()
{
  QStringList activePages = KABPrefs::instance()->mAdvancedCustomFields;

  QListViewItemIterator it( mPageView );
  while ( it.current() ) {
    PageItem *item = static_cast<PageItem*>( it.current() );
    if ( activePages.find( item->name() ) != activePages.end() ) {
      item->setOn( true );
      item->setIsActive( true );
    }

    ++it;
  }
}

void KCMKabCustomFields::save()
{
  QListViewItemIterator it( mPageView, QListViewItemIterator::Checked );

  QStringList activePages;
  while ( it.current() ) {
    PageItem *item = static_cast<PageItem*>( it.current() );
    activePages.append( item->name() );

    ++it;
  }

  KABPrefs::instance()->mAdvancedCustomFields = activePages;

  KABPrefs::instance()->writeConfig();
}

void KCMKabCustomFields::defaults()
{
}

void KCMKabCustomFields::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  QHBoxLayout *hbox = new QHBoxLayout( layout, KDialog::spacingHint() );

  mPageView = new KListView( this );
  mPageView->addColumn( i18n( "UI Files" ) );
  mPageView->setAllColumnsShowFocus( true );
  mPageView->setFullWidth( true );
  hbox->addWidget( mPageView );

  mPagePreview = new QLabel( this );
  mPagePreview->setFrameStyle( QFrame::Box | QFrame::Sunken );
  mPagePreview->setMinimumWidth( 300 );
  hbox->addWidget( mPagePreview );

  QStringList list = KGlobal::dirs()->findAllResources( "data", "kaddressbook/contacteditorpages/*.ui", true, true );
  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    new PageItem( mPageView, *it );
  }

  hbox = new QHBoxLayout( layout, KDialog::spacingHint() );
  hbox->addStretch( 1 );

  mDesignerButton = new QPushButton( i18n( "Edit with Qt Designer..." ), this );
  hbox->addWidget( mDesignerButton );
}

void KCMKabCustomFields::updatePreview( QListViewItem *item )
{
  if ( item ) {
    PageItem *pageItem = static_cast<PageItem*>( item );
    mPagePreview->setPixmap( pageItem->preview() );
	} else {
    mPagePreview->setPixmap( QPixmap() );
  }
}

void KCMKabCustomFields::itemClicked( QListViewItem *item )
{
  if ( !item )
    return;

  PageItem *pageItem = static_cast<PageItem*>( item );

  if ( pageItem->isOn() != pageItem->isActive() ) {
    emit changed( true );
    pageItem->setIsActive( pageItem->isOn() );
  }
}

void KCMKabCustomFields::startDesigner()
{
  QString cmdLine = "designer";

  QListViewItem *item = mPageView->selectedItem();
  if ( item ) {
    PageItem *pageItem = static_cast<PageItem*>( item );
    cmdLine += " " + pageItem->path();
  }

  KRun::runCommand( cmdLine );
}

const KAboutData* KCMKabCustomFields::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkabcustomfields" ),
                                      I18N_NOOP( "KAddressBook Custom Fields Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}

#include "kcmkabcustomfields.moc"
