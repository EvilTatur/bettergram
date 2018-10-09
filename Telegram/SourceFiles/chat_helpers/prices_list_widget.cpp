#include "prices_list_widget.h"
#include "table_column_header_widget.h"
#include "bettergram_numeric_page_indicator_widget.h"

#include <bettergram/bettergramservice.h>
#include <bettergram/cryptopricelist.h>
#include <bettergram/cryptoprice.h>

#include <ui/widgets/buttons.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/input_fields.h>
#include <ui/text/text_helper.h>
#include <lang/lang_keys.h>
#include <styles/style_window.h>
#include <styles/style_dialogs.h>
#include <core/click_handler_types.h>
#include <styles/style_chat_helpers.h>
#include <styles/style_widgets.h>

#include <QMouseEvent>

namespace ChatHelpers {

using namespace Bettergram;

class PricesListWidget::Footer : public TabbedSelector::InnerFooter
{
public:
	Footer(not_null<PricesListWidget*> parent);

protected:
	void processPanelHideFinished() override;
	void resizeEvent(QResizeEvent* e) override;
	void onFooterClicked();

private:
	not_null<PricesListWidget*> _parent;
	object_ptr<Ui::LinkButton> _link;
};

PricesListWidget::Footer::Footer(not_null<PricesListWidget*> parent)
	:  InnerFooter(parent)
	, _parent(parent)
	, _link(object_ptr<Ui::LinkButton>(this, lang(lng_prices_footer), st::largeLinkButton))
{
	//TODO: bettergram: if a user is paid then we must to hide the "Upgrade Now" text

	_link->setClickedCallback([this] { onFooterClicked(); });
}

void PricesListWidget::Footer::resizeEvent(QResizeEvent* e)
{
	_link->move(rect().center() - _link->rect().center());
}

void PricesListWidget::Footer::processPanelHideFinished()
{
}

void PricesListWidget::Footer::onFooterClicked()
{
	BettergramService::openUrl(QUrl("https://www.livecoinwatch.com"));
}

PricesListWidget::PricesListWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: Inner(parent, controller)
{
	_lastUpdateLabel = new Ui::FlatLabel(this, st::pricesPanLastUpdateLabel);

	_siteName = new Ui::IconButton(this, st::pricesPanSiteNameIcon);
	_siteName->setClickedCallback([] { BettergramService::openUrl(QUrl("https://www.livecoinwatch.com")); });

	_marketCap = new Ui::FlatLabel(this, st::pricesPanMarketCapLabel);
	_marketCap->setRichText(textcmdLink(1, lang(lng_prices_market_cap)));
	_marketCap->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	_marketCapValue = new Ui::FlatLabel(this, st::pricesPanMarketCapValueLabel);
	_marketCapValue->setRichText(textcmdLink(1, BettergramService::instance()->cryptoPriceList()->marketCapString()));
	_marketCapValue->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	_btcDominance = new Ui::FlatLabel(this, st::pricesPanMarketCapLabel);
	_btcDominance->setRichText(textcmdLink(1, lang(lng_prices_btc_dominance)));
	_btcDominance->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	_btcDominanceValue = new Ui::FlatLabel(this, st::pricesPanMarketCapValueLabel);
	_btcDominanceValue->setRichText(textcmdLink(1, BettergramService::instance()->cryptoPriceList()->btcDominanceString()));
	_btcDominanceValue->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	_filterTextEdit = new Ui::FlatInput(this, st::dialogsFilter, langFactory(lng_dlg_filter));

	connect(_filterTextEdit, &Ui::FlatInput::changed,
			this, &PricesListWidget::onFilterTextChanged);

	connect(_filterTextEdit, &Ui::FlatInput::cancelled,
			this, &PricesListWidget::onCancelFilter);

	_cancelFilterButton = new Ui::CrossButton(this, st::dialogsCancelSearch);
	_cancelFilterButton->setClickedCallback([this] { onCancelFilter(); });

	_favoriteButton = new Ui::IconButton(this, st::pricesPanFavoriteButton);
	_favoriteButton->setClickedCallback([this] { onFavoriteButtonClicked(); });

	_yourFavoriteListIsEmpty = new Ui::FlatLabel(this, st::pricesPanYourFavoriteListIsEmptyLabel);
	_yourFavoriteListIsEmpty->setRichText(lang(lng_prices_your_favorite_list_is_empty));
	_yourFavoriteListIsEmpty->setVisible(false);

	_pageIndicator = new BettergramNumericPageIndicatorWidget(1, 0, this);

	connect(_pageIndicator, &BettergramNumericPageIndicatorWidget::currentPageChanged,
			this, &PricesListWidget::onCurrentPageChanged);

	_coinHeader = new TableColumnHeaderWidget(this);
	_coinHeader->setText(lng_prices_header_coin);
	_coinHeader->setTextFlags(Qt::AlignLeft | Qt::AlignVCenter);

	_priceHeader = new TableColumnHeaderWidget(this);
	_priceHeader->setText(lng_prices_header_price);
	_priceHeader->setTextFlags(Qt::AlignRight | Qt::AlignVCenter);

	_24hHeader = new TableColumnHeaderWidget(this);
	_24hHeader->setText(lng_prices_header_24h);
	_24hHeader->setTextFlags(Qt::AlignRight | Qt::AlignVCenter);

	connect(_coinHeader, &TableColumnHeaderWidget::sortOrderChanged,
			this, &PricesListWidget::onCoinColumnSortOrderChanged);

	connect(_priceHeader, &TableColumnHeaderWidget::sortOrderChanged,
			this, &PricesListWidget::onPriceColumnSortOrderChanged);

	connect(_24hHeader, &TableColumnHeaderWidget::sortOrderChanged,
			this, &PricesListWidget::on24hColumnSortOrderChanged);

	updateControlsGeometry();
	updateLastUpdateLabel();

	CryptoPriceList *priceList = BettergramService::instance()->cryptoPriceList();

	connect(priceList, &CryptoPriceList::namesUpdated,
			this, &PricesListWidget::onCryptoPriceNamesUpdated);

	connect(priceList, &CryptoPriceList::valuesUpdated,
			this, &PricesListWidget::onCryptoPriceValuesUpdated);

	connect(priceList, &CryptoPriceList::sortOrderChanged,
			this, &PricesListWidget::onCryptoPriceSortOrderChanged);

	connect(priceList, &CryptoPriceList::isShowOnlyFavoritesChanged,
			this, &PricesListWidget::onIsShowOnlyFavoritesChanged);

	setMouseTracking(true);

	onIsShowOnlyFavoritesChanged();
}

void PricesListWidget::getCryptoPriceValues()
{
	BettergramService *service = BettergramService::instance();
	CryptoPriceList *priceList = BettergramService::instance()->cryptoPriceList();

	if (priceList->isShowOnlyFavorites()) {
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(getCurrentShortNames());

		return;
	}

	switch (priceList->sortOrder()) {
	case(CryptoPriceList::SortOrder::Rank):
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(startRowIndexInCurrentPage(), _numberOfRowsInOnePage);
		break;
	case(CryptoPriceList::SortOrder::NameAscending):
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(getCurrentShortNames());
		break;
	case(CryptoPriceList::SortOrder::NameDescending):
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(getCurrentShortNames());
		break;
	case(CryptoPriceList::SortOrder::PriceAscending):
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(startRowIndexInCurrentPage(), _numberOfRowsInOnePage);
		break;
	case(CryptoPriceList::SortOrder::PriceDescending):
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(startRowIndexInCurrentPage(), _numberOfRowsInOnePage);
		break;
	case(CryptoPriceList::SortOrder::ChangeFor24hAscending):
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(startRowIndexInCurrentPage(), _numberOfRowsInOnePage);
		break;
	case(CryptoPriceList::SortOrder::ChangeFor24hDescending):
		_urlForFetchingCurrentPage =
				service->getCryptoPriceValues(startRowIndexInCurrentPage(), _numberOfRowsInOnePage);
		break;
	default:
		LOG(("Can not recognize sort order value %1")
			.arg(static_cast<int>(priceList->sortOrder())));
	}
}

int PricesListWidget::startRowIndexInCurrentPage() const
{
	return qMax(0, qMin(BettergramService::instance()->cryptoPriceList()->count() - 1,
						_pageIndicator->currentPage() * _numberOfRowsInOnePage));
}

QStringList PricesListWidget::getCurrentShortNames() const
{
	return BettergramService::instance()->cryptoPriceList()->getShortNames(startRowIndexInCurrentPage(),
																		   _numberOfRowsInOnePage);
}

void PricesListWidget::refreshRecent()
{
}

void PricesListWidget::clearSelection()
{
}

object_ptr<TabbedSelector::InnerFooter> PricesListWidget::createFooter()
{
	Expects(_footer == nullptr);

	auto res = object_ptr<Footer>(this);

	_footer = res;
	return std::move(res);
}

void PricesListWidget::afterShown()
{
	startPriceListTimer();
	getCryptoPriceValues();
}

void PricesListWidget::beforeHiding()
{
	stopPriceListTimer();
}

void PricesListWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timerId) {
		getCryptoPriceValues();
	}
}

void PricesListWidget::startPriceListTimer()
{
	if (!_timerId) {
		_timerId = startTimer(BettergramService::instance()->cryptoPriceList()->freq() * 1000);

		if (!_timerId) {
			LOG(("Can not start timer for %1 ms")
				.arg(BettergramService::instance()->cryptoPriceList()->freq()));
		}
	}
}

void PricesListWidget::stopPriceListTimer()
{
	if (_timerId) {
		killTimer(_timerId);
		_timerId = 0;
	}
}

void PricesListWidget::setSelectedRow(int selectedRow)
{
	if (_selectedRow != selectedRow) {
		_selectedRow = selectedRow;

		if (_selectedRow >= 0) {
			setCursor(style::cur_pointer);
		} else {
			setCursor(style::cur_default);
		}

		update();
	}
}

void PricesListWidget::setNumberOfRowsInOnePage(int numberOfRowsInOnePage)
{
	if (_numberOfRowsInOnePage != numberOfRowsInOnePage) {
		_numberOfRowsInOnePage = numberOfRowsInOnePage;

		getCryptoPriceValues();
		update();
	}
}

int PricesListWidget::getTableTop() const
{
	return _pageIndicator->y() + _pageIndicator->height() + st::pricesPanPadding;
}

int PricesListWidget::getTableBottom() const
{
	return getTableContentTop() + getTableContentHeight();
}

int PricesListWidget::getTableContentTop() const
{
	return getTableTop() + st::pricesPanTableHeaderHeight;
}

int PricesListWidget::getTableContentHeight() const
{
	return parentWidget() ? parentWidget()->height() - getTableContentTop() : 0;
}

int PricesListWidget::getRowTop(int row) const
{
	return getTableContentTop() + row * st::pricesPanTableRowHeight;
}

QRect PricesListWidget::getTableRectangle() const
{
	return QRect(0,
				 getTableTop(),
				 width(),
				 getTableBottom());
}

QRect PricesListWidget::getTableHeaderRectangle() const
{
	return QRect(0,
				 getTableTop(),
				 width(),
				 st::pricesPanTableHeaderHeight);
}

QRect PricesListWidget::getTableContentRectangle() const
{
	return QRect(0,
				 getTableContentTop(),
				 width(),
				 getTableContentHeight());
}

QRect PricesListWidget::getRowRectangle(int row) const
{
	return QRect(0,
				 getRowTop(row),
				 width(),
				 st::pricesPanTableRowHeight);
}

void PricesListWidget::countSelectedRow(const QPoint &point)
{
	if (_selectedRow == -1) {
		if (!getTableContentRectangle().contains(point)) {
			// Nothing changed
			return;
		}
	} else if (getRowRectangle(_selectedRow).contains(point)) {
		// Nothing changed
		return;
	}

	if (!getTableContentRectangle().contains(point)) {
		setSelectedRow(-1);
		return;
	}

	for (int row = 0; row < _pricesAtCurrentPage.count(); row++) {
		if (getRowRectangle(row).contains(point)) {
			setSelectedRow(row);
			return;
		}
	}

	setSelectedRow(-1);
}

bool PricesListWidget::isInFavoritesColumn(const QPoint &point)
{
	return QRect(_coinHeader->x() + _coinHeader->contentsMargins().left(),
				 getTableContentTop(),
				 st::pricesPanTableFavoriteImageSize,
				 getTableContentHeight())
			.contains(point);
}

TabbedSelector::InnerFooter* PricesListWidget::getFooter() const
{
	return _footer;
}

int PricesListWidget::countDesiredHeight(int newWidth)
{
	Q_UNUSED(newWidth);

	return getTableBottom();
}

void PricesListWidget::mousePressEvent(QMouseEvent *e)
{
	countSelectedRow(e->pos());

	if (isInFavoritesColumn(e->pos())) {
		_pressedFavoriteIcon = _selectedRow;
		_pressedRow = -1;
	} else {
		_pressedRow = _selectedRow;
		_pressedFavoriteIcon = -1;
	}
}

void PricesListWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton) {
		return;
	}

	countSelectedRow(e->pos());

	if (_pressedFavoriteIcon >= 0
			&& _pressedFavoriteIcon < _pricesAtCurrentPage.count()
			&& _pressedFavoriteIcon == _selectedRow) {
		_pricesAtCurrentPage.at(_pressedFavoriteIcon)->toggleIsFavorite();

		if (BettergramService::instance()->cryptoPriceList()->isShowOnlyFavorites()) {
			updatePagesCount();
			getCryptoPriceValues();
			update();
		} else {
			update(getRowRectangle(_selectedRow));
		}

		return;
	}

	if (_pressedRow >= 0
			&& _pressedRow < _pricesAtCurrentPage.count()
			&& _pressedRow == _selectedRow) {
		QUrl url = _pricesAtCurrentPage.at(_pressedRow)->url();

		if (!url.isEmpty()) {
			BettergramService::openUrl(url);
		}
	}
}

void PricesListWidget::mouseMoveEvent(QMouseEvent *e)
{
	countSelectedRow(e->pos());

	if (isInFavoritesColumn(e->pos())) {
		if (!_isHoveredFavoriteIcon) {
			_isHoveredFavoriteIcon = true;
			update(getRowRectangle(_selectedRow));
		}
	} else {
		if (_isHoveredFavoriteIcon) {
			_isHoveredFavoriteIcon = false;
			update(getRowRectangle(_selectedRow));
		}
	}
}

void PricesListWidget::enterEventHook(QEvent *e)
{
	QPoint point = mapFromGlobal(QCursor::pos());
	countSelectedRow(point);
}

void PricesListWidget::leaveEventHook(QEvent *e)
{
	Q_UNUSED(e);

	setSelectedRow(-1);
	_isHoveredFavoriteIcon = false;
}

void PricesListWidget::paintEvent(QPaintEvent *event) {
	Painter painter(this);
	QRect r = event ? event->rect() : rect();

	if (r != rect()) {
		painter.setClipRect(r);
	}

	painter.fillRect(r, st::pricesPanBg);

	int top = getTableContentTop();

	int columnCoinWidth = _coinHeader->width()
			- _coinHeader->contentsMargins().left()
			- _coinHeader->contentsMargins().right()
			- st::pricesPanTableFavoriteImageSize
			- st::pricesPanTableImageSize
			- st::pricesPanTablePadding;

	int columnPriceWidth = _priceHeader->width()
			- _priceHeader->contentsMargins().left()
			- _priceHeader->contentsMargins().right();

	int column24hWidth = _24hHeader->width()
			- _24hHeader->contentsMargins().left()
			- _24hHeader->contentsMargins().right();

	int columnCoinLeft = _coinHeader->x() + _coinHeader->contentsMargins().left();
	int columnPriceLeft = _priceHeader->x() + _priceHeader->contentsMargins().left();
	int column24hLeft = _24hHeader->x() + _24hHeader->contentsMargins().left();

	int columnCoinIconLeft = columnCoinLeft
			+ st::pricesPanTableFavoriteImageSize
			+ st::pricesPanTablePadding / 2;

	int columnCoinTextLeft = columnCoinIconLeft
			+ st::pricesPanTableImageSize
			+ st::pricesPanTablePadding;

	int favoriteButtonHovered = -1;

	// Draw rows

	if (_selectedRow != -1 && _selectedRow < _pricesAtCurrentPage.count()) {
		QRect favoriteRect(columnCoinLeft,
						   top + _selectedRow * st::pricesPanTableRowHeight,
						   st::pricesPanTableFavoriteImageSize,
						   st::pricesPanTableRowHeight);

		if (favoriteRect.contains(mapFromGlobal(QCursor::pos()))) {
			favoriteButtonHovered = _selectedRow;
		}

		QRect rowRectangle(0, getRowTop(_selectedRow), width(), st::pricesPanTableRowHeight);
		App::roundRect(painter, rowRectangle, st::pricesPanHover, StickerHoverCorners);
	}

	painter.setFont(st::semiboldFont);

	for (int i = 0; i < _pricesAtCurrentPage.count(); ++i) {
		const QSharedPointer<CryptoPrice> &price = _pricesAtCurrentPage.at(i);

		const style::icon *favoriteIcon = nullptr;

		if (i == favoriteButtonHovered) {
			if (price->isFavorite()) {
				favoriteIcon = &st::pricesPanFavoriteEnabledIconOver;
			} else {
				favoriteIcon = &st::pricesPanFavoriteDisabledIconOver;
			}
		} else {
			if (price->isFavorite()) {
				favoriteIcon = &st::pricesPanFavoriteEnabledIcon;
			} else {
				favoriteIcon = &st::pricesPanFavoriteDisabledIcon;
			}
		}

		QRect favoriteRect(columnCoinLeft,
						   top + (st::pricesPanTableRowHeight - st::pricesPanTableFavoriteImageSize) / 2,
						   st::pricesPanTableFavoriteImageSize,
						   st::pricesPanTableFavoriteImageSize);

		favoriteIcon->paintInCenter(painter, favoriteRect);

		if (!price->icon().isNull()) {
			QRect targetRect(columnCoinIconLeft,
							 top + (st::pricesPanTableRowHeight - st::pricesPanTableImageSize) / 2,
							 st::pricesPanTableImageSize,
							 st::pricesPanTableImageSize);

			painter.drawPixmap(targetRect, price->icon());
		}

		painter.setPen(st::pricesPanTableCryptoNameFg);

		QRect nameRect(columnCoinTextLeft,
					   top,
					   columnCoinWidth,
					   st::pricesPanTableRowHeight / 2);

		TextHelper::drawElidedText(painter,
								   nameRect,
								   price->name(),
								   Qt::AlignLeft | Qt::AlignBottom);

		painter.setPen(st::pricesPanTableCryptoShortNameFg);

		painter.drawText(columnCoinTextLeft,
						 top + st::pricesPanTableRowHeight / 2,
						 columnCoinWidth,
						 st::pricesPanTableRowHeight / 2,
						 Qt::AlignLeft | Qt::AlignTop,
						 price->shortName());

		switch (price->minuteDirection()) {
		case(CryptoPrice::Direction::Up): {
			painter.setPen(st::pricesPanTableUpFg);
			break;
		}
		case(CryptoPrice::Direction::Down): {
			painter.setPen(st::pricesPanTableDownFg);
			break;
		}
		default: {
			painter.setPen(st::pricesPanTableNoneFg);
		}
		}

		painter.drawText(columnPriceLeft, top, columnPriceWidth, st::pricesPanTableRowHeight,
						 Qt::AlignRight | Qt::AlignVCenter, price->currentPriceString());

		switch (price->dayDirection()) {
		case(CryptoPrice::Direction::Up): {
			painter.setPen(st::pricesPanTableUpFg);
			break;
		}
		case(CryptoPrice::Direction::Down): {
			painter.setPen(st::pricesPanTableDownFg);
			break;
		}
		default: {
			painter.setPen(st::pricesPanTableNoneFg);
		}
		}

		painter.drawText(column24hLeft, top, column24hWidth, st::pricesPanTableRowHeight,
						 Qt::AlignRight | Qt::AlignVCenter, price->changeFor24HoursString());

		top += st::pricesPanTableRowHeight;
	}
}

void PricesListWidget::resizeEvent(QResizeEvent *e)
{
	updateControlsGeometry();
}

void PricesListWidget::updateControlsGeometry()
{
	_lastUpdateLabel->moveToLeft(st::pricesPanPadding, st::pricesPanHeader);

	_siteName->moveToLeft((width() - _siteName->width()) / 2,
						  _lastUpdateLabel->y() + _lastUpdateLabel->height() + st::pricesPanPadding / 2);

	updateMarketCap();
	updateBtcDominance();

	_filterTextEdit->moveToLeft(st::pricesPanPadding,
								_marketCapValue->y() + _marketCapValue->height() + st::pricesPanPadding / 2);

	_favoriteButton->moveToLeft(width() - _favoriteButton->width(),
								_filterTextEdit->y()
								+ (_filterTextEdit->height() - _favoriteButton->height()) / 2);

	_filterTextEdit->resize(_favoriteButton->x() - st::pricesPanPadding - getMargins().left(),
							_filterTextEdit->height());

	_cancelFilterButton->moveToLeft(_filterTextEdit->x()
									+ _filterTextEdit->width()
									- _cancelFilterButton->width()
									- st::pricesPanPadding,
									_filterTextEdit->y());

	updatePagesCount();

	_pageIndicator->moveToLeft(0,
							   _filterTextEdit->y() + _filterTextEdit->height() + st::pricesPanPadding);

	_pageIndicator->resizeToWidth(width());

	int columnCoinWidth = width() - st::pricesPanColumnPriceWidth - st::pricesPanColumn24hWidth;

	_coinHeader->resize(columnCoinWidth, st::pricesPanTableHeaderHeight);
	_coinHeader->setContentsMargins(st::pricesPanTablePadding, 0, 0, 0);

	_priceHeader->resize(st::pricesPanColumnPriceWidth, st::pricesPanTableHeaderHeight);
	_priceHeader->setContentsMargins(0, 0, st::pricesPanTablePadding, 0);

	_24hHeader->resize(st::pricesPanColumn24hWidth, st::pricesPanTableHeaderHeight);
	_24hHeader->setContentsMargins(0, 0, st::pricesPanTablePadding, 0);

	int headerTop = getTableTop();

	_coinHeader->moveToLeft(0, headerTop);
	_priceHeader->moveToLeft(_coinHeader->x() + _coinHeader->width(), headerTop);
	_24hHeader->moveToLeft(_priceHeader->x() + _priceHeader->width(), headerTop);

	_yourFavoriteListIsEmpty->moveToLeft(st::pricesPanPadding,
										 getTableContentTop() + st::pricesPanPadding);

	_yourFavoriteListIsEmpty->resizeToWidth(width());
}

void PricesListWidget::updatePagesCount()
{
	int pricesCount = BettergramService::instance()->cryptoPriceList()->count();

	int numberOfRowsInOnePage = qMax(1, getTableContentHeight() / st::pricesPanTableRowHeight);

	int pagesCount = qMax(1,
						  static_cast<int>(std::ceil(static_cast<double>(pricesCount) / static_cast<double>(numberOfRowsInOnePage))));

	// We have to do it because the number of rows in one page may be changed,
	// but the current page may not. And we need to get new data from servers.

	disconnect(_pageIndicator, &BettergramNumericPageIndicatorWidget::currentPageChanged,
			   this, &PricesListWidget::onCurrentPageChanged);

	_pageIndicator->setPagesCount(pagesCount);

	connect(_pageIndicator, &BettergramNumericPageIndicatorWidget::currentPageChanged,
			this, &PricesListWidget::onCurrentPageChanged);

	setNumberOfRowsInOnePage(numberOfRowsInOnePage);
}

void PricesListWidget::updateLastUpdateLabel()
{
	_lastUpdateLabel->setText(lang(lng_prices_last_update)
							  .arg(BettergramService::instance()->cryptoPriceList()->lastUpdateString()));
}

void PricesListWidget::updateMarketCap()
{
	_marketCapValue->setRichText(textcmdLink(1, BettergramService::instance()->cryptoPriceList()->marketCapString()));
	_marketCapValue->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	int marketCapWidth = qMax(_marketCap->width(), _marketCapValue->width());
	int xOffset = st::pricesPanPadding;

	_marketCap->moveToLeft(xOffset + (marketCapWidth - _marketCap->width()) / 2,
						   _siteName->y() + _siteName->height() + st::pricesPanPadding / 2);

	_marketCapValue->moveToLeft(xOffset + (marketCapWidth - _marketCapValue->width()) / 2,
								_marketCap->y() + _marketCap->height());
}

void PricesListWidget::updateBtcDominance()
{
	_btcDominanceValue->setRichText(textcmdLink(1, BettergramService::instance()->cryptoPriceList()->btcDominanceString()));
	_btcDominanceValue->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	int btcDominanceWidth = qMax(_btcDominance->width(), _btcDominanceValue->width());
	int xOffset = width() - btcDominanceWidth - st::pricesPanPadding;

	_btcDominance->moveToLeft(xOffset + (btcDominanceWidth - _btcDominance->width()) / 2,
							  _marketCap->y());

	_btcDominanceValue->moveToLeft(xOffset + (btcDominanceWidth - _btcDominanceValue->width()) / 2,
								   _marketCapValue->y());
}

void PricesListWidget::onCoinColumnSortOrderChanged()
{
	_priceHeader->resetSortOrder(false);
	_24hHeader->resetSortOrder(false);

	CryptoPriceList::SortOrder sortOrder = CryptoPriceList::SortOrder::Rank;

	switch (_coinHeader->sortOrder())
	{
	case (TableColumnHeaderWidget::SortOrder::None):
		sortOrder = CryptoPriceList::SortOrder::Rank;
		break;
	case (TableColumnHeaderWidget::SortOrder::Ascending):
		sortOrder = CryptoPriceList::SortOrder::NameAscending;
		break;
	case (TableColumnHeaderWidget::SortOrder::Descending):
		sortOrder = CryptoPriceList::SortOrder::NameDescending;
		break;
	default:
		sortOrder = CryptoPriceList::SortOrder::Rank;
		LOG(("Can not recognize sort order %1").arg(static_cast<int>(_coinHeader->sortOrder())));
		break;
	}

	BettergramService::instance()->cryptoPriceList()->setSortOrder(sortOrder);
}

void PricesListWidget::onPriceColumnSortOrderChanged()
{
	_coinHeader->resetSortOrder(false);
	_24hHeader->resetSortOrder(false);

	CryptoPriceList::SortOrder sortOrder = CryptoPriceList::SortOrder::Rank;

	switch (_priceHeader->sortOrder())
	{
	case (TableColumnHeaderWidget::SortOrder::None):
		sortOrder = CryptoPriceList::SortOrder::Rank;
		break;
	case (TableColumnHeaderWidget::SortOrder::Ascending):
		sortOrder = CryptoPriceList::SortOrder::PriceAscending;
		break;
	case (TableColumnHeaderWidget::SortOrder::Descending):
		sortOrder = CryptoPriceList::SortOrder::PriceDescending;
		break;
	default:
		sortOrder = CryptoPriceList::SortOrder::Rank;
		LOG(("Can not recognize sort order %1").arg(static_cast<int>(_priceHeader->sortOrder())));
		break;
	}

	BettergramService::instance()->cryptoPriceList()->setSortOrder(sortOrder);
}

void PricesListWidget::on24hColumnSortOrderChanged()
{
	_coinHeader->resetSortOrder(false);
	_priceHeader->resetSortOrder(false);

	CryptoPriceList::SortOrder sortOrder = CryptoPriceList::SortOrder::Rank;

	switch (_24hHeader->sortOrder())
	{
	case (TableColumnHeaderWidget::SortOrder::None):
		sortOrder = CryptoPriceList::SortOrder::Rank;
		break;
	case (TableColumnHeaderWidget::SortOrder::Ascending):
		sortOrder = CryptoPriceList::SortOrder::ChangeFor24hAscending;
		break;
	case (TableColumnHeaderWidget::SortOrder::Descending):
		sortOrder = CryptoPriceList::SortOrder::ChangeFor24hDescending;
		break;
	default:
		sortOrder = CryptoPriceList::SortOrder::Rank;
		LOG(("Can not recognize sort order %1").arg(static_cast<int>(_24hHeader->sortOrder())));
		break;
	}

	BettergramService::instance()->cryptoPriceList()->setSortOrder(sortOrder);
}

void PricesListWidget::onCryptoPriceNamesUpdated()
{
	getCryptoPriceValues();
}

void PricesListWidget::onCryptoPriceValuesUpdated(const QUrl &url,
												  const QList<QSharedPointer<CryptoPrice>> &prices)
{
	if (_urlForFetchingCurrentPage == url) {
		_pricesAtCurrentPage = prices;
	}

	updateLastUpdateLabel();
	updateMarketCap();
	updateBtcDominance();
	update();

	CryptoPriceList *cryptoPriceList = BettergramService::instance()->cryptoPriceList();

	_yourFavoriteListIsEmpty->setVisible(cryptoPriceList->isShowOnlyFavorites()
										 && cryptoPriceList->favoriteList().isEmpty());
}

void PricesListWidget::onCryptoPriceSortOrderChanged()
{
	getCryptoPriceValues();
	update();
}

void PricesListWidget::onCurrentPageChanged()
{
	getCryptoPriceValues();
	update();
}

void PricesListWidget::onFilterTextChanged()
{
	_cancelFilterButton->toggle(!_filterTextEdit->text().isEmpty(), anim::type::normal);

	BettergramService::instance()->cryptoPriceList()->setFilterText(_filterTextEdit->getLastText());
}

void PricesListWidget::onCancelFilter()
{
	_filterTextEdit->clear();
	_filterTextEdit->updatePlaceholder();
	_cancelFilterButton->toggle(false, anim::type::normal);
}

void PricesListWidget::onFavoriteButtonClicked()
{
	BettergramService::instance()->cryptoPriceList()->toggleIsShowOnlyFavorites();
}

void PricesListWidget::onIsShowOnlyFavoritesChanged()
{
	CryptoPriceList *cryptoPriceList = BettergramService::instance()->cryptoPriceList();

	if (cryptoPriceList->isShowOnlyFavorites()) {
		_favoriteButton->setIconOverride(&st::pricesPanFavoriteEnabledIcon,
										 &st::pricesPanFavoriteEnabledIconOver);
	} else {
		_favoriteButton->setIconOverride(nullptr, nullptr);
	}

	updatePagesCount();
	getCryptoPriceValues();

	_pageIndicator->setCurrentPage(0);

	_yourFavoriteListIsEmpty->setVisible(cryptoPriceList->isShowOnlyFavorites()
										 && cryptoPriceList->favoriteList().isEmpty());
}

} // namespace ChatHelpers
