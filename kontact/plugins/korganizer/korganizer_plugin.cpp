
#include <qwidget.h>

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include "core.h"

#include "korganizer_plugin.h"

typedef KGenericFactory< KOrganizerPlugin, Kontact::Core > KOrganizerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkorganizerplugin,
                            KOrganizerPluginFactory( "kporganizerplugin" ) );

KOrganizerPlugin::KOrganizerPlugin( Kontact::Core *_core, const char *name, const QStringList& )
  : Kontact::Plugin( _core, _core, name ), 
    m_part( 0 ), m_iface( 0 )
{
  setInstance( KOrganizerPluginFactory::instance() );

  setXMLFile( "kpkorganizerplugin.rc" );

  insertNewAction( new KAction( i18n( "New Event" ), BarIcon( "event" ),
                   0, this, SLOT( slotNewEvent() ), actionCollection(),
                   "new_event" ) );

  insertNewAction( new KAction( i18n( "New Todo" ), BarIcon( "todo" ),
                   0, this, SLOT( slotNewTodo() ), actionCollection(),
                   "new_todo" ) );
}

KOrganizerPlugin::~KOrganizerPlugin()
{
}

KParts::ReadOnlyPart* KOrganizerPlugin::part()
{
  if ( !m_part ) {
    (void) dcopClient(); // ensure that we register to DCOP as "korganizer"
    m_iface = new KOrganizerIface_stub( dcopClient(), "korganizer", "KOrganizerIface" );

    m_part = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkorganizer",
                                                             0, 0, // parentwidget,name
                                                             this, 0 ); // parent,name
  }

  return m_part;
}

void KOrganizerPlugin::slotNewEvent()
{
  part();
  if ( !m_iface )
    return;
}

void KOrganizerPlugin::slotNewTodo()
{
  part();
  if ( !m_iface )
    return;
}

bool KOrganizerPlugin::createDCOPInterface( const QString& serviceType )
{
  kdDebug(5602) << k_funcinfo << serviceType << endl;
  if ( serviceType == "DCOP/Organizer" || serviceType == "DCOP/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

#include "korganizer_plugin.moc"
