/* 
karm.cpp (c) 2006 by Thorsten Staerk

This program is here to remind the user that karm is now called ktimetracker. 
We cannot be sure the user is on a graphical display when starting this, 
so print out and display a messagebox.
We cannot be sure the user is aware the system wants to start karm, 
so say "someone, probably you, has called karm...".

Compile it like this:
g++ -I/home/kde-devel/qt-unstable/include/Qt -I/home/kde-devel/qt-unstable/include -I/home/kde-devel/qt-unstable/include/QtCore -I/home/kde-devel/qt-unstable/include/QtGui -I/home/kde-devel/kde/include -lQtCore_debug -L/home/kde-devel/qt-unstable/lib -L/home/kde-devel/kde/lib -lkdeui karm.cpp -o karm        
*/          

#include <QString>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <iostream>

using namespace std;

int main (int argc, char *argv[])
{
  KAboutData aboutData( "KArmReminder", "KArmReminder",
      "1.0", "KArmReminder", KAboutData::License_GPL,
      "(c) 2006" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication khello;
  cout << "Someone, probably you, has called karm.\n";
  cout << "KArm is now called KTimeTracker. This makes it easier to recognize.\n";
  cout << "With KDE 4, this program reminds you to call ktimetracker.\n";
  cout << "With KDE 5, this reminder will be gone.\n";
  KMessageBox::information(0,"Someone, probably you, has called karm. KArm is now called KTimeTracker. This makes it easier to recognize. With KDE 4, this program reminds you to call ktimetracker. With KDE 5, this reminder will have gone.","KArm is now ktimetracker");
}
