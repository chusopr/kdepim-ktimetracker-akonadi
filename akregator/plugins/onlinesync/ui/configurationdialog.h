#ifndef CONFIGURATIONFRAME_H
#define CONFIGURATIONFRAME_H
 
#include <KDialog>
 
// include the automatically generated header file for the ui-file
#include "ui_configurationframe.h"

class ConfigurationDialog : public KDialog
{
    Q_OBJECT
    public:
        ConfigurationDialog( QWidget *parent=0 );
        ~ConfigurationDialog();

    private slots:
        void slotButtonClicked();

    private:
        Ui::ConfigurationFrame ui;
};
 
#endif
