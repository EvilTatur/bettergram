#pragma once

#include "chat_helpers/tabbed_selector.h"

namespace Bettergram {
class CryptoPrice;
} // namespace Bettergram

namespace Window {
class Controller;
} // namespace Window

namespace Ui {
class FlatLabel;
class FlatInput;
class IconButton;
class CrossButton;
} // namespace Ui

namespace ChatHelpers {

class TableColumnHeaderWidget;
class BettergramNumericPageIndicatorWidget;

/**
 * @brief The PricesListWidget class shows cryptocurrency price list.
 * In normal Qt application we should use QTableView, but it would be strange for this application
 * because it uses low level painting for drawing custom widgets.
 */
class PricesListWidget : public TabbedSelector::Inner
{
	Q_OBJECT

public:
	PricesListWidget(QWidget* parent, not_null<Window::Controller*> controller);

	void refreshRecent() override;
	void clearSelection() override;
	object_ptr<TabbedSelector::InnerFooter> createFooter() override;

	void afterShown() override;
	void beforeHiding() override;

public slots:

//signals:

protected:
	TabbedSelector::InnerFooter* getFooter() const override;
	int countDesiredHeight(int newWidth) override;

	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void enterEventHook(QEvent *e) override;
	void leaveEventHook(QEvent *e) override;

	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *e) override;
	void timerEvent(QTimerEvent *event) override;

private:
	class Footer;

	int _timerId = 0;
	int _selectedRow = -1;
	int _pressedRow = -1;
	int _pressedFavoriteIcon = -1;

	bool _isHoveredFavoriteIcon = false;

	/// Number of rows (crypto prices) at one page.
	/// It is dynamically changed value because we keep one page for one screen, without scrolling,
	/// so when height of the window is changed we also change the number of rows in one page
	int _numberOfRowsInOnePage = 1;

	QUrl _urlForFetchingCurrentPage;
	QList<QSharedPointer<Bettergram::CryptoPrice>> _pricesAtCurrentPage;

	Ui::FlatLabel *_lastUpdateLabel = nullptr;
	Ui::IconButton *_siteName = nullptr;
	Ui::FlatLabel *_marketCap = nullptr;
	Ui::FlatLabel *_marketCapValue = nullptr;
	Ui::FlatLabel *_btcDominance = nullptr;
	Ui::FlatLabel *_btcDominanceValue = nullptr;
	Ui::FlatInput *_filterTextEdit = nullptr;
	Ui::CrossButton *_cancelFilterButton = nullptr;
	Ui::IconButton *_favoriteButton = nullptr;
	BettergramNumericPageIndicatorWidget *_pageIndicator = nullptr;
	TableColumnHeaderWidget *_coinHeader = nullptr;
	TableColumnHeaderWidget *_priceHeader = nullptr;
	TableColumnHeaderWidget *_24hHeader = nullptr;
	Footer *_footer = nullptr;

	void getCryptoPriceValues();

	int startRowIndexInCurrentPage() const;
	QStringList getCurrentShortNames() const;

	void setSelectedRow(int selectedRow);
	void setNumberOfRowsInOnePage(int numberOfRowsInOnePage);

	int getTableTop() const;
	int getTableBottom() const;
	int getTableContentTop() const;
	int getTableContentHeight() const;
	int getRowTop(int row) const;

	QRect getTableRectangle() const;
	QRect getTableHeaderRectangle() const;
	QRect getTableContentRectangle() const;
	QRect getRowRectangle(int row) const;

	void countSelectedRow(const QPoint &point);
	bool isInFavoritesColumn(const QPoint &point);

	void updateControlsGeometry();
	void updatePagesCount();
	void updateLastUpdateLabel();
	void updateMarketCap();
	void updateBtcDominance();

	void startPriceListTimer();
	void stopPriceListTimer();

private slots:
	void onCoinColumnSortOrderChanged();
	void onPriceColumnSortOrderChanged();
	void on24hColumnSortOrderChanged();

	void onCryptoPriceNamesUpdated();

	void onCryptoPriceValuesUpdated(const QUrl &url,
									const QList<QSharedPointer<Bettergram::CryptoPrice>> &prices);

	void onCryptoPriceSortOrderChanged();

	void onCurrentPageChanged();
	void onFilterTextChanged();
	void onCancelFilter();
	void onFavoriteButtonClicked();
	void onIsShowOnlyFavoritesChanged();
};

} // namespace ChatHelpers
