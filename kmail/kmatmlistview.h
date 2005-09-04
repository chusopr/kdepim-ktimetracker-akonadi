/* -*- mode: C++; c-file-style: "gnu" -*-
 * KMComposeWin Header File
 * Author: Markus Wuebben <markus.wuebben@kde.org>
 */
#ifndef __KMAIL_KMATMLISTVIEW_H__
#define __KMAIL_KMATMLISTVIEW_H__

#include <q3listview.h>
#include <q3cstring.h>

class KMComposeWin;
class MessageComposer;
class QCheckBox;

class KMAtmListViewItem : public QObject, public Q3ListViewItem
{
  Q_OBJECT
  friend class ::KMComposeWin;
  friend class ::MessageComposer;

public:
  KMAtmListViewItem(Q3ListView * parent);
  virtual ~KMAtmListViewItem();
  virtual void paintCell( QPainter * p, const QColorGroup & cg,
                          int column, int width, int align );

  void setUncompressedMimeType( const Q3CString & type, const Q3CString & subtype ) {
    mType = type; mSubtype = subtype;
  }
  void uncompressedMimeType( Q3CString & type, Q3CString & subtype ) const {
    type = mType; subtype = mSubtype;
  }
  void setUncompressedCodec( const Q3CString & codec ) { mCodec = codec; }
  Q3CString uncompressedCodec() const { return mCodec; }

signals:
  void compress( int );
  void uncompress( int );

protected:
  void enableCryptoCBs(bool on);
  void setEncrypt(bool on);
  bool isEncrypt();
  void setSign(bool on);
  bool isSign();
  void setCompress(bool on);
  bool isCompress();

private slots:
  void slotCompress();

private:
  Q3ListView* mListview;
  QCheckBox* mCBEncrypt;
  QCheckBox* mCBSign;
  QCheckBox* mCBCompress;
  bool mCBSignEnabled, mCBEncryptEnabled;
  Q3CString mType, mSubtype, mCodec;
};


#endif // __KMAIL_KMATMLISTVIEW_H__
