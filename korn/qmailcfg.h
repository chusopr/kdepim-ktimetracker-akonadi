/*
* qmailcfg.h -- Declaration of class KQMailCfg.
* Generated by newclass on Mon Aug  3 13:21:18 EST 1998.
*/
#ifndef SSK_QMAILCFG_H
#define SSK_QMAILCFG_H

#include "moncfg.h"

class KQMailDrop;
class KURLRequester;
/**
* Configuration Dialog for @ref KQMailDrop monitors.
* @author Sirtaj Singh Kang (taj@kde.org)
* @version $Id$
*/
class KQMailCfg : public KMonitorCfg
{
public:
	/**
	* KQMailCfg Constructor
	*/
	KQMailCfg( KQMailDrop * );

	/**
	* KQMailCfg Destructor
	*/
	virtual ~KQMailCfg() {}
	
        virtual QString name() const;
        virtual QWidget *makeWidget( QWidget *parent );
        virtual void updateConfig();

private:
	KQMailCfg& operator=( KQMailCfg& );
	KQMailCfg( const KQMailCfg& );

	KURLRequester *_pathEdit;
};

#endif // SSK_QMAILCFG_H
