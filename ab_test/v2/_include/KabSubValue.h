#include <qvaluelist.h>

#include <KabEnum.h>

#ifndef KAB_SUBVALUE_H
#define KAB_SUBVALUE_H

namespace KAB
{

class SubValue
{
  public:
  
    SubValue()
    {
      // Empty.
    }
    
    SubValue(const SubValue & v)
      : name_(v.name_),
        data_(v.data_)
    {
      // Empty.
    }
      
    ~SubValue()
    {
      // Empty.
    }
  
    SubValue & operator = (const SubValue & s)
    {
      if (this == &s) return *this;

      name_ = s.name_;
      data_ = s.data_;
      
      return *this;
    }
    
    bool operator == (const SubValue & s) const
    {
      return ((name_ == s.name_) && (data_ == s.data_));
    } 
        
    QString     name()  const { return name_; }
    QByteArray  data()  const { return data_; }
    
    void  setName(const QString     & n) { name_ = n; }
    void  setData(const QByteArray  & d) { data_ = d; }
  
    friend QDataStream & operator << (QDataStream &, const SubValue &);
    friend QDataStream & operator >> (QDataStream &, SubValue &);
    
  private:
    
    QString     name_;
    QByteArray  data_;
};

typedef QValueList<SubValue> SubValueList;
typedef SubValue XValue;
typedef QValueList<XValue> XValueList;

  QDataStream &
operator << (QDataStream & str, const SubValue & v)
{
  str << v.name_ << v.data_;
  return str;
}

  QDataStream &
operator >> (QDataStream & str, SubValue & v)
{
  str >> v.name_ >> v.data_;
  return str;
}

} // End namespace KAB

#endif

