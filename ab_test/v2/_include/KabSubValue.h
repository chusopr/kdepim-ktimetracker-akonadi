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
  
    virtual void save(QDataStream & str) { str << name_ << data_; }
    virtual void load(QDataStream & str) { str >> name_ >> data_; }
    
  private:
    
    QString     name_;
    QByteArray  data_;
};

typedef QValueList<SubValue> SubValueList;
typedef SubValue XValue;
typedef QValueList<XValue> XValueList;

} // End namespace KAB

#endif

