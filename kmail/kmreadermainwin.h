// -*- mode: C++; c-file-style: "gnu" -*-

#ifndef KMReaderMainWin_h
#define KMReaderMainWin_h

#include "secondarywindow.h"

#include <kurl.h>

#include <boost/scoped_ptr.hpp>
#include "kmail-akonadi.h"
class KMReaderWin;
class KAction;
class KActionMenu;
class KFontAction;
class KFontSizeAction;
class CustomTemplatesMenu;
template <typename T, typename S> class QMap;

namespace KMail {
class MessageActions;
}

namespace KMime {
  class Message;
  class Content;
}
namespace Akonadi {
  class Item;
}

class KMReaderMainWin : public KMail::SecondaryWindow
{
  Q_OBJECT

public:
  KMReaderMainWin( bool htmlOverride, bool htmlLoadExtOverride, char *name = 0 );
  KMReaderMainWin( char *name = 0 );
  KMReaderMainWin(KMime::Content* aMsgPart,
    bool aHTML, const QString& aFileName, const QString& pname,
    const QString & encoding, char *name = 0 );
  virtual ~KMReaderMainWin();

  void setUseFixedFont( bool useFixedFont );

  /**
   * take ownership of and show @param msg
   *
   * The last two parameters, serNumOfOriginalMessage and nodeIdOffset, are needed when @p msg
   * is derived from another message, e.g. the user views an encapsulated message in this window.
   * Then, the reader needs to know about that original message, so those to parameters are passed
   * onto setOriginalMsg() of KMReaderWin.
   */
  void showMsg( const QString & encoding, KMime::Message *msg,
                unsigned long serNumOfOriginalMessage = 0, int nodeIdOffset = -1 );
#ifdef USE_AKONADI_VIEWER
  void showMessage( const QString & encoding, const Akonadi::Item &msg );
#endif
private slots:
    void slotMessagePopup(KMime::Message& ,const KUrl&,const QPoint& );
  void slotTrashMsg();
  void slotPrintMsg();
  void slotForwardInlineMsg();
  void slotForwardAttachedMsg();
  void slotRedirectMsg();
  void slotFontAction(const QString &);
  void slotSizeAction(int);
  void slotCreateTodo();
  void slotCustomReplyToMsg( const QString &tmpl );
  void slotCustomReplyAllToMsg( const QString &tmpl );
  void slotCustomForwardMsg( const QString &tmpl );

  void slotEditToolbars();
  void slotConfigChanged();
  void slotUpdateToolbars();

private:
  void initKMReaderMainWin();
  void setupAccel();
  KAction *copyActionMenu();
  void updateCustomTemplateMenus();

  KMReaderWin *mReaderWin;
  KMime::Message *mMsg;
  KUrl mUrl;
  // a few actions duplicated from kmmainwidget
  KAction *mTrashAction, *mPrintAction, *mSaveAsAction, *mSaveAtmAction,
          *mViewSourceAction;
  KFontAction *fontAction;
  KFontSizeAction *fontSizeAction;
  KMail::MessageActions *mMsgActions;

  // Custom template actions menu
  boost::scoped_ptr<CustomTemplatesMenu> mCustomTemplateMenus;
};

#endif /*KMReaderMainWin_h*/
