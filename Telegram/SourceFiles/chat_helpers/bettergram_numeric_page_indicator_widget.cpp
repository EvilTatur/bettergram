#include "bettergram_numeric_page_indicator_widget.h"

#include <ui/widgets/labels.h>
#include <styles/style_chat_helpers.h>
#include <core/click_handler.h>
#include <lang/lang_keys.h>

namespace ChatHelpers {

BettergramNumericPageIndicatorWidget::BettergramNumericPageIndicatorWidget(int pagesCount,
																		   int currentPage,
																		   QWidget* parent)
	: TWidget(parent),
	  _pagesCount(std::max(1, pagesCount)),
	  _currentPage(currentPage)
{
	resize(width(), st::bettergramNumericPageIndicatorHeight);
	updateControlsGeometry();

	setCursor(style::cur_pointer);
	setMouseTracking(true);
}

int BettergramNumericPageIndicatorWidget::pagesCount() const
{
	return _pagesCount;
}

void BettergramNumericPageIndicatorWidget::setPagesCount(int pagesCount)
{
	int effectivePagesCount = std::max(1, pagesCount);

	if (_pagesCount != effectivePagesCount) {
		_pagesCount = effectivePagesCount;
		emit pagesCountChanged();

		updateControlsGeometry();
	}
}

int BettergramNumericPageIndicatorWidget::currentPage() const
{
	return _currentPage;
}

void BettergramNumericPageIndicatorWidget::setCurrentPage(int currentPage)
{
	if (_currentPage != currentPage) {
		_currentPage = currentPage;
		emit currentPageChanged();

		updateControlsGeometry();
	}
}

void BettergramNumericPageIndicatorWidget::setHoveredPage(int hoveredPage)
{
	if (_hoveredPage != hoveredPage) {
		_hoveredPage = hoveredPage;
		update();
	}
}

void BettergramNumericPageIndicatorWidget::countHoveredPage(const QPoint &position)
{
	for (int i = 0; i < _indicators.size(); ++i) {
		if (_indicators[i].rect().contains(position)) {
			setHoveredPage(i);
			return;
		}
	}

	setHoveredPage(-1);
}

int BettergramNumericPageIndicatorWidget::countNeededIndicators() const
{
	const QMargins margins = contentsMargins();

	return std::max(1,
					static_cast<int>(floor((width() - margins.left() - margins.right()) / st::bettergramNumericPageIndicatorLabelWidth)));
}

void BettergramNumericPageIndicatorWidget::createIndicators()
{
	int count = countNeededIndicators();

	if (count > _indicators.size()) {
		int diff = count - _indicators.size();

		for (int i = 0; i < diff; ++i) {
			_indicators.push_back(Indicator());
		}
	} else if (count < _indicators.size()) {
		int diff = _indicators.size() - count;

		for (int i = 0; i < diff; ++i) {
			_indicators.removeLast();
		}
	}
}

void BettergramNumericPageIndicatorWidget::fillIndicators()
{
	//TODO: bettergram: realize BettergramNumericPageIndicatorWidget::fillLabels()

	if (_indicators.isEmpty()) {
		return;
	}

	int left = getMargins().left();
	int right = getMargins().right();

	left = (width() - (_indicators.size() * st::bettergramNumericPageIndicatorLabelWidth)) / 2;

	for (int i = 0; i < _indicators.size(); ++i) {
		Indicator &indicator = _indicators[i];

		indicator.setLeft(left);
		indicator.setPageIndex(i);

		left += indicator.width();
	}
}

void BettergramNumericPageIndicatorWidget::mousePressEvent(QMouseEvent *e)
{
	_pressedPage = _hoveredPage;
}

void BettergramNumericPageIndicatorWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (_pressedPage < 0 || (_pressedPage != _hoveredPage)) {
		return;
	}

	setCurrentPage(_pressedPage);
}

void BettergramNumericPageIndicatorWidget::mouseMoveEvent(QMouseEvent *e)
{
	countHoveredPage(e->pos());
}

void BettergramNumericPageIndicatorWidget::enterEventHook(QEvent *e)
{
	countHoveredPage(mapFromGlobal(QCursor::pos()));
}

void BettergramNumericPageIndicatorWidget::leaveEventHook(QEvent *e)
{
	_hoveredPage = -1;
	_pressedPage = -1;

	update();
}

void BettergramNumericPageIndicatorWidget::resizeEvent(QResizeEvent *e)
{
	updateControlsGeometry();
}

void BettergramNumericPageIndicatorWidget::paintEvent(QPaintEvent *event)
{
	Painter painter(this);
	QRect r = event ? event->rect() : rect();

	if (r != rect()) {
		painter.setClipRect(r);
	}

	painter.fillRect(r, st::bettergramNumericPageIndicatorBg);

	painter.setFont(st::semiboldFont);
	painter.setPen(st::tableHeaderFg);

	for (int i = 0; i < _indicators.size(); ++i) {
		const Indicator &indicator = _indicators[i];

		if (i == _hoveredPage || i == _currentPage) {
			QRect indicatorRectangle(indicator.left(),
									 0,
									 st::bettergramNumericPageIndicatorLabelWidth,
									 height());
			App::roundRect(painter,
						   indicatorRectangle,
						   st::bettergramNumericPageIndicatorLabelPanHover,
						   StickerHoverCorners);
		}

		painter.drawText(indicator.left(),
						 0,
						 indicator.width(),
						 height(),
						 Qt::AlignHCenter | Qt::AlignVCenter,
						 indicator.text());

	}
}

void BettergramNumericPageIndicatorWidget::updateControlsGeometry()
{
	createIndicators();
	fillIndicators();
	update();
}

void BettergramNumericPageIndicatorWidget::Indicator::setLeft(int left)
{
	_left = left;
}

int BettergramNumericPageIndicatorWidget::Indicator::width() const
{
	return st::bettergramNumericPageIndicatorLabelWidth;
}

QRect BettergramNumericPageIndicatorWidget::Indicator::rect() const
{
	return QRect(left(), 0, width(), st::bettergramNumericPageIndicatorHeight);
}

void BettergramNumericPageIndicatorWidget::Indicator::setPageIndex(int pageIndex)
{
	if (_pageIndex != pageIndex) {
		_pageIndex = pageIndex;
		setText();
	} else if (_text.isEmpty()) {
		setText();
	}
}

void BettergramNumericPageIndicatorWidget::Indicator::setText()
{
	if (_pageIndex < 0) {
		_text = QStringLiteral("...");
	} else {
		_text = QString::number(_pageIndex);
	}
}

} // namespace ChatHelpers
