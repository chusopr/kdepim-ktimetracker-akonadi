#ifndef KARM_MAIN_WINDOW_H
#define KARM_MAIN_WINDOW_H

#include <kmainwindow.h>

class QListViewItem;
class QPoint;

class KAccel;
class KDialogBase;

class KAccelMenuWatch;
class KarmTray;
class Preferences;
class TaskView;

/**
 * Main window to tie the application together.
 */

class MainWindow : public KMainWindow
{
  Q_OBJECT

  private:
    KAccel          *_accel;
    KAccelMenuWatch *_watcher;
    TaskView        *_taskView;
    long            _totalSum;
    long            _sessionSum;
    Preferences     *_preferences;
    bool            _hideOnClose;
    KarmTray        *_tray;

  public:
    MainWindow();
    virtual ~MainWindow();

  protected slots:
    void keyBindings();
    void startNewSession();
    void resetAllTimes();
    void updateTime( long, long );
    void updateStatusBar();
    void save();
    void quit();
    void print();
    void slotSelectionChanged();
    void hideOnClose( bool hide );
    void contextMenuRequest( QListViewItem*, const QPoint&, int );
    void enableStopAll();
    void disableStopAll();
    void timeLoggingChanged( bool on );

  protected:
    void startStatusBar();
    virtual void saveProperties( KConfig* );
    void saveGeometry();
    void loadGeometry();
    bool queryClose();

  private:
    void makeMenus();

    KDialogBase *dialog;
    KAction* actionStart;
    KAction* actionStop;
    KAction* actionStopAll;
    KAction* actionDelete;
    KAction* actionEdit;
    KAction* actionAddComment;
    KAction* actionPreferences;

    friend class KarmTray;
};

#endif // KARM_MAIN_WINDOW_H
