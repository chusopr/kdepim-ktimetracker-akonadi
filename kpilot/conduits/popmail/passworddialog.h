// passworddialog.h
//
// (c) 1997 by Michael Roth
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 

// $Revision$

// Taken from KPasswd

#ifndef MRQPASSWD_H
#define MRQPASSWD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdialog.h>
#include <qlined.h>

class PasswordDialog : public QDialog
{
    Q_OBJECT
   
public:
    PasswordDialog( const char *head, QWidget* parent=0, const char* name=0, bool modal=false, WFlags f=0 );
    
    const char *password();			// Gibt das Paswort zurueck
    
private:
    const char *_head;
    QLineEdit *_w_password;      
};


#endif


