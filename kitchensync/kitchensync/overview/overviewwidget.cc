
#include "overviewwidget.h"

#include <qvariant.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qmovie.h>
#include <qptrlist.h>
#include <qvbox.h>
#include <qhbox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include <manipulatorpart.h>

using namespace KitchenSync;

OverviewWidget::OverviewWidget( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl ) {

    if ( !name )
        setName( "overviewWidget" );
    //setCaption( i18n( "KitchenSync - Overview" ) );

    box = new QVBoxLayout( this );

    QHBox *topBox = new QHBox( this );
    topBox->setSpacing( 200 );
    topBox->setMinimumHeight( 130 );

    QVBox *nameBox = new QVBox( topBox );

    deviceName = new QLabel( nameBox, "deviceName" );
    deviceName->setText( i18n( "<h2>Device</h2>" ) );
    deviceName->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );

    nameField = new QLabel ( nameBox, "namField" );
    nameField->setText( i18n("Name") );
    nameField->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );

    QPixmap logo;
    kdDebug() << "Locate " << locate("appdata", "pics/opie_logo") << endl;
    logo.load(locate ("appdata", "pics/opie_logo.png" ) );

    deviceLogo = new QLabel( topBox, "deviceLogo" );
    //deviceLogo->setGeometry( QRect( 360, 20, 120, 120 ) );
    deviceLogo->setPixmap( logo );
    //deviceLogo->setScaledContents( TRUE );


    Line = new QFrame( this, "Line" );
    //Line->setGeometry( QRect( 0, 140, 611, 20 ) );
    Line->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0, 0, 0,  Line->sizePolicy().hasHeightForWidth() ) );
    Line->setProperty( "frameShape", (int)QFrame::HLine );
    Line->setFrameShadow( QFrame::Sunken );
    Line->setFrameShape( QFrame::HLine );


    //QLabel *aniIcon = new QLabel( this);
    //aniIcon->setMovie(KGlobal::iconLoader()->loadMovie("image", KIcon::NoGroup, 32));

    QLabel *filler = new QLabel(this);

    sv = new QScrollView( this );
    sv->setResizePolicy(QScrollView::AutoOneFit);
    sv->setHScrollBarMode( QScrollView::AlwaysOff );
    sv->setFrameShape(QFrame::NoFrame);
    sv->addChild(filler);
    sv->show();

    box->addWidget( topBox );
    box->addWidget( Line );
    box->addWidget( sv );
    m_svView=0;
}

// this will crash but it's deprecated
void OverviewWidget::showList(QPtrList<ManipulatorPart> list) {

    QVBox* progressLayout = new QVBox( sv->viewport() );

    ManipulatorPart* currentPart;
    for (currentPart = list.first(); currentPart != 0; currentPart = list.next()) {

        QPixmap *image = currentPart->pixmap();
        QString text = currentPart->name();

        NewProgress *test = new NewProgress(*image, text, progressLayout);

        sv->addChild(test);
    }
}
void OverviewWidget::clearProgress(const Profile& prof,  Konnector* con,  const QString& id){
    m_progress.setAutoDelete( true );
    setDeviceName( con->id( id ) );
    setNameField( prof.name() );
    QPixmap pix;
    kdDebug() << con->iconName(id ) << endl;
    kdDebug() << "Clear Progress " << locate("appdata",  con->iconName(id) ) << endl;
    QStringList list = KGlobal::dirs()->findDirs("data", "kitchensync" );
    setLogo( con->iconSet(id).pixmap()   );
    if (m_svView == 0 ) {
        m_svView = new QVBox( sv->viewport() );
        //box->addWidget( m_svView );
        sv->addChild(m_svView);
    }
}
void OverviewWidget::addProgress( ManipulatorPart* part ){
    kdDebug() << "OverviewWidget add Progress" << endl;
    current = new NewProgress( *part->pixmap(), part->name(), sv->viewport() );
    //m_svView->addWidget( current );
    sv->addChild( current );
    m_progress.append( current );
}
void OverviewWidget::currentDone(){
    //current->setStatusLabel( 1 );
}
void OverviewWidget::setDeviceName(const QString &name) {
    deviceName->setText( i18n("<h2> %1 </h2>").arg(name) );
}

void OverviewWidget::setNameField(const QString &name) {
    nameField->setText( i18n("<b> %1 <b>").arg(name) );
}

void OverviewWidget::setLogo(const QPixmap &image0) {
    deviceLogo->setPixmap( image0 );
}


NewProgress::NewProgress( QPixmap &icon,
			  QString text,
			  QWidget* parent,
			  const char* name,
			  WFlags fl) : QWidget(parent, name, fl) {

    progressItemPix = new QLabel( this, "progressItemPix" );
    progressItemPix->setGeometry( QRect ( 20, 0, 20, 20) );
    progressItemPix->setPixmap( icon );

    progressLabel = new QLabel( this, "progressLabel" );
    progressLabel->setGeometry( QRect( 50, 0, 440, 20 ) );
    progressLabel->setText( i18n( "<b> %1 <b>" ).arg(text) );
    progressLabel->setAlignment( int( QLabel::AlignTop | QLabel::AlignLeft ) );

    statusLabel = new QLabel ( this, "statusLabel" );
    statusLabel->setGeometry( QRect ( 450, 0, 20, 20) );
}

void NewProgress::setProgressItemPix(const QPixmap &image) {
    progressItemPix->setPixmap( image );
}

void NewProgress::setProgressLabel(const QString &text) {
    progressLabel->setText( i18n(text) );
}

void NewProgress::setStatusLabel(int status) {

    QPixmap workingIcon( " " );
    QPixmap doneIcon(" ");
    if (status=0) {
        progressItemPix->setPixmap( workingIcon );
    } else {
      progressItemPix->setPixmap( doneIcon );
    }
}

void NewProgress::timerEvent(QTimerEvent *) {
    progressItemPix->setMovie(KGlobal::iconLoader()->loadMovie("image", KIcon::NoGroup, 32));
}
NewProgress::~NewProgress() {
}

OverviewWidget::~OverviewWidget() {
}

#include "overviewwidget.moc"
