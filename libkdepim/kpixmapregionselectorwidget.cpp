#include "kpixmapregionselectorwidget.h"
#include <qpainter.h>
#include <qcolor.h>
#include <qimage.h>
#include <qlayout.h>
#include <kimageeffect.h>
#include <kdebug.h>
#include <stdlib.h>

using namespace KPIM;

KPixmapRegionSelectorWidget::KPixmapRegionSelectorWidget( QWidget *parent, 
      const char *name) : QWidget( parent, name)
{
   QHBoxLayout * hboxLayout=new QHBoxLayout( this );
   
   hboxLayout->addStretch();
   QVBoxLayout * vboxLayout=new QVBoxLayout( this );

   vboxLayout->addStretch();
   m_label = new QLabel(this, "pixmapHolder");
   m_label->setBackgroundMode( Qt::NoBackground );
   m_label->installEventFilter( this );

   vboxLayout->addWidget(m_label);
   vboxLayout->addStretch();

   hboxLayout->addLayout(vboxLayout);
   hboxLayout->addStretch();

   m_forcedAspectRatio=0;

   m_zoomFactor=1.0;
}

KPixmapRegionSelectorWidget::~KPixmapRegionSelectorWidget()
{
}

void KPixmapRegionSelectorWidget::setPixmap( const QPixmap &pixmap )
{
   m_originalPixmap = pixmap;
   m_unzoomedPixmap = pixmap;
   m_label->setPixmap( pixmap );
   resetSelection();
}

void KPixmapRegionSelectorWidget::resetSelection()
{
   m_selectedRegion = m_originalPixmap.rect();
   updatePixmap();
}

QRect KPixmapRegionSelectorWidget::selectedRegion() const
{
     return m_selectedRegion;
}

void KPixmapRegionSelectorWidget::setSelectedRegion(const QRect &rect)
{
   if (!rect.isValid()) resetSelection();
   else 
   {
      m_selectedRegion=rect;
      updatePixmap();

      QRect r=unzoomedSelectedRegion();
   }
}

void KPixmapRegionSelectorWidget::updatePixmap()
{
   if (m_selectedRegion.width()>m_originalPixmap.width()) m_selectedRegion.setWidth( m_originalPixmap.width() );
   if (m_selectedRegion.height()>m_originalPixmap.height()) m_selectedRegion.setHeight( m_originalPixmap.height() );

   QPainter painter;
   if (m_linedPixmap.isNull())
   {
     m_linedPixmap = m_originalPixmap;

     painter.begin(&m_linedPixmap);
     painter.setRasterOp( Qt::XorROP );
     painter.fillRect(0,0,m_linedPixmap.width(), m_linedPixmap.height(), 
                  QBrush( QColor(255,255,255), Qt::BDiagPattern) );
     painter.end();

     QImage image=m_linedPixmap.convertToImage();
     image=KImageEffect::fade(image, 0.4, QColor(0,0,0));
     m_linedPixmap.convertFromImage(image);
   } 

   QPixmap pixmap = m_linedPixmap;

   painter.begin(&pixmap);
   painter.drawPixmap( m_selectedRegion.topLeft(), 
        m_originalPixmap, m_selectedRegion );

   painter.setPen( QColor(255,255,255) );
   painter.setRasterOp( Qt::XorROP );

   painter.drawRect( m_selectedRegion );

   painter.end();

   m_label->setPixmap(pixmap);
}

bool KPixmapRegionSelectorWidget::eventFilter(QObject *obj, QEvent *ev)
{
   if ( ev->type() == QEvent::MouseButtonPress )
   {
      QMouseEvent *mev= (QMouseEvent *)(ev);
      //kdDebug() << QString("click at  %1,%2").arg( mev->x() ).arg( mev->y() ) << endl;

      if ( m_selectedRegion.contains( mev->pos() ) 
          && m_selectedRegion!=m_originalPixmap.rect() )
         m_state=Moving;
      else
         m_state=Resizing;

      m_tempFirstClick=mev->pos();

      return TRUE;
   }

   if ( ev->type() == QEvent::MouseMove )
   {
      QMouseEvent *mev= (QMouseEvent *)(ev);

      //kdDebug() << QString("move to  %1,%2").arg( mev->x() ).arg( mev->y() ) << endl;
      
      if ( m_state == Resizing )
      {
         setSelectedRegion ( 
              calcSelectionRectangle( m_tempFirstClick, mev->pos() ) );
      }
      else if (m_state == Moving )
      {
         int mevx = mev->x();
         int mevy = mev->y();
         bool mouseOutside=false;
         if ( mevx < 0 ) 
         {
           m_selectedRegion.moveBy(-m_selectedRegion.x(),0);
           mouseOutside=true;
         }
         else if ( mevx > m_originalPixmap.width() )
         {
           m_selectedRegion.moveBy(m_originalPixmap.width()-m_selectedRegion.width()-m_selectedRegion.x(),0);
           mouseOutside=true;
         }
         if ( mevy < 0 ) 
         {
           m_selectedRegion.moveBy(0,-m_selectedRegion.y());
           mouseOutside=true;
         }
         else if ( mevy > m_originalPixmap.height() )
         {
           m_selectedRegion.moveBy(0,m_originalPixmap.height()-m_selectedRegion.height()-m_selectedRegion.y());
           mouseOutside=true;
         }
         if (mouseOutside) { updatePixmap(); return TRUE; };

         m_selectedRegion.moveBy( mev->x()-m_tempFirstClick.x(),
                                  mev->y()-m_tempFirstClick.y() );

         // Check that the region has not fallen outside the image
         if (m_selectedRegion.x() < 0)
            m_selectedRegion.moveBy(-m_selectedRegion.x(),0);
         else if (m_selectedRegion.right() > m_originalPixmap.width())
            m_selectedRegion.moveBy(-(m_selectedRegion.right()-m_originalPixmap.width()),0); 

         if (m_selectedRegion.y() < 0) 
            m_selectedRegion.moveBy(0,-m_selectedRegion.y());
         else if (m_selectedRegion.bottom() > m_originalPixmap.height()) 
            m_selectedRegion.moveBy(0,-(m_selectedRegion.bottom()-m_originalPixmap.height()));

         m_tempFirstClick=mev->pos();
         updatePixmap();
      } 
      return TRUE;
   }

   if ( ev->type() == QEvent::MouseButtonRelease )
   {
      QMouseEvent *mev= (QMouseEvent *)(ev);

      if ( m_state == Resizing && mev->pos() == m_tempFirstClick)
         resetSelection();

      m_state=None;

      return TRUE;
   }

   QWidget::eventFilter(obj, ev);
   return FALSE;
}

QRect KPixmapRegionSelectorWidget::calcSelectionRectangle( const QPoint & startPoint, const QPoint & _endPoint )
{
   QPoint endPoint = _endPoint;
   if ( endPoint.x() < 0 ) endPoint.setX(0);
   else if ( endPoint.x() > m_originalPixmap.width() ) endPoint.setX(m_originalPixmap.width());
   if ( endPoint.y() < 0 ) endPoint.setY(0);
   else if ( endPoint.y() > m_originalPixmap.height() ) endPoint.setY(m_originalPixmap.height());
   int w=abs(startPoint.x()-endPoint.x());
   int h=abs(startPoint.y()-endPoint.y());

   if (m_forcedAspectRatio>0)
   {
      double aspectRatio=w/double(h);

      if (aspectRatio>m_forcedAspectRatio) 
         h=(int)(w/m_forcedAspectRatio);
      else
         w=(int)(h*m_forcedAspectRatio);
   }

   int x,y;
   if ( startPoint.x() < endPoint.x() )
     x=startPoint.x();
   else
     x=startPoint.x()-w;
   if ( startPoint.y() < endPoint.y() )
     y=startPoint.y();
   else
     y=startPoint.y()-h;

   if (x<0)
   {
      w+=x;
      x=0;
      h=(int)(w/m_forcedAspectRatio);

      if ( startPoint.y() > endPoint.y() )
        y=startPoint.y()-h;
   }
   else if (x+w>m_originalPixmap.width())
   {
      w=m_originalPixmap.width()-x;
      h=(int)(w/m_forcedAspectRatio);

      if ( startPoint.y() > endPoint.y() )
        y=startPoint.y()-h;
   }
   if (y<0)
   {
      h+=y;
      y=0;
      w=(int)(h*m_forcedAspectRatio);

      if ( startPoint.x() > endPoint.x() )
        x=startPoint.x()-w;
   }
   else if (y+h>m_originalPixmap.height())
   {
      h=m_originalPixmap.height()-y;
      w=(int)(h*m_forcedAspectRatio);

      if ( startPoint.x() > endPoint.x() )
        x=startPoint.x()-w;
   }

   return QRect(x,y,w,h);
}

QRect KPixmapRegionSelectorWidget::unzoomedSelectedRegion() const
{
  return QRect((int)(m_selectedRegion.x()/m_zoomFactor),
               (int)(m_selectedRegion.y()/m_zoomFactor),
               (int)(m_selectedRegion.width()/m_zoomFactor),
               (int)(m_selectedRegion.height()/m_zoomFactor));
}

QImage KPixmapRegionSelectorWidget::selectedImage() const
{
   QImage origImage=m_unzoomedPixmap.convertToImage();
   return origImage.copy(unzoomedSelectedRegion());
}

void KPixmapRegionSelectorWidget::setSelectionAspectRatio(int width, int height)
{
   m_forcedAspectRatio=width/double(height);
}

void KPixmapRegionSelectorWidget::setFreeSelectionAspectRatio()
{
   m_forcedAspectRatio=0;
}

void KPixmapRegionSelectorWidget::setMaximumWidgetSize(int width, int height)
{
   m_maxWidth=width;
   m_maxHeight=height;

   m_originalPixmap=m_unzoomedPixmap;
   if (m_selectedRegion == m_originalPixmap.rect()) m_selectedRegion=QRect();

//   kdDebug() << QString(" original Pixmap :") << m_originalPixmap.rect() << endl;
//   kdDebug() << QString(" unzoomed Pixmap : %1 x %2 ").arg(m_unzoomedPixmap.width()).arg(m_unzoomedPixmap.height()) << endl;

   if ( !m_originalPixmap.isNull() &&
       ( m_originalPixmap.width() > m_maxWidth || 
         m_originalPixmap.height() > m_maxHeight ) )
   {
         /* We have to resize the pixmap to get it complete on the screen */
         QImage image=m_originalPixmap.convertToImage();
         m_originalPixmap.convertFromImage( image.smoothScale( width, height, QImage::ScaleMin ) );
         //m_originalPixmap.convertFromImage( KImageEffect::sample( image, width, height ) );
         double oldZoomFactor = m_zoomFactor;
         m_zoomFactor=m_originalPixmap.width()/(double)m_unzoomedPixmap.width();

         if (m_selectedRegion.isValid())
         {
            m_selectedRegion=
                  QRect((int)(m_selectedRegion.x()*m_zoomFactor/oldZoomFactor),
                        (int)(m_selectedRegion.y()*m_zoomFactor/oldZoomFactor),
                        (int)(m_selectedRegion.width()*m_zoomFactor/oldZoomFactor),
                        (int)(m_selectedRegion.height()*m_zoomFactor/oldZoomFactor) );
         }
   }
   
   if (!m_selectedRegion.isValid()) m_selectedRegion = m_originalPixmap.rect();

   m_linedPixmap=QPixmap();
   updatePixmap();
   resize(m_label->width(), m_label->height());
}

