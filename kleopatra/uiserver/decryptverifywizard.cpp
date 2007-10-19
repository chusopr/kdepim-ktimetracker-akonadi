#include "decryptverifywizard.h"

#include "decryptverifyoperationwidget.h"
#include "decryptverifyresultwidget.h"

#include <utils/filenamerequester.h>

#include <KLocale>

#include <QScrollArea>
#include <QWizardPage>
#include <QLayout>
#include <QLabel>

#include <vector>
#include <cassert>

using namespace Kleo;

namespace {

    class ScrollArea : public QScrollArea {
        Q_OBJECT
    public:
        explicit ScrollArea( QWidget * p=0 )
            : QScrollArea( p )
        {
            setWidget( new QWidget );
            new QVBoxLayout( widget() );
            setWidgetResizable( true );
        }
        ~ScrollArea() {}
    };

    class OperationsPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit OperationsPage( QWidget * p=0 );
        ~OperationsPage();

        void setOutputDirectory( const QString & dir ) {
            m_ui.outputDirectoryFNR.setFileName( dir );
        }

        QString outputDirectory() const {
            return m_ui.outputDirectoryFNR.fileName();
        }

        void ensureIndexAvailable( unsigned int idx );

        DecryptVerifyOperationWidget * widget( unsigned int idx ) {
            return m_widgets.at( idx );
        }

    private:
        std::vector<DecryptVerifyOperationWidget*> m_widgets;

        struct UI {
            QVBoxLayout     vlay;
            QHBoxLayout      hlay;
            QLabel            outputDirectoryLB;
            FileNameRequester outputDirectoryFNR;
            ScrollArea       scrollArea; // ### replace with KDScrollArea when done

            explicit UI( OperationsPage * q );
        } m_ui;
    };


    class ResultPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit ResultPage( QWidget * p=0 );
        ~ResultPage();

        void ensureIndexAvailable( unsigned int idx );

        DecryptVerifyResultWidget * widget( unsigned int idx ) {
            return m_widgets.at( idx );
        }

    private:
        std::vector<DecryptVerifyResultWidget*> m_widgets;

        struct UI {
            QVBoxLayout vlay;
            ScrollArea   scrollArea; // ### replace with KDScrollArea when done
            
            explicit UI( ResultPage * q );
        } m_ui;
    };
}

class DecryptVerifyWizard::Private {
    friend class ::Kleo::DecryptVerifyWizard;
    DecryptVerifyWizard * const q;
public:
    Private( DecryptVerifyWizard * qq );
    ~Private();

    void ensureIndexAvailable( unsigned int idx ) {
        operationsPage.ensureIndexAvailable( idx );
        resultPage.ensureIndexAvailable( idx );
    }

private:
    OperationsPage operationsPage;
    ResultPage resultPage;
};


DecryptVerifyWizard::DecryptVerifyWizard( QWidget * p, Qt::WindowFlags f )
    : QWizard( p, f ), d( new Private( this ) )
{

}

DecryptVerifyWizard::~DecryptVerifyWizard() {}

void DecryptVerifyWizard::setOutputDirectory( const QString & dir ) {
    d->operationsPage.setOutputDirectory( dir );
}

QString DecryptVerifyWizard::outputDirectory() const {
    return d->operationsPage.outputDirectory();
}

DecryptVerifyOperationWidget * DecryptVerifyWizard::operationWidget( unsigned int idx ) {
    d->ensureIndexAvailable( idx );
    return d->operationsPage.widget( idx );
}

DecryptVerifyResultWidget * DecryptVerifyWizard::resultWidget( unsigned int idx ) {
    d->ensureIndexAvailable( idx );
    return d->resultPage.widget( idx );
}






DecryptVerifyWizard::Private::Private( DecryptVerifyWizard * qq )
    : q( qq ),
      operationsPage( q ),
      resultPage( q )
{
    q->addPage( &operationsPage );
    q->addPage( &resultPage );
}

DecryptVerifyWizard::Private::~Private() {}







OperationsPage::OperationsPage( QWidget * p )
    : QWizardPage( p ), m_widgets(), m_ui( this )
{
    setTitle( i18n("FIXME Choose operations to be performed") );
    setSubTitle( i18n("FIXME Here, you can check and, if needed, override "
                      "the operations Kleopatra detected for the input given.") );
}

OperationsPage::~OperationsPage() {}

OperationsPage::UI::UI( OperationsPage * q )
    : vlay( q ),
      hlay(),
      outputDirectoryLB( i18n("&Output directory:"), q ),
      outputDirectoryFNR( q ),
      scrollArea( q )
{
    KDAB_SET_OBJECT_NAME( vlay );
    KDAB_SET_OBJECT_NAME( hlay );
    KDAB_SET_OBJECT_NAME( outputDirectoryLB );
    KDAB_SET_OBJECT_NAME( outputDirectoryFNR );
    KDAB_SET_OBJECT_NAME( scrollArea );

    outputDirectoryLB.setBuddy( &outputDirectoryFNR );

    vlay.addWidget( &scrollArea );
    vlay.addStretch( 1 );
    vlay.addLayout( &hlay );
    hlay.addWidget( &outputDirectoryLB );
    hlay.addWidget( &outputDirectoryFNR );
}

void OperationsPage::ensureIndexAvailable( unsigned int idx ) {

    if ( idx < m_widgets.size() )
        return;

    assert( m_ui.scrollArea.widget() );
    assert( qobject_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() ) );
    QBoxLayout & blay = *static_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() );

    for ( unsigned int i = m_widgets.size() ; i < idx+1 ; ++i ) {
        DecryptVerifyOperationWidget * w = new DecryptVerifyOperationWidget( m_ui.scrollArea.widget() );
        blay.addWidget( w );
        w->show();
        m_widgets.push_back( w );
    }
}






ResultPage::ResultPage( QWidget * p )
    : QWizardPage( p ), m_widgets(), m_ui( this )
{
    setTitle( i18n("Results") );
}

ResultPage::~ResultPage() {}

ResultPage::UI::UI( ResultPage * q )
    : vlay( q ),
      scrollArea( q )
{
    KDAB_SET_OBJECT_NAME( vlay );
    KDAB_SET_OBJECT_NAME( scrollArea );

    vlay.addWidget( &scrollArea );
    vlay.addStretch( 1 );
}

void ResultPage::ensureIndexAvailable( unsigned int idx ) {

    if ( idx < m_widgets.size() )
        return;

    assert( m_ui.scrollArea.widget() );
    assert( qobject_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() ) );
    QBoxLayout & blay = *static_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() );

    for ( unsigned int i = m_widgets.size() ; i < idx+1 ; ++i ) {
        DecryptVerifyResultWidget * w = new DecryptVerifyResultWidget( m_ui.scrollArea.widget() );
        blay.addWidget( w );
        w->show();
        m_widgets.push_back( w );
    }
    
}





#include "decryptverifywizard.moc"
#include "moc_decryptverifywizard.cpp"
