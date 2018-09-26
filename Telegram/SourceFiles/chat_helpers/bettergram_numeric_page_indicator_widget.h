#pragma once

#include <ui/twidget.h>

namespace Ui {
class FlatLabel;
} // namespace Ui

namespace ChatHelpers {

/**
 * @brief The BettergramNumericPageIndicatorWidget class is used to show page numbers.
 * Users can click to numbers and the BettergramNumericPageIndicatorWidget emits appropriate signal
 */
class BettergramNumericPageIndicatorWidget : public TWidget
{
	Q_OBJECT

public:
	BettergramNumericPageIndicatorWidget(int pagesCount, int currentPage, QWidget* parent);

	int pagesCount() const;
	void setPagesCount(int pagesCount);

	int currentPage() const;
	void setCurrentPage(int currentPage);

public slots:

signals:
	void pagesCountChanged();
	void currentPageChanged();

protected:
	void resizeEvent(QResizeEvent *e) override;

private:
	class Indicator
	{
	public:
		explicit Indicator(Ui::FlatLabel *label, BettergramNumericPageIndicatorWidget *widget);

		int pageIndex() const { return _pageIndex; }
		void setPageIndex(int pageIndex);

		Ui::FlatLabel *label() const { return _label; }

	private:
		BettergramNumericPageIndicatorWidget *_widget = nullptr;
		int _pageIndex = 0;
		Ui::FlatLabel *_label = nullptr;

		ClickHandlerPtr getIndicatorClickHandler();
		void click();

		void setLabelText();
	};

	int _pagesCount = 1;
	int _indicatorsCount = 0;
	int _currentPage = 0;

	QList<Indicator> _indicators;

	int countNeededLabels() const;
	void createLabels();
	void fillLabels();
	Ui::FlatLabel *createLabel();

	void updateControlsGeometry();
};

} // namespace ChatHelpers
