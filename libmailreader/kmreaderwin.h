/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KMREADERWIN_H
#define KMREADERWIN_H

#include "mailviewer_export.h"

#include <QWidget>
#include <QTimer>
#include <QStringList>
#include <QCloseEvent>
#include <QEvent>
#include <QList>
#include <QMap>
#include <QResizeEvent>
#include <kurl.h>
#include <kservice.h>
#include "libkdepim/messagestatus.h"
using KPIM::MessageStatus;
#include <kvbox.h>

#include <kmime/kmime_message.h>

//Akonadi includes
#include <akonadi/item.h>

//TODO(Andras) Just a note so I won't forget: use MailViewer as namespace and library name instead of KMail/MailReader before moving back to trunk

class QSplitter;
class KHBox;
class QTreeWidgetItem;
class QString;
class QTextCodec;
class QTreeView;
class QModelIndex;

class KActionCollection;
class KAction;
class KSelectAction;
class KToggleAction;
class KToggleAction;
class KHTMLPart;
class KUrl;
class KConfigSkeleton;

namespace MailViewer {
  class MimeTreeModel;
  class ConfigureWidget;
  class ObjectTreeParser;
  class AttachmentStrategy;
  class EditorWatcher;

  class HeaderStrategy;
  class HeaderStyle;
  class HtmlWriter;
  class KHtmlPartHtmlWriter;
  class HtmlStatusBar;
  class CSSHelper;
  namespace Interface {
    class BodyPartMemento;
  }
}

namespace {
  class AttachmentURLHandler;
  class FallBackURLHandler;
  class HtmlAnchorHandler;
}

namespace KParts {
  struct BrowserArguments;
  class OpenUrlArguments;
}

/**
   This class implements a "reader window", that is a window
   used for reading or viewing messages.
*/

//TODO(Andras) once only those methods are public that really need to be public, probably export the whole class instead of just some methods
class KMReaderWin: public QWidget {
  Q_OBJECT
  //TODO try to get rid of the friendship
  friend class MailViewer::ObjectTreeParser;
  friend class MailViewer::KHtmlPartHtmlWriter;
  friend class ::AttachmentURLHandler;
  friend class ::FallBackURLHandler;
  friend class ::HtmlAnchorHandler;

public:
  /**
   * Create a mail viewer widget
   * @param config a config object from where the configuration is read
   * @param parent parent widget
   * @param mainWindow the application's main window
   * @param actionCollection the action collection where the widget's actions will belong to
   * @param f window flags
   */
  MAILVIEWER_EXPORT KMReaderWin( QWidget *parent,  KSharedConfigPtr config = KSharedConfigPtr(), QWidget *mainWindow = 0,
               KActionCollection *actionCollection = 0, Qt::WindowFlags f = 0 );
  virtual ~KMReaderWin();

  /**
   * Returns the current message displayed in the viewer.
   */
  MAILVIEWER_EXPORT KMime::Message* message() const { return mMessage;} //TODO: convert mMessage to KMime::Message::Ptr ?

   /** Get codec corresponding to the currently selected override character encoding.
      @return The override codec or 0 if auto-detection is selected. */
  MAILVIEWER_EXPORT const QTextCodec * overrideCodec() const;

  /** The display update mode: Force updates the display immediately, Delayed updates
  after some time (150ms by default */
  enum UpdateMode {
    Force = 0,
    Delayed
  };

  enum Ownership {
    Transfer= 0,
    Keep
  };

  /** Set the message that shall be shown.
  * @param msg - the message to be shown. If 0, an empty page is displayed.
  * @param updateMode - update the display immediately or not. See UpdateMode.
  *  @param Ownership - Transfer means the ownership of the msg pointer is taken by the lib
  */
  MAILVIEWER_EXPORT void setMessage(KMime::Message* msg, UpdateMode updateMode = Delayed, Ownership = Keep);

  /** Set the Akonadi item that will be displayed.
  * @param item - the Akonadi item to be displayed. If it doesn't hold a mail (KMime::Message::Ptr as payload data),
  *               an empty page is shown.
  * @param updateMode - update the display immediately or not. See UpdateMode.
  */
  MAILVIEWER_EXPORT void setMessageItem(const Akonadi::Item& item, UpdateMode updateMode = Delayed );

  /** Convenience method to clear the reader and discard the current message. Sets the internal message pointer to 0.
  * @param updateMode - update the display immediately or not. See UpdateMode.
  */
  MAILVIEWER_EXPORT void clear(UpdateMode updateMode = Delayed ) { setMessage(0, updateMode); }

  /** Saves the relative position of the scroll view. Call this before calling update()
      if you want to preserve the current view. */
  MAILVIEWER_EXPORT void saveRelativePosition();

  /** Print message. */
  MAILVIEWER_EXPORT void printMessage(  KMime::Message* aMsg );

  /** Return selected text */
  MAILVIEWER_EXPORT QString copyText();

  /** Get the html override setting */
  MAILVIEWER_EXPORT bool htmlOverride() const { return mHtmlOverride; }

  /** Override default html mail setting */
  MAILVIEWER_EXPORT void setHtmlOverride( bool override );

  /** Get the load external references override setting */
  MAILVIEWER_EXPORT bool htmlLoadExtOverride() const { return mHtmlLoadExtOverride; }

/** Override default load external references setting */
  MAILVIEWER_EXPORT void setHtmlLoadExtOverride( bool override );

  /** Display a generic HTML splash page instead of a message.
  * @param info - the text to be displayed in HTML format
  */
  MAILVIEWER_EXPORT void displaySplashPage( const QString &info );

  /** Enable the displaying of messages again after an splash (or other) page was displayed */
  MAILVIEWER_EXPORT void enableMessageDisplay();

  /** Returns true if the message view is scrolled to the bottom. */
  MAILVIEWER_EXPORT bool atBottom() const;

  MAILVIEWER_EXPORT bool isFixedFont() { return mUseFixedFont; }
  MAILVIEWER_EXPORT void setUseFixedFont( bool useFixedFont ) { mUseFixedFont = useFixedFont; }

  //TODO: check if we want to keep the actions and expose them to the outside or
  //provide public slots only and let the user create the actions.
  
  // Action to reply to a message
  // but action( "some_name" ) some name could be used instead.
  MAILVIEWER_EXPORT KToggleAction *toggleFixFontAction() { return mToggleFixFontAction; }
  MAILVIEWER_EXPORT KAction *copyAction() { return mCopyAction; }
  MAILVIEWER_EXPORT MAILVIEWER_EXPORT KAction *selectAllAction() { return mSelectAllAction; }
  MAILVIEWER_EXPORT KAction *copyURLAction() { return mCopyURLAction; }
  MAILVIEWER_EXPORT KAction *urlOpenAction() { return mUrlOpenAction; }
  MAILVIEWER_EXPORT KAction *saveMessageAction() { return mSaveMessageAction; }

/*FIXME(Andras) port it - remove?
  /** Set the serial number of the message this reader window is currently
   *  waiting for. Used to discard updates for already deselected messages. 
  void setWaitingForSerNum( unsigned long serNum ) { mWaitingForSerNum = serNum; }
*/
  
  QWidget* mainWindow() { return mMainWindow; }

  /** Enforce message decryption. */
  MAILVIEWER_EXPORT void setDecryptMessageOverwrite( bool overwrite = true ) { mDecrytMessageOverwrite = overwrite; }

  /** Show signature details. */
  MAILVIEWER_EXPORT bool showSignatureDetails() const { return mShowSignatureDetails; }

  /** Show signature details. */
  MAILVIEWER_EXPORT void setShowSignatureDetails( bool showDetails = true ) { mShowSignatureDetails = showDetails; }

  /* show or hide the list that points to the attachments */
  MAILVIEWER_EXPORT bool showAttachmentQuicklist() const { return mShowAttachmentQuicklist; }

  /* show or hide the list that points to the attachments */
  MAILVIEWER_EXPORT void setShowAttachmentQuicklist( bool showAttachmentQuicklist = true ) { mShowAttachmentQuicklist = showAttachmentQuicklist; }

  /**
   * Get an instance for the configuration widget. The caller has the ownership and must delete the widget. See also configObject();
   * The caller should also call the widget's slotSettingsChanged() if the configuration has changed.
   */
  MAILVIEWER_EXPORT QWidget* configWidget();

  /**
   * Returns the configuration object that can be used in a KConfigDialog together with configWidget();
   */
  MAILVIEWER_EXPORT KConfigSkeleton *configObject();


  /** Returns the message part for a given content index. */
  KMime::Content* nodeForContentIndex( const KMime::ContentIndex& index );
  
protected:
//Below are the members that are called by friend classes
    /** Returns message part from given URL or null if invalid. */
  KMime::Content* nodeFromUrl(const KUrl &url);

  /** Open the attachment pointed to the node.
   * @param fileName - if not empty, use this file to load the attachment content
  */
  void openAttachment( KMime::Content *node, const QString & fileName );

  void emitUrlClicked( const KUrl & url, int button ) {
    emit urlClicked( url, button );
  }

  void emitPopupMenu( const KUrl & url, const QPoint & p ) {
    if ( mMessage )
      emit popupMenu( *mMessage, url, p );
  }

  /** Access to the KHTMLPart used for the viewer. Use with
      care! */
  KHTMLPart *htmlPart() const { return mViewer; }

  void showAttachmentPopup( int id, const QString & name, const QPoint & p );
  
  /** retrieve BodyPartMemento of id \a which for partNode \a node */
   MailViewer::Interface::BodyPartMemento * bodyPartMemento( const KMime::Content * node, const QByteArray & which ) const;

   /** set/replace BodyPartMemento \a memento of id \a which for
      partNode \a node. If there was a BodyPartMemento registered
      already, replaces (deletes) that one. */
   void setBodyPartMemento( const KMime::Content * node, const QByteArray & which, MailViewer::Interface::BodyPartMemento * memento );


private:
    /**
   * Sets the current attachment ID and the current attachment temporary filename
   * to the given values.
   * Call this so that slotHandleAttachment() knows which attachment to handle.
   */
  void prepareHandleAttachment( int id, const QString& fileName );

   /** deletes all BodyPartMementos. Use this when skipping to another
      message (as opposed to re-loading the same one again). */
   void clearBodyPartMementos();
   
  /** This function returns the complete data that were in this
  * message parts - *after* all encryption has been removed that
  * could be removed.
  * - This is used to store the message in decrypted form.
  */
  void objectTreeToDecryptedMsg( KMime::Content* node,
                                 QByteArray& resultingData,
                                 KMime::Message& theMessage,
                                 bool weAreReplacingTheRootNode = false,
                                 int recCount = 0 );

signals:
  /** Emitted after parsing of a message to have it stored
      in unencrypted state in it's folder. */
  void replaceMsgByUnencryptedVersion();

  /** The user presses the right mouse button. 'url' may be 0. */
  void popupMenu(KMime::Message &msg, const KUrl &url, const QPoint& mousePos);

  /** The user has clicked onto an URL that is no attachment. */
  void urlClicked(const KUrl &url, int button);

  /** Pgp displays a password dialog */
  void noDrag(void);

public slots:

  /** Re-parse the current message. */
  MAILVIEWER_EXPORT void update(KMReaderWin::UpdateMode updateMode = Delayed);

  /** Select message body. */
  void selectAll();

  /** HTML Widget scrollbar and layout handling. */
  void slotScrollUp();
  void slotScrollDown();
  void slotScrollPrior();
  void slotScrollNext();
  void slotJumpDown();
  void slotDocumentChanged();
  void slotDocumentDone();
  void slotTextSelected(bool);

  /** An URL has been activate with a click. */
  void slotUrlOpen(const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);

  /** The mouse has moved on or off an URL. */
  void slotUrlOn(const QString &url);

  /** The user presses the right mouse button on an URL. */
  void slotUrlPopup(const QString &, const QPoint& mousePos);

  /** The user selected "Find" from the menu. */
  void slotFind();

  /** The user toggled the "Fixed Font" flag from the view menu. */
  void slotToggleFixedFont();

  /** Show the message source */
  void slotShowMsgSrc();

  /** Copy the selected text to the clipboard */
  void slotCopySelectedText();

   void slotUrlClicked();

  /** Copy URL in mUrlCurrent to clipboard. Removes "mailto:" at
      beginning of URL before copying. */
  void slotUrlCopy();
  void slotUrlOpen( const KUrl &url = KUrl() );
  /** Save the page to a file */
  void slotUrlSave();

  void slotSaveMessage();

  void slotMessageArrived( KMime::Message *msg );

  void slotLevelQuote( int l );
  void slotTouchMessage();

  /** Delete the attachment the @param node points to. Returns false if the user
  cancelled the deletion, true in all other cases (including failure to delete
  the attachment!) */
  bool slotDeleteAttachment( KMime::Content* node, bool showWarning = true );

  /** Edit the attachment the @param node points to. Returns false if the user
  cancelled the editing, true in all other cases! */
  bool slotEditAttachment( KMime::Content* node, bool showWarning = true );

  /**
   * Does an action for the current attachment.
   * The action is defined by the KMHandleAttachmentCommand::AttachmentAction
   * enum.
   * prepareHandleAttachment() needs to be called before calling this to set the
   * correct attachment ID.
   */
  void slotHandleAttachment( int action );


private slots:
  /** Refresh the reader window */
  void updateReaderWin();

  void slotMimePartSelected( const QModelIndex &index );

  void slotCycleHeaderStyles();
  void slotBriefHeaders();
  void slotFancyHeaders();
  void slotEnterpriseHeaders();
  void slotStandardHeaders();
  void slotLongHeaders();
  void slotAllHeaders();

  void slotCycleAttachmentStrategy();
  void slotIconicAttachments();
  void slotSmartAttachments();
  void slotInlineAttachments();
  void slotHideAttachments();

  /** Some attachment operations. */
  void slotAtmView( KMime::Content *atmNode );
  void slotDelayedResize();

  /** Print message. Called on as a response of finished() signal of mPartHtmlWriter
      after rendering is finished.
      In the very end it deletes the KMReaderWin window that was created
      for the purpose of rendering. */
  void slotPrintMsg();

  void slotSetEncoding();
  void injectAttachments();
  void slotSettingsChanged();
  void slotMimeTreeContextMenuRequested( const QPoint& pos );
  void slotAttachmentOpenWith();
  void slotAttachmentOpen();
  void slotAttachmentSaveAs();
  void slotAttachmentView();
  void slotAttachmentSaveAll();
  void slotAttachmentProperties();
  void slotAttachmentCopy();
  void slotAttachmentDelete();
  void slotAttachmentEdit();
  void slotAttachmentEditDone(MailViewer::EditorWatcher* editorWatcher);

private:
  /** Is html mail to be supported? Takes into account override */
  bool htmlMail();

  /** Is loading ext. references to be supported? Takes into account override */
  bool htmlLoadExternal();

  /** Return the HtmlWriter connected to the KHTMLPart we use */
  MailViewer::HtmlWriter * htmlWriter() { return mHtmlWriter; }

  /** Returns whether the message should be decryted. */
  bool decryptMessage() const;

  MailViewer::CSSHelper* cssHelper() const;
//(Andras) end of moved methods

  /** reimplemented in order to update the frame width in case of a changed
      GUI style */
  void styleChange( QStyle& oldStyle );

  /** Set the width of the frame to a reasonable value for the current GUI
      style */
  void setStyleDependantFrameWidth();

  /** Watch for palette changes */
  virtual bool event(QEvent *e);

  /** Calculate the pixel size */
  int pointsToPixel(int pointSize) const;

  /** Feeds the HTML viewer with the contents of the given message.
    HTML begin/end parts are written around the message. */
  void displayMessage();

  /** Parse the root message and add it's contents to the reader window. */
  void parseMsg();

  /** Creates a nice mail header depending on the current selected
    header style. */
  QString writeMsgHeader( KMime::Message* aMsg, bool hasVCard = false, bool topLevel = false );

  /** Writes the given message part to a temporary file and returns the
      name of this file or QString() if writing failed.
  */
  QString writeMessagePartToTempFile( KMime::Content* msgPart );

  /**
    Creates a temporary dir for saving attachments, etc.
    Will be automatically deleted when another message is viewed.
    @param param Optional part of the directory name.
  */
  QString createTempDir( const QString &param = QString() );

  /** show window containing information about a vCard. */
  void showVCard(KMime::Content *msgPart);

  /** HTML initialization. */
  virtual void initHtmlWidget(void);

  /** Some necessary event handling. */
  virtual void closeEvent(QCloseEvent *);
  virtual void resizeEvent(QResizeEvent *);

  /** Cleanup the attachment temp files */
  virtual void removeTempFiles();

  /** Event filter */
  bool eventFilter( QObject *obj, QEvent *ev );

  /** Read settings from app's config file. */
  void readConfig();

  /** Write settings to app's config file. Calls sync() if withSync is true. */
  void writeConfig( bool withSync=true ) const;

   /** Get the message header style. */
  const MailViewer::HeaderStyle * headerStyle() const {
    return mHeaderStyle;
  }

  /** Set the header style and strategy. We only want them to be set
      together. */
  void setHeaderStyleAndStrategy( const MailViewer::HeaderStyle * style,
                                  const MailViewer::HeaderStrategy * strategy );

  /** Get the message header strategy. */
  const MailViewer::HeaderStrategy * headerStrategy() const {
    return mHeaderStrategy;
  }

  /** Get/set the message attachment strategy. */
  const MailViewer::AttachmentStrategy * attachmentStrategy() const {
    return mAttachmentStrategy;
  }
  void setAttachmentStrategy( const MailViewer::AttachmentStrategy * strategy );

  /** Get selected override character encoding.
      @return The encoding selected by the user or an empty string if auto-detection
      is selected. */
  QString overrideEncoding() const { return mOverrideEncoding; }

  /** Set the override character encoding. */
  void setOverrideEncoding( const QString & encoding );

  void setPrintFont( const QFont& font );

  /** Set printing mode */
  virtual void setPrinting(bool enable) { mPrinting = enable; }

  /** Instead of settings a message to be shown sets a message part
      to be shown */
  void setMessagePart( KMime::Content* aMsgPart, bool aHTML,
                   const QString& aFileName, const QString& pname );

  void setMessagePart( KMime::Content * node );

  /** Show or hide the Mime Tree Viewer if configuration
      is set to smart mode.  */
  void showHideMimeTree( bool isPlainTextTopLevel );

  /** View message part of type message/RFC822 in extra viewer window. */
  void atmViewMsg(KMime::Content* msgPart);

  KUrl tempFileUrlFromNode( const KMime::Content *node );

  void adjustLayout();
  void createWidgets();
  void createActions();
  void saveSplitterSizes( KConfigGroup & c ) const;

  void showContextMenu( KMime::Content* content, const QPoint& point);

  KToggleAction * actionForHeaderStyle( const MailViewer::HeaderStyle *,
                                       const MailViewer::HeaderStrategy * );
  KToggleAction * actionForAttachmentStrategy( const MailViewer::AttachmentStrategy * );
  /** Read override codec from configuration */
  void readGlobalOverrideCodec();

  QString renderAttachments( KMime::Content *node, const QColor &bgColor );

  KMime::Content* findContentByType(KMime::Content *content, const QByteArray &type);
    /**
   * Fixes an encoding received by a KDE function and returns the proper,
   * MIME-compilant encoding name instead.
   * @see encodingForName
   */
  static QString fixEncoding( const QString &encoding );

  /**
   * Drop-in replacement for KCharsets::encodingForName(). The problem with
   * the KCharsets function is that it returns "human-readable" encoding names
   * like "ISO 8859-15" instead of valid encoding names like "ISO-8859-15".
   * This function fixes this by replacing whitespace with a hyphen.
   */
  static QString encodingForName( const QString &descriptiveName );

  /** Return a QTextCodec for the specified charset.
   * This function is a bit more tolerant, than QTextCodec::codecForName */
  static const QTextCodec* codecForName(const QByteArray& _str);
  /**
   * Return a list of the supported encodings
   * @param usAscii if true, US-Ascii encoding will be prepended to the list.
   */
  static QStringList supportedEncodings( bool usAscii );

  QString createAtmFileLink( const QString& atmFileName ) const;
  KService::Ptr getServiceOffer( KMime::Content *content);
  bool saveContent( KMime::Content* content, const KUrl& url, bool encoded );
  void saveAttachments( const KMime::Content::List & contents );
  KMime::Content::List allContents( KMime::Content * content );
  KMime::Content::List selectedContents();
  void attachmentOpenWith( KMime::Content *node );
  void attachmentOpen( KMime::Content *node );

  bool mHtmlMail, mHtmlLoadExternal, mHtmlOverride, mHtmlLoadExtOverride;
  KMime::Message *mMessage; //the current message, if it was set manually
  Akonadi::Item mMessageItem; //the message item from Akonadi
  bool mDeleteMessage; //the message was created in the lib, eg. by calling setMessageItem()
  // widgets:
  QSplitter * mSplitter;
  KHBox *mBox;
  MailViewer::HtmlStatusBar *mColorBar;
  QTreeView* mMimePartTree; //FIXME(Andras) port the functionality from KMMimePartTree to a new view class or to here with signals/slots
  MailViewer::MimeTreeModel *mMimePartModel;
  KHTMLPart *mViewer;

  const MailViewer::AttachmentStrategy * mAttachmentStrategy;
  const MailViewer::HeaderStrategy * mHeaderStrategy;
  const MailViewer::HeaderStyle * mHeaderStyle;
  bool mAutoDelete;
  /** where did the user save the attachment last time */
  QString mSaveAttachDir;
  static const int delay;
  QTimer mUpdateReaderWinTimer;
  QTimer mResizeTimer;
  QTimer mDelayedMarkTimer;
  QString mOverrideEncoding;
  QString mOldGlobalOverrideEncoding; // used to detect changes of the global override character encoding
  bool mMsgDisplay;
  bool mNoMDNsWhenEncrypted;
  unsigned long mLastSerNum;
  MessageStatus mLastStatus;

  MailViewer::CSSHelper * mCSSHelper;
  bool mUseFixedFont;
  bool mPrinting;
  bool mShowColorbar;
  //bool mShowCompleteMessage;
  QStringList mTempFiles;
  QStringList mTempDirs;
  int mMimeTreeMode;
  bool mMimeTreeAtBottom;
  QList<int> mSplitterSizes;
  QString mIdOfLastViewedMessage;
  QWidget *mMainWindow;
  KActionCollection *mActionCollection;
  KAction *mCopyAction, *mCopyURLAction,
      *mUrlOpenAction, *mSelectAllAction,
      *mScrollUpAction, *mScrollDownAction, *mScrollUpMoreAction, *mScrollDownMoreAction, *mViewSourceAction, *mSaveMessageAction;
  KSelectAction *mSelectEncodingAction;
  KToggleAction *mToggleFixFontAction;
  KUrl mUrlClicked;
  MailViewer::HtmlWriter * mHtmlWriter;
  /** Used only to be able to connect and disconnect finished() signal
      in printMsg() and slotPrintMsg() since mHtmlWriter points only to abstract non-QObject class. */
  QPointer<MailViewer::KHtmlPartHtmlWriter> mPartHtmlWriter;
  QMap<QByteArray, MailViewer::Interface::BodyPartMemento*> mBodyPartMementoMap;

  int mChoice;
  unsigned long mWaitingForSerNum;
  float mSavedRelativePosition;
  int mLevelQuote;
  bool mDecrytMessageOverwrite;
  bool mShowSignatureDetails;
  bool mShowAttachmentQuicklist;
  bool mExternalWindow;
  QMap<MailViewer::EditorWatcher*, KMime::Content*> mEditorWatchers;
};


#endif

