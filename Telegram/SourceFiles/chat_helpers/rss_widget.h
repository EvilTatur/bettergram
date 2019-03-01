#pragma once

#include "tabbed_selector.h"
#include "list_row_array.h"

namespace Window {
class Controller;
} // namespace Window

namespace Ui {
class FlatLabel;
class IconButton;
class PopupMenu;
} // namespace Ui

namespace Bettergram {
class BaseArticlePreviewItem;
class BaseArticleGroupPreviewItem;
class RssChannel;
class RssChannelList;
class PinnedNewsItem;
} // namespace Bettergram

namespace ChatHelpers {

/**
 * @brief The RssWidget class shows RSS feeds.
 */
class RssWidget : public TabbedSelector::Inner
{
	Q_OBJECT

public:
	RssWidget(QWidget* parent, not_null<Window::Controller*> controller);

	void refreshRecent() override;
	void clearSelection() override;
	object_ptr<TabbedSelector::InnerFooter> createFooter() override;

	void afterShown() override;
	void beforeHiding() override;

protected:
	RssWidget(QWidget* parent,
			  not_null<Window::Controller*> controller,
			  Bettergram::RssChannelList *rssChannelList,
			  const QString &showOnlyUnreadTitle,
			  const QString &showAllTitle,
			  const QString &markAsReadTitle,
			  const QString &markAllSiteNewsAsReadTitle,
			  const QString &markAllNewsAsReadTitle,
			  const style::color &rowReadFg,
			  const style::color &rowBodyFg,
			  const style::color &rowHeaderFg,
			  const style::color &siteNameFg,
			  const style::color &bg,
			  const style::color &hover,
			  const style::color &importantBg,
			  const style::color &importantFg,
			  int padding,
			  int headerPadding,
			  int imageWidth,
			  int imageHeight,
			  int rowVerticalPadding,
			  int rowHeight,
			  int channelRowHeight,
			  int dateTimeHeight,
			  bool isShowDescriptions,
			  bool isShowChannelIcons);

	TabbedSelector::InnerFooter *getFooter() const override;
	int countDesiredHeight(int newWidth) override;

	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void enterEventHook(QEvent *e) override;
	void leaveEventHook(QEvent *e) override;
	void contextMenuEvent(QContextMenuEvent *e) override;

	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *e) override;
	void timerEvent(QTimerEvent *event) override;

private:
	class Footer;

	/**
	 * @brief Row class is used to group RSS news by RSS channels
	 */
	class Row
	{
	public:
		explicit Row(const QSharedPointer<Bettergram::BaseArticlePreviewItem> &item,
					 bool isImportant = false)
			: _item(item),
			  _isImportant(isImportant)
		{}

		explicit Row(const QSharedPointer<Bettergram::BaseArticleGroupPreviewItem> &channel,
					 bool isImportant = false)
			: _channel(channel),
			  _isImportant(isImportant)
		{}

		bool isItem() const
		{
			return !_item.isNull();
		}

		bool isChannel() const
		{
			return !_channel.isNull();
		}

		bool isImportant() const
		{
			return _isImportant;
		}

		const QSharedPointer<Bettergram::BaseArticlePreviewItem> &item() const
		{
			return _item;
		}

		const QSharedPointer<Bettergram::BaseArticleGroupPreviewItem> &channel() const
		{
			return _channel;
		}

	private:
		QSharedPointer<Bettergram::BaseArticlePreviewItem> _item;
		QSharedPointer<Bettergram::BaseArticleGroupPreviewItem> _channel;

		bool _isImportant = false;
	};

	Bettergram::RssChannelList *const _rssChannelList;
	QSharedPointer<Bettergram::BaseArticleGroupPreviewItem> _pinnedNewsGroupItem;

	const QString _showOnlyUnreadTitle;
	const QString _showAllTitle;

	const QString _markAsReadTitle;
	const QString _markAllSiteNewsAsReadTitle;
	const QString _markAllNewsAsReadTitle;

	const style::color _rowReadFg;
	const style::color _rowBodyFg;
	const style::color _rowHeaderFg;
	const style::color _siteNameFg;
	const style::color _bg;
	const style::color _hover;
	const style::color _importantBg;
	const style::color _importantFg;

	const int _padding;
	const int _headerPadding;
	const int _imageWidth;
	const int _imageHeight;
	const int _rowVerticalPadding;
	const int _rowHeight;
	const int _channelRowHeight;
	const int _dateTimeHeight;

	const bool _isShowDescriptions;
	const bool _isShowChannelIcons;

	ListRowArray<Row> _rows;

	int _timerId = 0;
	int _pinnedNewsTimerId = 0;
	int _selectedRow = -1;
	int _pressedRow = -1;
	bool _isSortBySite = false;
	bool _isShowRead = true;

	Ui::FlatLabel *_lastUpdateLabel = nullptr;
	Ui::FlatLabel *_sortModeLabel = nullptr;
	Ui::FlatLabel *_isShowReadLabel = nullptr;
	Footer *_footer = nullptr;
	base::unique_qptr<Ui::PopupMenu> _menu = nullptr;

	const style::color &getNewsHeaderColor(const QSharedPointer<Bettergram::BaseArticlePreviewItem> &item) const;
	const style::color &getNewsBodyColor(const QSharedPointer<Bettergram::BaseArticlePreviewItem> &item) const;

	ClickHandlerPtr getSortModeClickHandler();
	ClickHandlerPtr getIsShowReadClickHandler();

	void toggleIsSortBySite();
	void setIsSortBySite(bool isSortBySite);

	void toggleIsShowRead();
	void setIsShowRead(bool isShowRead);

	void setSelectedRow(int selectedRow);

	int getListAreaTop() const;

	void countSelectedRow(const QPoint &point);

	void updateControlsGeometry();
	void updateLastUpdateLabel();
	void updateSortModeLabel();
	void updateIsShowReadLabel();

	void startRssTimer();
	void stopRssTimer();

	void startPinnedNewsTimer();
	void stopPinnedNewsTimer();

	void fillRowsInSortByTimeMode();
	void fillRowsInSortBySiteMode();

	void addPinnedNews();
	void addPinnedNews(const QList<QSharedPointer<Bettergram::PinnedNewsItem>> &news);

	void updateRows();
	void createPinnedNewsGroupItem();

private slots:
	void onLastUpdateChanged();
	void onIconChanged();
	void onRssUpdated();
};

} // namespace ChatHelpers
