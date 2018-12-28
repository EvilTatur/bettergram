/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/

// This file is copied from tabbed_selector.h.
// We have to do this instead of just using the TabbedSelector class
// because that class with TabbedPanel were not created as reusable things
// and it is safer to just copy/paste this instead of
// trying to adapt the existing classes

#pragma once

#include "tabbed_selector.h"

namespace ChatHelpers {

enum class BettergramSelectorTab {
	Prices,
	News,
	Videos,
	//Icos,
	Resources
};

class PricesListWidget;
class RssWidget;
class VideosWidget;
class ResourcesWidget;

class BettergramTabbedSelector : public Ui::RpWidget, private base::Subscriber {
	Q_OBJECT

public:
	enum class Mode {
		Full,
		PricesOnly
	};

	BettergramTabbedSelector(
			QWidget *parent,
			not_null<Window::Controller*> controller,
			Mode mode = Mode::Full);

	rpl::producer<> cancelled() const;
	rpl::producer<> checkForHide() const;
	rpl::producer<> slideFinished() const;

	void setRoundRadius(int radius);

	void hideFinished();
	void showStarted();
	void beforeHiding();
	void afterShown();

	int marginTop() const;
	int marginBottom() const;
	int scrollTop() const;

	bool preventAutoHide() const;
	bool isSliding() const {
		return _a_slide.animating();
	}

	void setAfterShownCallback(Fn<void(BettergramSelectorTab)> callback) {
		_afterShownCallback = std::move(callback);
	}
	void setBeforeHidingCallback(Fn<void(BettergramSelectorTab)> callback) {
		_beforeHidingCallback = std::move(callback);
	}

	// Float player interface.
	bool wheelEventFromFloatPlayer(QEvent *e);
	QRect rectForFloatPlayer() const;

	auto showRequests() const {
		return _showRequests.events();
	}

	~BettergramTabbedSelector();

protected:
	void paintEvent(QPaintEvent *e) override;
	void resizeEvent(QResizeEvent *e) override;

private:
	class Tab {
	public:
		static constexpr auto kCount = 4;

		Tab(BettergramSelectorTab type, object_ptr<TabbedSelector::Inner> widget);

		object_ptr<TabbedSelector::Inner> takeWidget();
		void returnWidget(object_ptr<TabbedSelector::Inner> widget);

		BettergramSelectorTab type() const {
			return _type;
		}
		TabbedSelector::Inner* widget() const {
			return _weak;
		}
		not_null<TabbedSelector::InnerFooter*> footer() const {
			return _footer;
		}

		void saveScrollTop();
		void saveScrollTop(int scrollTop) {
			_scrollTop = scrollTop;
		}
		int getScrollTop() const {
			return _scrollTop;
		}

	private:
		BettergramSelectorTab _type = BettergramSelectorTab::Prices;
		object_ptr<TabbedSelector::Inner> _widget = { nullptr };
		QPointer<TabbedSelector::Inner> _weak;
		object_ptr<TabbedSelector::InnerFooter> _footer;
		int _scrollTop = 0;
	};

	bool full() const;
	Tab createTab(
		BettergramSelectorTab type,
		not_null<Window::Controller*> controller);

	void paintSlideFrame(Painter &p, TimeMs ms);
	void paintContent(Painter &p);

	void updateRestrictedLabelGeometry();
	void handleScroll();

	QImage grabForAnimation();

	void scrollToY(int y);

	void showAll();
	void hideForSliding();

	void setWidgetToScrollArea();
	void createTabsSlider();
	void switchTab();
	not_null<Tab*> getTab(BettergramSelectorTab type) {
		return &_tabs[static_cast<int>(type)];
	}
	not_null<const Tab*> getTab(BettergramSelectorTab type) const {
		return &_tabs[static_cast<int>(type)];
	}
	not_null<Tab*> currentTab() {
		return getTab(_currentTabType);
	}
	not_null<const Tab*> currentTab() const {
		return getTab(_currentTabType);
	}

	Mode _mode = Mode::Full;
	int _roundRadius = 0;
	int _footerTop = 0;

	std::unique_ptr<TabbedSelector::SlideAnimation> _slideAnimation;
	Animation _a_slide;

	object_ptr<Ui::SettingsSlider> _tabsSlider = { nullptr };
	object_ptr<Ui::PlainShadow> _topShadow;
	object_ptr<Ui::PlainShadow> _bottomShadow;
	object_ptr<Ui::ScrollArea> _scroll;
	object_ptr<Ui::FlatLabel> _restrictedLabel = { nullptr };
	std::array<Tab, Tab::kCount> _tabs;
	BettergramSelectorTab _currentTabType = BettergramSelectorTab::Prices;

	Fn<void(BettergramSelectorTab)> _afterShownCallback;
	Fn<void(BettergramSelectorTab)> _beforeHidingCallback;

	rpl::event_stream<> _showRequests;
	rpl::event_stream<> _slideFinished;
};

} // namespace ChatHelpers
