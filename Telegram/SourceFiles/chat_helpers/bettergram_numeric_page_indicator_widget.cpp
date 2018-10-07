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

		if (_currentPage >= _pagesCount) {
			setCurrentPage(_pagesCount - 1);
		} else if (_currentPage < 0 && _pagesCount > 0) {
			setCurrentPage(0);
		} else {
			updateControlsGeometry();
		}

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
			setHoveredPage(_indicators[i].pageIndex());
			return;
		}
	}

	setHoveredPage(-1);
}

void BettergramNumericPageIndicatorWidget::countFitPages()
{
	const QMargins margins = contentsMargins();

	_fitPages = std::max(1,
						 static_cast<int>(floor((width() - margins.left() - margins.right()) / st::bettergramNumericPageIndicatorLabelWidth)));
}

void BettergramNumericPageIndicatorWidget::createIndicators()
{
	countFitPages();

	int count = qMin(_fitPages, _pagesCount);

	if ((count < _pagesCount) && (_currentPage >= count - 3) && (count > 7) && (count % 2 == 0)) {
		count--;
	}

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
	if (_indicators.isEmpty()) {
		return;
	}

	int left = getMargins().left();
	int right = getMargins().right();

	left = (width() - (_indicators.size() * st::bettergramNumericPageIndicatorLabelWidth)) / 2;

	for (int i = 0; i < _indicators.size(); ++i) {
		_indicators[i].setLeft(left);
		left += st::bettergramNumericPageIndicatorLabelWidth;
	}

	if (_indicators.size() == _pagesCount) {
		fillIndicators(_indicators.size());
		return;
	}

	if (_currentPage < _indicators.size() - 3) {
		fillIndicators(_indicators.size() - 2);

		_indicators[_indicators.size() - 2].setPageIndex(-1);
		_indicators[_indicators.size() - 1].setPageIndex(_pagesCount - 1);

		return;
	}

	if (_indicators.size() >= 7) {
		_indicators[0].setPageIndex(0);
		_indicators[1].setPageIndex(-1);

		_indicators[_indicators.size() - 1].setPageIndex(_pagesCount - 1);

		const int delta = _indicators.size() - 4;
		int startPageIndex = _currentPage - (delta / 2);

		if (startPageIndex + delta >= _pagesCount - 2) {
			for (int i = 2; i < _indicators.size() - 1; ++i) {
				_indicators[i].setPageIndex(_pagesCount - _indicators.size() + i);
			}
		} else {
			_indicators[_indicators.size() - 2].setPageIndex(-1);

			for (int i = 0; i < delta; ++i) {
				_indicators[2 + i].setPageIndex(startPageIndex + i);
			}
		}

	} else if (_indicators.size() == 6) {
		if (_currentPage > 0) {
			if (_currentPage + 1 < _pagesCount) {
				if (_currentPage == _pagesCount - 2 || _currentPage == _pagesCount - 3) {
					_indicators[0].setPageIndex(0);
					_indicators[1].setPageIndex(-1);
					_indicators[2].setPageIndex(_pagesCount - 4);
					_indicators[3].setPageIndex(_pagesCount - 3);
					_indicators[4].setPageIndex(_pagesCount - 2);
					_indicators[5].setPageIndex(_pagesCount - 1);
				} else if (_currentPage > 1) {
					_indicators[0].setPageIndex(0);

					if (_currentPage == 3) {
						_indicators[1].setPageIndex(1);
					} else {
						_indicators[1].setPageIndex(-1);
					}

					_indicators[2].setPageIndex(_currentPage - 1);
					_indicators[3].setPageIndex(_currentPage);
					_indicators[4].setPageIndex(_currentPage + 1);
					_indicators[5].setPageIndex(_pagesCount - 1);
				} else {
					// Current page is 1
					_indicators[0].setPageIndex(0);
					_indicators[1].setPageIndex(1);
					_indicators[2].setPageIndex(2);
					_indicators[3].setPageIndex(3);
					_indicators[4].setPageIndex(-1);
					_indicators[5].setPageIndex(_pagesCount - 1);
				}
			} else {
				// Current page is (_pagesCount - 1)
				_indicators[0].setPageIndex(0);
				_indicators[1].setPageIndex(1);
				_indicators[2].setPageIndex(2);
				_indicators[3].setPageIndex(-1);
				_indicators[4].setPageIndex(_pagesCount - 2);
				_indicators[5].setPageIndex(_pagesCount - 1);
			}
		} else {
			// Current page is 0
			_indicators[0].setPageIndex(0);
			_indicators[1].setPageIndex(1);
			_indicators[2].setPageIndex(2);
			_indicators[3].setPageIndex(3);
			_indicators[4].setPageIndex(-1);
			_indicators[5].setPageIndex(_pagesCount - 1);
		}
	} else if (_indicators.size() == 5) {
		if (_currentPage > 0) {
			if (_currentPage + 1 < _pagesCount) {
				if (_currentPage > 1) {
					_indicators[0].setPageIndex(0);

					if (_currentPage == 3) {
						_indicators[1].setPageIndex(1);
					} else {
						_indicators[1].setPageIndex(-1);
					}

					_indicators[2].setPageIndex(_currentPage - 1);
					_indicators[3].setPageIndex(_currentPage);
					_indicators[4].setPageIndex(_currentPage + 1);
				} else {
					// Current page is 1
					_indicators[0].setPageIndex(0);
					_indicators[1].setPageIndex(1);
					_indicators[2].setPageIndex(2);
					_indicators[3].setPageIndex(-1);
					_indicators[4].setPageIndex(_pagesCount - 1);
				}
			} else {
				// Current page is (_pagesCount - 1)
				_indicators[0].setPageIndex(0);
				_indicators[1].setPageIndex(1);
				_indicators[2].setPageIndex(-1);
				_indicators[3].setPageIndex(_pagesCount - 2);
				_indicators[4].setPageIndex(_pagesCount - 1);
			}
		} else {
			// Current page is 0
			_indicators[0].setPageIndex(0);
			_indicators[1].setPageIndex(1);
			_indicators[2].setPageIndex(2);
			_indicators[3].setPageIndex(-1);
			_indicators[4].setPageIndex(_pagesCount - 1);
		}
	} else if (_indicators.size() == 4) {
		if (_currentPage > 0) {
			if (_currentPage + 1 < _pagesCount) {
				if (_currentPage > 1) {
					_indicators[0].setPageIndex(0);
					_indicators[1].setPageIndex(_currentPage - 1);
					_indicators[2].setPageIndex(_currentPage);
					_indicators[3].setPageIndex(_currentPage + 1);
				} else {
					// Current page is 1
					_indicators[0].setPageIndex(0);
					_indicators[1].setPageIndex(1);
					_indicators[2].setPageIndex(2);
					_indicators[3].setPageIndex(_pagesCount - 1);
				}
			} else {
				// Current page is (_pagesCount - 1)
				_indicators[0].setPageIndex(0);
				_indicators[1].setPageIndex(-1);
				_indicators[2].setPageIndex(_pagesCount - 2);
				_indicators[3].setPageIndex(_pagesCount - 1);
			}
		} else {
			// Current page is 0
			_indicators[0].setPageIndex(0);
			_indicators[1].setPageIndex(1);
			_indicators[2].setPageIndex(-1);
			_indicators[3].setPageIndex(_pagesCount - 1);
		}
	} else if (_indicators.size() == 3) {
		if (_currentPage > 0) {
			if (_currentPage + 1 < _pagesCount) {
				_indicators[0].setPageIndex(_currentPage - 1);
				_indicators[1].setPageIndex(_currentPage);
				_indicators[2].setPageIndex(_currentPage + 1);
			} else {
				// Current page is (_pagesCount - 1)
				_indicators[0].setPageIndex(0);
				_indicators[1].setPageIndex(_pagesCount - 2);
				_indicators[2].setPageIndex(_pagesCount - 1);
			}
		} else {
			// Current page is 0
			_indicators[0].setPageIndex(0);
			_indicators[1].setPageIndex(1);
			_indicators[2].setPageIndex(_pagesCount - 1);
		}
	} else if (_indicators.size() == 2) {
		// Current page is 0
		_indicators[0].setPageIndex(0);
		_indicators[1].setPageIndex(1);
	} else if (_indicators.size() == 1) {
		_indicators[0].setPageIndex(_currentPage);
	}
}

void BettergramNumericPageIndicatorWidget::fillIndicators(int count)
{
	for (int i = 0; i < count; ++i) {
		_indicators[i].setPageIndex(i);
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
	countHoveredPage(e->pos());
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
	painter.setPen(st::bettergramNumericPageIndicatorLabelFg);

	for (int i = 0; i < _indicators.size(); ++i) {
		const Indicator &indicator = _indicators[i];

		if ((indicator.pageIndex() == _hoveredPage || indicator.pageIndex() == _currentPage)
				&& (indicator.pageIndex() != -1)) {
			QRect indicatorRectangle(indicator.left(),
									 0,
									 st::bettergramNumericPageIndicatorLabelWidth,
									 height());
			App::roundRect(painter,
						   indicatorRectangle,
						   st::bettergramNumericPageIndicatorLabelPanHover,
						   StickerHoverCorners);

			if (indicator.pageIndex() == _currentPage) {
				painter.setPen(st::bettergramNumericPageIndicatorLabelSelectedFg);
			}
		}

		painter.drawText(indicator.left(),
						 0,
						 indicator.width(),
						 height(),
						 Qt::AlignHCenter | Qt::AlignVCenter,
						 indicator.text());

		if (indicator.pageIndex() == _currentPage) {
			painter.setPen(st::bettergramNumericPageIndicatorLabelFg);
		}
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
		_text = QString::number(_pageIndex + 1);
	}
}

} // namespace ChatHelpers
