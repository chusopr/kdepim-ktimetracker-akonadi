#ifndef OVERVIEWPROGRESSENTRY_H
#define OVERVIEWPROGRESSENTRY_H

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpixmap.h>

namespace KSync {
namespace OverView {

    class OverViewProgressEntry : public QWidget {

	Q_OBJECT

    public:
        OverViewProgressEntry( QWidget* parent, const char* name );
        ~OverViewProgressEntry();
	
	void setText( QString );
	void setProgress( QString );
	void setPixmap( QPixmap );

    private:
	
	QHBoxLayout* m_layout;
	QLabel* m_textLabel;
	QLabel* m_progressField;
	QLabel* m_pixmapLabel;

    };
}
}

#endif
