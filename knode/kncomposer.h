/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCOMPOSER_H
#define KNCOMPOSER_H

#include <k3listview.h>

#include <kmainwindow.h>
#include <kdialog.h>
#include <keditcl.h>
#include <QLineEdit>
#include <QRegExp>
//Added by qt3to4:
#include <QByteArray>
#include <QCloseEvent>
#include <Q3PtrList>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QLabel>
#include <QList>
#include <QContextMenuEvent>
#include <QMenu>
#include <QDragEnterEvent>
#include <QSplitter>

#include <kdeversion.h>
#include <keditcl.h>

#include <kabc/addresslineedit.h>
#include <knodecomposeriface.h>

class Q3GroupBox;

class KProcess;
class KSpell;
class K3DictSpellingHighlighter;
class KSelectAction;
class KToggleAction;

class KNLocalArticle;
class KNAttachment;
class SpellingFilter;
class KComboBox;
class QComboBox;
/** Message composer window. */
class KNComposer : public KMainWindow , virtual public KNodeComposerIface {

  Q_OBJECT

  public:
    enum composerResult { CRsendNow, CRsendLater, CRdelAsk,
                          CRdel, CRsave, CRcancel };
    enum MessageMode { news=0, mail=1, news_mail=2 };

    // unwraped == original, not rewraped text
    // firstEdit==true: place the cursor at the end of the article
    KNComposer( KNLocalArticle *a, const QString &text = QString(), const QString &sig = QString(),
                const QString &unwraped = QString(), bool firstEdit = false,
                bool dislikesCopies = false, bool createCopy = false );
    ~KNComposer();
    void setConfig(bool onlyFonts);
    void setMessageMode(MessageMode mode);

    //get result
    bool hasValidData();
    composerResult result() const              { return r_esult; }
    KNLocalArticle* article()const             { return a_rticle; }
    bool applyChanges();

    void closeEvent(QCloseEvent *e);

    //set data from the given article
    void initData(const QString &text);

    /** Inserts at cursor position if clear is false, replaces content otherwise
     * puts the file content into a box if box==true
     * "file" is already open for reading
     */
    void insertFile( QFile *file, bool clear = false, bool box = false, const QString &boxTitle = QString() );

    /// ask for a filename, handle network urls
    void insertFile(bool clear=false, bool box=false);

    QMenu * popupMenu( const QString& name );
    int listOfResultOfCheckWord( const QStringList & lst , const QString & selectWord);

//internal classes
    class ComposerView;
    class Editor;
    class AttachmentView;
    class AttachmentViewItem;
    class AttachmentPropertiesDlg;

    //GUI
    ComposerView *v_iew;
    QMenu *a_ttPopup;

    //Data
    composerResult r_esult;
    KNLocalArticle *a_rticle;
    QString s_ignature, u_nwraped;
    QByteArray c_harset;
    MessageMode m_ode;
    bool n_eeds8Bit,    // false: fall back to us-ascii
         v_alidated,    // hasValidData was run and found no problems, n_eeds8Bit is valid
         a_uthorDislikesMailCopies;

    //edit
    bool e_xternalEdited;
    KProcess *e_xternalEditor;
    KTempFile *e_ditorTempfile;
    KSpell *s_pellChecker;
    SpellingFilter* mSpellingFilter;

    //Attachments
    QList<KNAttachment*> mDeletedAttachments;
    QList<KAction*> m_listAction;
    bool a_ttChanged;

  //------------------------------ <Actions> -----------------------------

    KAction       *a_ctExternalEditor,
                  *a_ctSpellCheck,
                  *a_ctRemoveAttachment,
                  *a_ctAttachmentProperties,
                  *a_ctSetCharsetKeyb;
    KToggleAction *a_ctPGPsign,
                  *a_ctDoPost, *a_ctDoMail, *a_ctWordWrap;
    KSelectAction *a_ctSetCharset;
    bool spellLineEdit;
  protected slots:
    void slotSendNow();
    void slotSendLater();
    void slotSaveAsDraft();
    void slotArtDelete();
    void slotAppendSig();
    void slotInsertFile();
    void slotInsertFileBoxed();
    void slotAttachFile();
    void slotRemoveAttachment();
    void slotAttachmentProperties();
    void slotToggleDoPost();
    void slotToggleDoMail();
    void slotSetCharset(const QString &s);
    void slotSetCharsetKeyboard();
    void slotToggleWordWrap();
    void slotUndoRewrap();
    void slotExternalEditor();
    void slotSpellcheck();

    void slotUpdateStatusBar();
    void slotUpdateCursorPos();
    void slotConfKeys();
    void slotConfToolbar();
    void slotNewToolbarConfig();

  //------------------------------ </Actions> ----------------------------

    // GUI
    void slotSubjectChanged(const QString &t);
    void slotGroupsChanged(const QString &t);
    void slotToBtnClicked();
    void slotGroupsBtnClicked();

    // external editor
    void slotEditorFinished(KProcess *);
    void slotCancelEditor();

    // attachment list
    void slotAttachmentPopup(K3ListView*, Q3ListViewItem *it, const QPoint &p);
    void slotAttachmentSelected(Q3ListViewItem *it);
    void slotAttachmentEdit(Q3ListViewItem *it);
    void slotAttachmentRemove(Q3ListViewItem *it);

    // spellcheck operation
    void slotSpellStarted(KSpell *);
    void slotSpellDone(const QString&);
    void slotSpellFinished();

    // DND handling
    virtual void slotDragEnterEvent(QDragEnterEvent *);
    virtual void slotDropEvent(QDropEvent *);

    void slotUndo();
    void slotRedo();
    void slotCut();
    void slotCopy();
    void slotPaste();
    void slotSelectAll();
    void slotMisspelling(const QString &text, const QStringList &lst, unsigned int pos);
    void slotCorrected (const QString &oldWord, const QString &newWord, unsigned int pos);
    void addRecentAddress();

  protected:

    // DND handling
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);

  signals:
    void composerDone(KNComposer*);

  private:
    bool mFirstEdit;

};



class KNLineEditSpell;
class KNLineEdit;

/** Message composer view. */
class KNComposer::ComposerView  : public QSplitter {

  public:
    ComposerView( KNComposer *_composer );
    ~ComposerView();
    void focusNextPrevEdit(const QWidget* aCur, bool aNext);
    void setMessageMode(KNComposer::MessageMode mode);
    void showAttachmentView();
    void hideAttachmentView();
    void showExternalNotification();
    void hideExternalNotification();
    void restartBackgroundSpellCheck();
    QList<QWidget*> mEdtList;

    QLabel      *l_to,
                *l_groups,
                *l_fup2;
    KNLineEditSpell *s_ubject;

    KNLineEdit   *g_roups;
    KNLineEdit  *t_o;

    KComboBox   *f_up2;
    QPushButton *g_roupsBtn,
                *t_oBtn;

    Editor      *e_dit;
    Q3GroupBox   *n_otification;
    QPushButton *c_ancelEditorBtn;

    QWidget         *a_ttWidget;
    AttachmentView  *a_ttView;
    QPushButton     *a_ttAddBtn,
                    *a_ttRemoveBtn,
                    *a_ttEditBtn;
    K3DictSpellingHighlighter *mSpellChecker;

    bool v_iewOpen;
};


/** Message Compser editor, handles tabs (expanding them in textLine(), etc.) */
class KNComposer::Editor : public KEdit {

  Q_OBJECT

  public:
    Editor( KNComposer::ComposerView *_composerView, KNComposer *_composer, QWidget *parent = 0 );
    ~Editor();
    QStringList processedText();

  public slots:
    void slotPasteAsQuotation();
    void slotFind();
    void slotSearchAgain();
    void slotReplace();
    void slotAddQuotes();
    void slotRemoveQuotes();
    void slotAddBox();
    void slotRemoveBox();
    void slotRot13();
    void slotCorrectWord();

protected slots:
    void slotSpellStarted( KSpell *);
    void slotSpellDone(const QString &);
    void slotSpellFinished();
    void slotMisspelling (const QString &, const QStringList &lst, unsigned int);
    virtual void cut();
    virtual void clear();
    virtual void del();
    void slotAddSuggestion( const QString &, const QStringList &lst, unsigned int );
  signals:
    void sigDragEnterEvent(QDragEnterEvent *);
    void sigDropEvent(QDropEvent *);

  protected:

    // DND handling
    virtual void contentsDragEnterEvent(QDragEnterEvent *);
    virtual void contentsDropEvent(QDropEvent *);
    virtual void contentsContextMenuEvent( QContextMenuEvent *e );
    virtual void keyPressEvent ( QKeyEvent *e);

    virtual bool eventFilter(QObject*, QEvent*);
private:
    KNComposer *m_composer;
    KNComposer::ComposerView *m_composerView;
    KSpell *spell;
    QMap<QString,QStringList> m_replacements;
    QRegExp m_bound;
};


/** Attachment view of the message composer. */
class KNComposer::AttachmentView : public K3ListView {

  Q_OBJECT

  public:
    AttachmentView( QWidget *parent );
    ~AttachmentView();

  protected:
    void keyPressEvent( QKeyEvent *e );

  signals:
    void delPressed ( Q3ListViewItem * );      // the user used Key_Delete on a list view item
};


/** Attachment view item. */
class KNComposer::AttachmentViewItem : public K3ListViewItem {

  public:
    AttachmentViewItem(K3ListView *v, KNAttachment *a);
    ~AttachmentViewItem();

  KNAttachment *attachment;

};


/** Attachment properties dialog. */
class KNComposer::AttachmentPropertiesDlg : public KDialog {

  Q_OBJECT

  public:
    AttachmentPropertiesDlg( KNAttachment *a, QWidget *parent = 0 );
    ~AttachmentPropertiesDlg();

    void apply();

  protected:
    KLineEdit *m_imeType,
              *d_escription;
    QComboBox *e_ncoding;

    KNAttachment *a_ttachment;
    bool n_onTextAsText;

  protected slots:
    void accept();
    void slotMimeTypeTextChanged(const QString &text);
};

//-----------------------------------------------------------------------------
/** Line edit for addresses used in the composer. */
class KNLineEdit : public KABC::AddressLineEdit
{
    Q_OBJECT
    typedef KABC::AddressLineEdit KNLineEditInherited;
public:

    KNLineEdit( KNComposer::ComposerView *_composerView, bool useCompletion, QWidget *parent = 0 );
protected:
    // Inherited. Always called by the parent when this widget is created.
    virtual void loadAddresses();
    void keyPressEvent(QKeyEvent *e);
    virtual void contextMenuEvent( QContextMenuEvent*e );
private slots:
    void editRecentAddresses();
private:
    KNComposer::ComposerView *composerView;
};

/** Line edit with on-the-fly spell checking. */
class KNLineEditSpell : public KNLineEdit
{
    Q_OBJECT
public:
    KNLineEditSpell( KNComposer::ComposerView *_composerView, bool useCompletion, QWidget * parent );
    void highLightWord( unsigned int length, unsigned int pos );
    void spellCheckDone( const QString &s );
    void spellCheckerMisspelling( const QString &text, const QStringList &, unsigned int pos);
    void spellCheckerCorrected( const QString &old, const QString &corr, unsigned int pos);
};

#endif
