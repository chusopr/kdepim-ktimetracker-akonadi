#include "kcalcore/kcalcore_export.h"
#include "kcalcore/calstorage.h"
#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KJob>

namespace KCalCore {

class CalFormat;
class Calendar;

class KCALCORE_EXPORT AkonadiStorage : public CalStorage
{
  Q_OBJECT
  public:

    typedef QSharedPointer<AkonadiStorage> Ptr;

    explicit AkonadiStorage( const Calendar::Ptr &calendar,
                          const Akonadi::Collection &collection );

    virtual ~AkonadiStorage();

    void setCollection( const Akonadi::Collection &collection );

    Akonadi::Collection collection() const;

    bool open();

    bool load();

    bool save();

    bool close();

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( AkonadiStorage )
    class Private;
    Private *const d;
    //@endcond
    int fetchResult;
    int saveResult;
  private Q_SLOTS:
    void fetchJobResult(KJob* job);
    void saveJobResult(KJob* job);
    void itemsReceived(const Akonadi::Item::List &items);
};

}