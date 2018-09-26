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
	// We have a strange bug when we try to add new labels at the width changed,
	// so this is a workaround for that bug
	_indicators.reserve(15);
	for (int i = 0; i < 15; i++) {
		_indicators.push_back(Indicator(createLabel(), this));
	}

	updateControlsGeometry();
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

int BettergramNumericPageIndicatorWidget::countNeededLabels() const
{
	const QMargins margins = contentsMargins();

	return std::max(1,
					static_cast<int>(floor((width() - margins.left() - margins.right()) / st::bettergramNumericPageIndicatorLabelWidth)));
}

void BettergramNumericPageIndicatorWidget::createLabels()
{
	_indicatorsCount = countNeededLabels();

	if (_indicatorsCount > _indicators.size()) {
		int diff = _indicatorsCount - _indicators.size();

		for (int i = 0; i < diff; ++i) {
			_indicators.push_back(Indicator(createLabel(), this));
		}
	}
}

void BettergramNumericPageIndicatorWidget::fillLabels()
{
	//TODO: bettergram: realize BettergramNumericPageIndicatorWidget::fillLabels()

	if (_indicators.isEmpty()) {
		return;
	}

	if (_indicatorsCount > _indicators.size()) {
		return;
	}

	int left = getMargins().left();

	_indicators.first().setPageIndex(0);
	int top = (height() - _indicators.first().label()->height()) / 2;

	for (int i = 0; i < _indicatorsCount; ++i) {
		Indicator &indicator = _indicators[i];

		indicator.setPageIndex(i);
		indicator.label()->moveToLeft(left, top);

		left += indicator.label()->width();
	}
}

Ui::FlatLabel *BettergramNumericPageIndicatorWidget::createLabel()
{
	return new Ui::FlatLabel(this, st::bettergramNumericPageIndicatorLabel);
}

void BettergramNumericPageIndicatorWidget::resizeEvent(QResizeEvent *e)
{
	updateControlsGeometry();
}

void BettergramNumericPageIndicatorWidget::updateControlsGeometry()
{
	createLabels();
	fillLabels();
}

BettergramNumericPageIndicatorWidget::Indicator::Indicator(Ui::FlatLabel *label,
														   BettergramNumericPageIndicatorWidget *widget) :
	_widget(widget),
	_label(label)
{
}

void BettergramNumericPageIndicatorWidget::Indicator::setPageIndex(int pageIndex)
{
	if (_pageIndex != pageIndex) {
		_pageIndex = pageIndex;
		setLabelText();
	} else if (_label->isEmpty()) {
		setLabelText();
	}
}

ClickHandlerPtr BettergramNumericPageIndicatorWidget::Indicator::getIndicatorClickHandler()
{
	return std::make_shared<LambdaClickHandler>([this] { click(); });
}

void BettergramNumericPageIndicatorWidget::Indicator::click()
{
	if (_widget) {
		_widget->setCurrentPage(_pageIndex);
	}
}

void BettergramNumericPageIndicatorWidget::Indicator::setLabelText()
{
	if (_pageIndex < 0) {
		_label->setRichText(QStringLiteral("..."));
	} else {
		_label->setRichText(textcmdLink(1, QString::number(_pageIndex)));
		_label->setLink(1, getIndicatorClickHandler());
	}
}

} // namespace ChatHelpers
