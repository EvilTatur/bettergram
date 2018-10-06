#pragma once

#include <ui/twidget.h>

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
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void enterEventHook(QEvent *e) override;
	void leaveEventHook(QEvent *e) override;

	void resizeEvent(QResizeEvent *e) override;
	void paintEvent(QPaintEvent *event) override;

private:
	class Indicator
	{
	public:
		explicit Indicator() = default;

		int left() const { return _left; }
		void setLeft(int left);

		int width() const;
		int right() const { return left() + width(); }

		QRect rect() const;

		int pageIndex() const { return _pageIndex; }
		void setPageIndex(int pageIndex);

		const QString &text() const { return _text; }

	private:
		int _left = 0;
		int _pageIndex = 0;
		QString _text;

		void setText();
	};

	int _pagesCount = 1;
	int _currentPage = 0;
	int _hoveredPage = -1;
	int _pressedPage = -1;

	QList<Indicator> _indicators;

	void setHoveredPage(int hoveredPage);

	void countHoveredPage(const QPoint &position);
	int countNeededIndicators() const;

	void createIndicators();
	void fillIndicators();

	void updateControlsGeometry();
};

} // namespace ChatHelpers
