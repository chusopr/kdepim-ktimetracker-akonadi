// System includes
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

// Qt includes
#include <qdatetime.h>

// Local includes
#include <rmm/DateTime.h>
#include <rmm/Enum.h>
#include <rmm/Token.h>

using namespace RMM;

DateTime::DateTime()
    :    HeaderBody()
{
    // Empty.
}

DateTime::DateTime(const QCString & s)
    :    HeaderBody(s)
{
    // Empty.
}

DateTime::DateTime(const DateTime & t)
    :   HeaderBody(t),
        zone_   (t.zone_.copy()),
        qdate_  (t.qdate_)
{
    // Empty.
}

DateTime::~DateTime()
{ 
    // Empty.
}

    DateTime &
DateTime::operator = (const DateTime & t)
{
    if (this == &t) return *this; // Don't do a = a.
    qdate_  = t.qdate_;
    zone_   = t.zone_.copy();
    
    HeaderBody::operator = (t);
    return *this;
}

    DateTime &
DateTime::operator = (const QCString & s)
{
    HeaderBody::operator = (s);
    return *this;
}

    bool
DateTime::operator == (DateTime & dt)
{
    parse();
    dt.parse();

    return (qdate_ == dt.qdate_ && zone_ == dt.zone_);
}

    QDataStream &
RMM::operator >> (QDataStream & s, DateTime & dt)
{
    s   >> dt.qdate_
        >> dt.zone_;
    dt.parsed_        = true;
    dt.assembled_    = false;
    return s;
}

    QDataStream &
RMM::operator << (QDataStream & s, DateTime & dt)
{
    dt.parse();
    s    << dt.qdate_
        << dt.zone_;
    return s;
}

    QDateTime
DateTime::qdt()
{
    parse();
    return qdate_;
}

    QCString
DateTime::timeZone()
{
    parse();
    return zone_;
}

    void
DateTime::_parse()
{
    Q_UINT32    dayOfMonth_;
    Q_UINT32    month_;
    Q_UINT32    year_;
    Q_UINT32    hour_;
    Q_UINT32    min_;
    Q_UINT32    sec_;

    QStrList tokens;
    tokenise(strRep_, " :", tokens);

    // date-time = [day ","] date time

    if (tokens.count() < 6 || tokens.count() > 9) {
        // Invalid date-time !
        rmmDebug("Invalid date-time");
        return;
    }
    
    int i = 0;
    
    // If the first token can be identified as a day name, then ignore it.
    
    bool haveDay = false;
    if (isalpha(tokens.at(i)[0])) { haveDay = true; i++; }

    if (tokens.at(i)[0] == '0')
        dayOfMonth_ = tokens.at(i++)[1] - '0';
    else
        dayOfMonth_ = atoi(tokens.at(i++));

    month_ = strToMonth(tokens.at(i++));

    if (strlen(tokens.at(i)) == 2)
        year_ = atoi(tokens.at(i++)) + 1900;
    else
        year_ = atoi(tokens.at(i++));
    
    hour_    = atoi(tokens.at(i++));
    min_    = atoi(tokens.at(i++));
    
    // If the earlier token for day of week was there, and the total token
    // count is 8, then we must also have a seconds field next
    // OR if the earlier token for day was NOT there, and the total token count
    // is 7, then again, we must have a seconds field.
    
    if (( haveDay && (tokens.count() == 7)) ||
        (!haveDay && (tokens.count() == 6)))
        sec_ = strtol(tokens.at(i++), NULL, 10);
    else
        sec_ = 0;

    if (tokens.count() - 1 == (unsigned)i)
        zone_ = tokens.at(i);
    
    QDate d;
    d.setYMD(year_, month_, dayOfMonth_);
    qdate_.setDate(d);
    
    QTime t;
    t.setHMS(hour_, min_, sec_);
    qdate_.setTime(t);
}

    void
DateTime::_assemble()
{
    if (!qdate_.isValid()) {
        rmmDebug("I'm not VALID !");
        return;
    }

    QDate d = qdate_.date();
    QTime t = qdate_.time();
    
    strRep_ = d.dayName(d.dayOfWeek()).ascii();
    strRep_ += ',';
    strRep_ += ' ';
    strRep_ += QCString().setNum(d.day());
    strRep_ += ' ';
    strRep_ += d.monthName(d.month()).ascii();
    strRep_ += ' ';
    strRep_ += QCString().setNum(d.year());
    strRep_ += ' ';
    strRep_ += t.toString().ascii();

    if (!zone_.isEmpty())
        strRep_ += ' ' + zone_;
}

    void
DateTime::createDefault()
{
    qdate_ = QDateTime::currentDateTime();
    zone_ = "";
    parsed_ = true;
    assembled_ = false;
}

    Q_UINT32
DateTime::asUnixTime()
{
    parse();
    struct tm timeStruct;
    
    QDate d = qdate_.date();
    QTime t = qdate_.time();
    
    timeStruct.tm_sec    = t.second();
    timeStruct.tm_min    = t.minute();
    timeStruct.tm_hour    = t.hour();
    timeStruct.tm_mday    = d.day();
    timeStruct.tm_mon    = d.month() - 1;
    timeStruct.tm_year    = d.year() - 1900;
    timeStruct.tm_isdst    = -1; // Unknown
    
    time_t timeT = mktime(&timeStruct);    

    return (Q_UINT32)timeT;    
}

// vim:ts=4:sw=4:tw=78
