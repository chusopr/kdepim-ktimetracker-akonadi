
// Qt includes
#include <qstrlist.h>
#include <qlabel.h>
#include <qpainter.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kpixmap.h>

// Local includes
#include "EmpathHeaderViewWidget.h"
#include "EmpathUIUtils.h"
#include "EmpathConfig.h"
#include "EmpathDefines.h"
#include <RMM_Header.h>

EmpathHeaderViewWidget::EmpathHeaderViewWidget(
		QWidget * parent, const char * name)
	:	QWidget(parent, name),
		glowing_(false)
{
	empathDebug("ctor");
	clipIcon_ = empathIcon("clip.png");
	clipGlow_ = empathIcon("clip-glow.png");
	setMouseTracking(true);
	setFixedHeight(0);
}

EmpathHeaderViewWidget::~EmpathHeaderViewWidget()
{
	empathDebug("dtor");
}

	void
EmpathHeaderViewWidget::useEnvelope(REnvelope & e)
{
	empathDebug("useEnvelope()");
	headerList_.clear();
	KConfig * c(KGlobal::config());
	// FIXME Must be QStringList when available.
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	QStrList l;
	c->readListEntry(EmpathConfig::KEY_SHOW_HEADERS, l, ',');
	
	QStrListIterator it(l);
	
	for (; it.current() ; ++it) {
		
		QCString s = it.current();
		s = s.stripWhiteSpace();
		empathDebug("Using header: " + QString::fromLatin1(s));
	
		RHeader * h(e.get(s));
		if (h == 0) continue;
		
		headerList_.append(h->headerName() + ":");
		headerList_.append(h->headerBody()->asString());
	}
	
	int th = QFontMetrics(KGlobal::generalFont()).height();
	setFixedHeight(th * l.count() + 4);
	
	paintEvent(0);
}

	void
EmpathHeaderViewWidget::resizeEvent(QResizeEvent * e)
{
	resized_ = true;

	QWidget::resizeEvent(e);
}
	
	void
EmpathHeaderViewWidget::paintEvent(QPaintEvent * e)
{
	if (!resized_ && e) {
		bitBlt(this, e->rect().topLeft(), &buf_, e->rect());
		return;
	}
	
	resized_ = false;

	buf_.resize(width(), height());

	KPixmap px;
	px.resize(width(), height());
	px.gradientFill(
		kapp->palette().color(QPalette::Normal, QColorGroup::Base),
		kapp->palette().color(QPalette::Normal, QColorGroup::Background));
	
	int th = QFontMetrics(KGlobal::generalFont()).height();
	
	int i(0);
	
	QPainter p;
	p.begin(&buf_);
	
	p.drawPixmap(0, 0, px);
	
	
	p.drawPixmap(width() - 26, 2, clipIcon_);
	
	QRect brect;
	
	int maxWidth(0);
	
	QStrListIterator it(headerList_);
	for (; it.current(); ++it) {
		
		p.drawText(6, i * th + 4, 100, th,
			QPainter::AlignLeft | QPainter::AlignTop | QPainter::SingleLine,
			it.current(),
			-1, &brect);
		
		maxWidth = (maxWidth > brect.width() ? maxWidth : brect.width());
		++it;
		++i;
	}
	
	i = 0;
	it.toFirst();
	
	for (; it.current(); ++it) {

		++it;

		p.drawText(maxWidth + 10, i * th + 4, width() - 30, th,
			QPainter::AlignLeft | QPainter::AlignTop | QPainter::SingleLine,
			it.current(),
			-1, &brect);
		
		++i;
	}
	
	bitBlt(this, 0, 0, &buf_);
	p.end();
}

	void
EmpathHeaderViewWidget::mouseMoveEvent(QMouseEvent * e)
{
	QPainter p;
	p.begin(this);
	
	if (e != 0 && e->x() > width() - 26 && e->x() < width() - 2 &&
		e->y() > 2 && e->y() < 26) {
		
		if (!glowing_) {
			p.drawPixmap(width() - 26, 2, clipGlow_);
			glowing_ = true;
		}

	} else if (glowing_) {
	
		KPixmap px;
		px.resize(30, height());
		px.gradientFill(
			kapp->palette().color(QPalette::Normal, QColorGroup::Base),
			kapp->palette().color(QPalette::Normal, QColorGroup::Background));		
		p.drawPixmap(width() - 30, 0, px);
		p.drawPixmap(width() - 26, 2, clipIcon_);
		glowing_ = false;
	}
	p.end();
}

	void
EmpathHeaderViewWidget::mousePressEvent(QMouseEvent * e)
{
	if (e != 0 && e->x() > width() - 26 && e->x() < width() - 2 &&
		e->y() > 2 && e->y() < 26)
		emit(clipClicked());
}

