#ifndef __KMAIL_PLUGIN_H__
#define __KMAIL_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"
//#include "kmailpartIface_stub.h"


class KMailPlugin : public Kontact::Plugin
{
  Q_OBJECT

public:

  KMailPlugin(Kontact::Core *core, const char *name, const QStringList & /*args*/);
  ~KMailPlugin();


private slots:

  void slotShowPart();

private:
  void loadPart();
//  KMailPartIface_stub *m_stub;
  KParts::ReadOnlyPart *m_part;
};


#endif
