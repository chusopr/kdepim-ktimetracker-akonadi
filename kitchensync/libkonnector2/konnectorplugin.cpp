
#include "konnectorinfo.h"
#include "konnectorplugin.h"

using namespace KSync;

Konnector::Konnector( QObject *obj, const char *name, const QStringList & )
    : QObject( obj, name )
{
}

Konnector::~Konnector()
{
}

void Konnector::setUDI( const QString& udi )
{
    m_udi = udi;
}

QString Konnector::udi() const
{
    return m_udi;
}

void Konnector::add( const QString& res )
{
    m_adds << res;
}

void Konnector::remove( const QString& res )
{
    m_adds.remove( res );
}

QStringList Konnector::resources() const
{
    return m_adds;
}

bool Konnector::isConnected() const
{
    return  info().isConnected();
}

void Konnector::progress( const Progress& prog )
{
    emit sig_progress( m_udi, prog );
}

void Konnector::error( const Error& err )
{
    emit sig_error( m_udi, err );
}

ConfigWidget *Konnector::configWidget( const Kapabilities&, QWidget*, const char* )
{
    return 0;
}

ConfigWidget *Konnector::configWidget( QWidget*, const char* )
{
    return 0;
}

QStringList Konnector::builtIn() const
{
    return QStringList();
}

void Konnector::doWrite( Syncee::PtrList list )
{
    write( list );
}

#include "konnectorplugin.moc"
