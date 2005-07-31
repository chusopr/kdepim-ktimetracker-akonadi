/*
* maildrop.h -- Declaration of class KMailDrop.
* Generated by newclass on Sat Nov 29 20:07:45 EST 1997.
*/
#ifndef SSK_MAILDROP_H
#define SSK_MAILDROP_H

#include<qobject.h>
#include<qstring.h>
#include<qcolor.h>
#include<qvaluevector.h>
#include <qptrlist.h>

class Protocol;

class KConfigBase;
class KConfigGroup;
class KDropCfgDialog;
class QColor;
class KornMailSubject;
class KornMailId;

template< class T, class R > class QMap;

/**
* Abstract base class for all mailbox monitors.
* @author Sirtaj Singh Kang (taj@kde.org)
* @version $Id$
*/
class KMailDrop : public QObject
{
  Q_OBJECT

  public:

    enum  Style { Plain, Colour, Icon };

  private:

    QString _caption;
    QString _clickCmd;
    QString _nMailCmd;
    QString _soundFile;
    Style   _style;
    QColor  _bgColour;
    QColor  _fgColour;
    QColor  _nbgColour;
    QColor  _nfgColour;
    QString _icon;
    QString _nIcon;
    int     _lastCount;
    QString _realName;
    bool    _passivePopup;
    bool    _passiveDate;

  public:

    static const char *TypeConfigKey;
    static const char *CaptionConfigKey;
    static const char *ClickConfigKey;
    static const char *NewMailConfigKey;
    static const char *SoundFileConfigKey;
    static const char *DisplayStyleConfigKey;
    static const char *NFgColourConfigKey;
    static const char *NBgColourConfigKey;
    static const char *FgColourConfigKey;
    static const char *BgColourConfigKey;
    static const char *IconConfigKey;
    static const char *NewMailIconConfigKey;
    static const char *ResetCounterConfigKey;
    static const char *PassivePopupConfigKey;
    static const char *PassiveDateConfigKey; //Enabled date in Passive popup
    static const char *UseBoxSettingsConfigKey;
    static const char *RealNameConfigKey;
    
    /**
     * KMailDrop Constructor
     */
    KMailDrop();

    /**
     * KMailDrop Destructor
     */
    virtual ~KMailDrop();

    /** 
     * @return TRUE if the mailbox and its configuration are valid.
     */
    virtual bool valid() = 0;

    /** 
     * Number of messages in the mailbox at the last count.
     * @return The number of messages in the mailbox since last count.
     */
    int count() {return _lastCount;};

    /** 
     * Recheck the number of letters in this mailbox. Raises the
     * changed(int) signal if new mail is found.
     *
     * Concrete subclasses MUST reimplement this method.
     */
    virtual void recheck()=0;

    /**
     * Force a recheck
     */
    virtual void forceRecheck() { recheck(); }

    /** 
     */
    virtual bool startMonitor()=0;

    /** 
     */
    virtual bool stopMonitor()=0;

    /** 
     * Check monitor run status.
     * @return true if monitor is running.
     */
    virtual bool running()=0;

    /** 
     * Add a configuration page to the configuration dialog.
     * Each reimplementation should first call the inherited implementation,
     * then call @ref KDropCfgDialog::addConfigPage with a custom
     * @ref KMonitorCfg object.
     */
//    virtual void addConfigPage( KDropCfgDialog * );

    /** 
     * Returns a newly created KBoxFactory object initialized to
     * be equivalent to this object (prototype pattern). 
     *
     * Deletion of the returned object becomes the responsibility of 
     * the caller.
     *
     * Subclasses should override this to return objects of their
     * own type.
     */
    virtual KMailDrop *clone() const = 0;

    /**
     * This function reads the settings which can be used by several
     * accounts. These values can be overwritten by the readConfigGroup
     * -function.
     *
     *@param cfg A configuration object with the group already
     * set to the configuration for this box
     */
    virtual void readGeneralConfigGroup( const KConfigBase& cfg );

    /** 
     * Read box configuration from a config group. Subclasses that
     * reimplement this should call the overridden method.
     *
     * @param cfg  A configuration object with the group already set to
     *     the configuration for this box.
     * @return true if read was successful, false otherwise.
     */
    virtual bool readConfigGroup( const KConfigBase& cfg );
    virtual bool readConfigGroup( const QMap< QString, QString > &, const Protocol * ) { return true; }

    /** 
     * Write box configuration to a config group. Subclasses that
     * reimplement this should call the overridden method.
     *
     * @param cfg  A configuration object with the group already set to
     *     the configuration for this box.
     * @return true if read was successful, false otherwise.
     */
    virtual bool writeConfigGroup( KConfigBase& cfg ) const;

    /** 
     * Return the type of this monitor, for display and
     * configuration purposes. Each concrete subclass should return a 
     * unique identifier.
     */
    virtual QString type() const = 0;

    /**
     * Return if the maildrop is synchrone (true) or asynchrone (false).
     * This way, classes like KornSubjectDlg know if functions like
     * readSubject() return a result immediately.
     * @param true by a synchrone type; false by an asynchrone (like KKkioDrop) type.
     */
    virtual bool synchrone() const { return true; }
    
    /**
     * Return true if the concrete subclass can read the subjects of
     * all new mails. This will enable the "Read Subjects" menu item.
     */
    virtual bool canReadSubjects() {return false;}

    /** 
     * Read the subjects of all new mails.
     * NOTE: the default implementation stops the timer, calls
     * doReadSubjects, restarts the time if necessary and updates
     * the displayed mail count. Concrete implementations should not
     * touch readSubjects() but implement doReadSubjects() instead!
     * @param stop: stop flag. If it is set to true during the execution,
     * readSubjects shoulkd return as soon as possible. The return value
     * is invalid in this case. If stop is 0, readSubjects will not
     * terminate before all mail subjects are loaded.
     * @return all new mails subjects as a vector.
     */
    virtual QValueVector<KornMailSubject> * readSubjects(bool * stop);

    /**
     * Read the subjects of all new mails. The concrete subclass has
     * to implement it, if canReadSubjects() returns true.
     * @param stop: stop flag. If it is set to true during the execution,
     * readSubjects should return as soon as possible. The return value
     * is invalid in this case. If stop is 0, readSubjects will not
     * terminate before all mail subjects are loaded.
     * @return all new mails subjects as a vector.
     */
    virtual QValueVector<KornMailSubject> * doReadSubjects(bool * stop);

    /**
     * Return true if the concrete subclass can delete individual mails.
     * This will enable the "Delete" button in the mail subjects dialog.
     */
    virtual bool canDeleteMails() {return false;}

    /**
     * Delete some mails in the mailbox. The concrete subclass has
     * to implement it, if canDeleteMails() returns true.
     * @param ids list of mail ids to delete. The ids are taken from
     * the corresponding KornMailSubject instances returned by a previous
     * call to doReadSubjects().
     * @param stop: stop flag. If it is set to true during the execution,
     * deleteMails() should return as soon as possible. The return value
     * is invalid in this case. If stop is 0, deleteMails() will not
     * terminate before the mails are deleted.
     * @return true, if the mail ids of the remaining mails might have changed.
     * The corresponding KornMailSubject instances returned by a previous
     * call to doReadSubjects() have to be discarded and readSubjects() must
     * be called again to get the correct mail ids. If false is returned,
     * the KornMailSubject instances of the remaining mails might be used
     * further more.
     */
    virtual bool deleteMails(QPtrList<const KornMailId> * ids, bool * stop);

    /**
     * Return true if the concrete subclass can load individual mails fully.
     * This will enable the "Full Message" button in the mail dialog.
     */
    virtual bool canReadMail() {return false;}

    /**
     * Load a mail from the mailbox fulle . The concrete subclass has
     * to implement it, if deleteMails() returns true.
     * @param id id of the mail to load. The id is taken from the corresponding 
     * KornMailSubject instances returned by a previous call to doReadSubjects().
     * @param stop: stop flag. If it is set to true during the execution,
     * readMail() should return as soon as possible. The return value
     * is invalid in this case. If stop is 0, readMail() will not
     * terminate before the mail is loaded.
     * @return the fully loaded mail (header and body) or "" on error.
     */
    virtual QString readMail(const KornMailId * id, bool * stop);

    // data that belongs in every monitor

    QString       caption()       const { return _caption; }
    QString       clickCmd()      const { return _clickCmd; }
    QString       newMailCmd()    const { return _nMailCmd; }
    QString       soundFile()     const { return _soundFile;}
    QColor        bgColour()      const { return _bgColour; }
    QColor        fgColour()      const { return _fgColour; }
    QColor        newBgColour()   const { return _nbgColour; }
    QColor        newFgColour()   const { return _nfgColour; }
    QString       icon()          const { return _icon; }
    QString       newIcon()       const { return _nIcon; }
    Style         displayStyle()  const { return _style; }
    bool          passivePopup()  const { return _passivePopup; }
    bool	  passiveDate()   const { return _passiveDate; }
    QString       realName()      const { return _realName; }
;
    void setCaption(QString);
    void setClickCmd(QString);
    void setNewMailCmd(QString);
    void setSoundFile(QString);
    void setDisplayStyle(Style);
    void setBgColour(QColor);
    void setFgColour(QColor);
    void setNewBgColour(QColor);
    void setNewFgColour(QColor);
    void setIcon(QString);
    void setNewIcon(QString);
    void setResetCounter(int);
    void setPassivePopup(bool);
    void setPassiveDate(bool);
    void setRealName(QString);

    /** 
     * This is called by the manager when it wishes to delete
     * a monitor. Clients should connect to the @ref ::notifyDisconnect
     * signal and ensure that the monitor is not accessed after
     * the signal has been received.
     *
     * Reimplementations should call this implementation too.
     */
    virtual void notifyClients();

    public slots:

    /**
     * Forcibly set the count to zero;
     */
    virtual void forceCountZero();
    
    /*
     * The next slots are used by kio; the present at this places
     * prevent warnings at runtime.
     */
    virtual void readSubjectsCanceled() {}
    virtual void readMailCanceled() {}
    virtual void deleteMailsCanceled() {}

    protected slots:

      void setCount( int, KMailDrop* );
      
signals:

    /** 
     * This signal is emitted when the mailbox discovers 
     * new messages in the maildrop.
     */
    void changed( int, KMailDrop* );

    /**
     * This signal is emitted when the valid-status changes.
     * @param isValid true then and only then if the box is valid
     */
    void validChanged( bool isValid );

    /** 
     * This is emitted on configuration change, normally
     * on an updateConfig() but 
     */
    void configChanged();

    /** 
     * Clients should connect to this and discontinue use
     * after it is emitted.
     */
    void notifyDisconnect();

    /**
     * rechecked() is called if an asynchrone maildrop has
     * rechecked the availability of email.
     */
    void rechecked();
    
    /**
     * The next signal is emitted when a passive popup could be displayed.
     * As argument, there is a KornSubject, which contains a subject and
     * some more info that could be used with the popup.
     */
    void showPassivePopup( QPtrList< KornMailSubject >*, int, bool, const QString& realname );

    /**
     * This signal is emitted when a passive error message should be displayed.
     *
     * @param error The error message
     * @param realName The real name of this object.
     */
    void showPassivePopup( const QString& error, const QString& realname );
    
    /**
     * readSubjects() might signal readSubject() if
     * an subject is received. This is only useful in
     * asynchrone situations.
     * @param the subject structure which is read
     */
    void readSubject( KornMailSubject * );
    
    /**
     * readSubjects() might signal readSubjectsTotalSteps() to
     * send the expected total number of steps to a possible
     * progress bar. See readSubjectsProgress();
     * @param totalSteps expected total number of steps.
     */
    void readSubjectsTotalSteps(int totalSteps);

    /**
     * readSubjects() might signal readSubjectsProgress() to
     * send the current progress in relation to the
     * expected total number of steps (see readSubjectsTotalSteps()).
     * @param curent progress.
     */
    void readSubjectsProgress(int progress);
    
    /**
     * readSubjects() might signal readSubjectsReady() to
     * remove the progress bar in asynchrone situations.
     * @param: true if succes, false if cancelled
     */
    void readSubjectsReady( bool success );

    /**
     * deleteMails() might signal deleteMailsTotalSteps() to
     * send the expected total number of steps to a possible
     * progress bar. See deleteMailsProgress();
     * @param totalSteps expected total number of steps.
     */
    void deleteMailsTotalSteps(int totalSteps);

    /**
     * deleteMails() might signal deleteMailsProgress() to
     * send the current progress in relation to the
     * expected total number of steps (see deleteMailsTotalSteps()).
     * @param curent progress.
     */
    void deleteMailsProgress(int progress);

    /**
     * deleteMails() might signal deleteMailsReady() if
     * it is not going to do something anyway.
     * This could be the case when an email has been succesfully
     * removed, or when the deletions failed. This is useful
     * in asynchrone situations.
     * @param: true if deletion was succesful; elsewise false.
     */
    void deleteMailsReady( bool );
    
    /**
     * readMail() might signal readMailTotalSteps() to
     * send the expected total number of steps to a possible
     * progress bar. See readMailProgress();
     * @param totalSteps expected total number of steps.
     */
    void readMailTotalSteps(int totalSteps);

    /**
     * readMail() might signal readMailProgress() to
     * send the current progress in relation to the
     * expected total number of steps (see readMailTotalSteps()).
     * @param curent progress.
     */
    void readMailProgress(int progress);
    
    /**
     * readMail() might signal readMailReady() if
     * a email is totally read. This is useful
     * in asynchrone situations.
     * @param pointer to the full email-message.
     */
    void readMailReady( QString* );
};

#endif // SSK_MAILDROP_H
