/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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

#include <kabc/picture.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kimageio.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>

#include "imagewidget.h"

ImageWidget::ImageWidget( const QString &title, bool readOnly,
                          QWidget *parent, const char *name )
  : QWidget( parent, name ), mReadOnly( readOnly )
{
  QHBoxLayout *topLayout = new QHBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );
  QGroupBox *box = new QGroupBox( 0, Qt::Vertical, title, this );
  QGridLayout *boxLayout = new QGridLayout( box->layout(), 3, 2,
                                            KDialog::spacingHint() );
  boxLayout->setRowStretch( 2, 1 );

  mImageLabel = new QLabel( i18n( "Picture" ), box );
  mImageLabel->setFixedSize( 50, 70 );
  mImageLabel->setScaledContents( true );
  mImageLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  boxLayout->addMultiCellWidget( mImageLabel, 0, 2, 0, 0, AlignTop );

  mImageUrl = new KURLRequester( box );
  mImageUrl->setFilter( KImageIO::pattern() );
  mImageUrl->setEnabled( !mReadOnly );
  boxLayout->addWidget( mImageUrl, 0, 1 );

  mUseImageUrl = new QCheckBox( i18n( "Store as URL" ), box );
  mUseImageUrl->setEnabled( false );
  boxLayout->addWidget( mUseImageUrl, 1, 1 );

  topLayout->addWidget( box );

  connect( mImageUrl, SIGNAL( textChanged( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mImageUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( loadImage() ) );
  connect( mImageUrl, SIGNAL( urlSelected( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mImageUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( updateGUI() ) );
  connect( mUseImageUrl, SIGNAL( toggled( bool ) ),
           SIGNAL( changed() ) );
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::setImage( const KABC::Picture &photo )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  if ( photo.isIntern() ) {
    mImageLabel->setPixmap( photo.data() );
    mUseImageUrl->setChecked( false );
  } else {
    mImageUrl->setURL( photo.url() );
    if ( !photo.url().isEmpty() )
      mUseImageUrl->setChecked( true );
    loadImage();
  }

  blockSignals( blocked );
}

KABC::Picture ImageWidget::image() const
{
  KABC::Picture photo;

  if ( mUseImageUrl->isChecked() )
    photo.setUrl( mImageUrl->url() );
  else {
    QPixmap *px = mImageLabel->pixmap();
    if ( px ) {
      if ( px->height() > px->width() )
        photo.setData( px->convertToImage().scaleHeight( 140 ) );
      else
        photo.setData( px->convertToImage().scaleWidth( 100 ) );

      photo.setType( "PNG" );
    }
  }

  return photo;
}

void ImageWidget::loadImage()
{
  mImageLabel->setPixmap( loadPixmap( mImageUrl->url() ) );
}

void ImageWidget::updateGUI()
{
  if ( !mReadOnly )
    mUseImageUrl->setEnabled( true );
}

QPixmap ImageWidget::loadPixmap( const KURL &url )
{
  QString tempFile;
  QPixmap pixmap;

  if ( url.isEmpty() )
    return pixmap;

  if ( url.isLocalFile() )
    pixmap = QPixmap( url.path() );
  else if ( KIO::NetAccess::download( url, tempFile, this ) ) {
    pixmap = QPixmap( tempFile );
    KIO::NetAccess::removeTempFile( tempFile );
  }

  return pixmap;
}

#include "imagewidget.moc"
