/*
* shell.h -- Declaration of class KornShell.
* Generated by newclass on Sun May  3 10:30:24 EST 1998.
*/
#ifndef SSK_SHELL_H
#define SSK_SHELL_H

#include<qptrlist.h>
#include<qwidget.h>
#include"kornset.h"
#include"kornbutt.h"


class QPopupMenu;
class QCloseEvent;
class QBoxLayout;

class KDropManager;
class KornOptDlg;
class KornButton;
class KAction;
class KornSubjectsDlg;


class KDialogBase;


/**
* Korn Top-level widget.
* @author Sirtaj Singh Kang (taj@kde.org)
* @version $Id$
*/
class KornShell : public QWidget
{
	Q_OBJECT
public:
	/**
	* KornShell Constructor
	*/
	KornShell( QWidget *parent = 0 );

	/**
	* KornShell Destructor
	*/
	virtual ~KornShell();

	/**
	 * Load app and monitor configurations and initialize application.
	 * @return false if a valid configuration was not found.
	 */
	bool init();

	/**
	 * Called from KornButton if the right mouse button was clicked.
	 * Opens the right mouse click menu, which depends on the KornButton
	 * clicked on.
	 * @param popup the calling KornButton instance.
	 */
	void popup(KornButton *button);

public slots:
        void popupMenu();
        void optionDlg();

	/**
	 * Called if the menu item "Re-Check" was chosen.
	 */
        void reCheck();

	/**
	 * Called if the menu item "Read Subjects" was chosen.
	 */
        void readSubjects();
        void help();
	void reportBug();
        void about();

	void dlgClosed();

	virtual void show();

	void saveSession();

private slots:
	/**
	* Deletes the button and removes it from the handled button
	* list if required.
	*/
	void disconnectButton( KornButton *button );

	void configDirty();

private:
	KornShell& operator=( KornShell& );
	KornShell( const KornShell& );

	bool		_configDirty;
	bool		_toWrite;

	QPtrList<KornButton> *_buttons;
	KornSettings	*_settings;
	KDropManager	*_manager;
	QPopupMenu	*_menu;
	QBoxLayout	*_layout;
	KAction		*_checkMailAction;
	KMailDrop	*_currentMailDrop;
	KornSubjectsDlg	*_subjectsDlg;
#if defined(test_headerbutton)
  HeadButton *_headbutton;
#endif
	/** 
	 * Creates and initializes the right-click popup menu.
	 */
	QPopupMenu	*initMenu();

	//KornOptDlg	*_optDlg;
	KDialogBase	*_optDlg;

	/**
	* Performs hand-holding for new users. This is called
	* if a valid set of maildrops is not found.
	*
	* @return true if a default configuration has been
	*		created for the user, false otherwise.
	*/
	bool firstTimeInit();

	/**
	* Creates the buttons and lays them out with the
	* specified layout policy. All old buttons and layouts
	* are deleted.
	*/
	void createButtons( KornSettings::Layout layout );
};

#endif // SSK_SHELL_H
