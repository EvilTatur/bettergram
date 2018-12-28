/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/

#include "bettergram_tabbed_selector.h"

#include "prices_list_widget.h"
#include "rss_widget.h"
#include "videos_widget.h"
#include "resources_widget.h"
#include "styles/style_chat_helpers.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/shadow.h"
#include "ui/widgets/discrete_sliders.h"
#include "ui/widgets/scroll_area.h"
#include "ui/image/image_prepare.h"
#include "storage/localstorage.h"
#include "lang/lang_keys.h"
#include "mainwindow.h"
#include "observer_peer.h"
#include "apiwrap.h"

namespace ChatHelpers {

BettergramTabbedSelector::Tab::Tab(BettergramSelectorTab type, object_ptr<TabbedSelector::Inner> widget)
	: _type(type)
	, _widget(std::move(widget))
	, _weak(_widget)
	, _footer(_widget ? _widget->createFooter() : nullptr) {
	if (_footer) {
		_footer->setParent(_widget->parentWidget());
	}
}

object_ptr<TabbedSelector::Inner> BettergramTabbedSelector::Tab::takeWidget() {
	return std::move(_widget);
}

void BettergramTabbedSelector::Tab::returnWidget(object_ptr<TabbedSelector::Inner> widget) {
	Expects(widget == _weak);

	_widget = std::move(widget);
}

void BettergramTabbedSelector::Tab::saveScrollTop() {
	Expects(widget() != nullptr);

	_scrollTop = widget()->getVisibleTop();
}

BettergramTabbedSelector::BettergramTabbedSelector(
		QWidget *parent,
		not_null<Window::Controller*> controller,
		Mode mode)
	: RpWidget(parent)
	, _mode(mode)
	, _tabsSlider(this, st::emojiTabs)
	, _topShadow(full() ? object_ptr<Ui::PlainShadow>(this) : nullptr)
	, _bottomShadow(this)
	, _scroll(this, st::emojiScroll)
	, _tabs { {
			createTab(BettergramSelectorTab::Prices, controller),
			createTab(BettergramSelectorTab::News, controller),
			createTab(BettergramSelectorTab::Videos, controller),
			//createTab(BettergramSelectorTab::Icos, controller),
			createTab(BettergramSelectorTab::Resources, controller),
		} }
	, _currentTabType(full()
					  ? Auth().settings().bettergramSelectorTab()
					  : BettergramSelectorTab::Prices) {
	resize(st::emojiPanWidth, st::emojiPanMaxHeight);

	for (auto &tab : _tabs) {
		if (!tab.widget()) {
			continue;
		}
		tab.footer()->hide();
		tab.widget()->hide();
	}
	createTabsSlider();
	setWidgetToScrollArea();

	_bottomShadow->setGeometry(0, _scroll->y() + _scroll->height() - st::lineWidth, width(), st::lineWidth);

	for (auto &tab : _tabs) {
		const auto widget = tab.widget();
		if (!widget) {
			continue;
		}
		widget->scrollToRequests(
		) | rpl::start_with_next([=, tab = &tab](int y) {
			if (tab == currentTab()) {
				scrollToY(y);
			} else {
				tab->saveScrollTop(y);
			}
		}, widget->lifetime());

		widget->disableScrollRequests(
		) | rpl::start_with_next([=, tab = &tab](bool disabled) {
			if (tab == currentTab()) {
				_scroll->disableScroll(disabled);
			}
		}, widget->lifetime());
	}

	if (full()) {
		_topShadow->raise();
	}
	_bottomShadow->raise();
	if (full()) {
		_tabsSlider->raise();
	}

	//setAttribute(Qt::WA_AcceptTouchEvents);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
	showAll();
}

BettergramTabbedSelector::Tab BettergramTabbedSelector::createTab(BettergramSelectorTab type, not_null<Window::Controller*> controller) {
auto createWidget = [&]() -> object_ptr<TabbedSelector::Inner> {
	if (!full() && type != BettergramSelectorTab::Prices) {
		return { nullptr };
	}
	switch (type) {
	case BettergramSelectorTab::Prices:
		return object_ptr<PricesListWidget>(this, controller);
	case BettergramSelectorTab::News:
		return object_ptr<RssWidget>(this, controller);
	case BettergramSelectorTab::Videos:
		return object_ptr<VideosWidget>(this, controller);
	case BettergramSelectorTab::Resources:
		return object_ptr<ResourcesWidget>(this, controller);
	}
	Unexpected("Type in BettergramTabbedSelector::createTab.");
};
return Tab{ type, createWidget() };
}

bool BettergramTabbedSelector::full() const {
	return (_mode == Mode::Full);
}

void BettergramTabbedSelector::resizeEvent(QResizeEvent *e) {
	if (full()) {
		_tabsSlider->resizeToWidth(width());
		_tabsSlider->moveToLeft(0, 0);
		_topShadow->setGeometry(
			_tabsSlider->x(),
			_tabsSlider->bottomNoMargins() - st::lineWidth,
			_tabsSlider->width(),
			st::lineWidth);
	}

	auto scrollWidth = width() - st::buttonRadius;
	auto scrollHeight = height() - scrollTop() - marginBottom();
	auto inner = currentTab()->widget();
	auto innerWidth = scrollWidth - st::emojiScroll.width;
	auto updateScrollGeometry = [&] {
		_scroll->setGeometryToLeft(
			st::buttonRadius,
			scrollTop(),
			scrollWidth,
			scrollHeight);
	};
	auto updateInnerGeometry = [&] {
		auto scrollTop = _scroll->scrollTop();
		auto scrollBottom = scrollTop + scrollHeight;
		inner->setMinimalHeight(innerWidth, scrollHeight);
		inner->setVisibleTopBottom(scrollTop, scrollBottom);
	};
	if (e->oldSize().height() > height()) {
		updateScrollGeometry();
		updateInnerGeometry();
	} else {
		updateInnerGeometry();
		updateScrollGeometry();
	}
	_bottomShadow->setGeometry(0, _scroll->y() + _scroll->height() - st::lineWidth, width(), st::lineWidth);
	updateRestrictedLabelGeometry();

	_footerTop = height() - st::emojiFooterHeight;
	for (auto &tab : _tabs) {
		if (!tab.widget()) {
			continue;
		}
		tab.footer()->resizeToWidth(width());
		tab.footer()->moveToLeft(0, _footerTop);
	}

	update();
}

void BettergramTabbedSelector::updateRestrictedLabelGeometry() {
	if (!_restrictedLabel) {
		return;
	}

	auto labelWidth = width() - st::stickerPanPadding * 2;
	_restrictedLabel->resizeToWidth(labelWidth);
	_restrictedLabel->moveToLeft(
		(width() - _restrictedLabel->width()) / 2,
		(height() / 3 - _restrictedLabel->height() / 2));
}

void BettergramTabbedSelector::paintEvent(QPaintEvent *e) {
	Painter p(this);

	auto ms = getms();

	auto switching = (_slideAnimation != nullptr);
	if (switching) {
		paintSlideFrame(p, ms);
		if (!_a_slide.animating()) {
			_slideAnimation.reset();
			afterShown();
			_slideFinished.fire({});
		}
	} else {
		paintContent(p);
	}
}

void BettergramTabbedSelector::paintSlideFrame(Painter &p, TimeMs ms) {
	if (_roundRadius > 0) {
		if (full()) {
			auto topPart = QRect(0, 0, width(), _tabsSlider->height() + _roundRadius);
			App::roundRect(p, topPart, st::emojiPanBg, ImageRoundRadius::Small, RectPart::FullTop | RectPart::NoTopBottom);
		} else {
			auto topPart = QRect(0, 0, width(), 3 * _roundRadius);
			App::roundRect(p, topPart, st::emojiPanBg, ImageRoundRadius::Small, RectPart::FullTop);
		}
	} else if (full()) {
		p.fillRect(0, 0, width(), _tabsSlider->height(), st::emojiPanBg);
	}
	auto slideDt = _a_slide.current(ms, 1.);
	_slideAnimation->paintFrame(p, slideDt, 1.);
}

void BettergramTabbedSelector::paintContent(Painter &p) {
	auto &bottomBg = st::emojiPanBg;
	if (_roundRadius > 0) {
		if (full()) {
			auto topPart = QRect(0, 0, width(), _tabsSlider->height() + _roundRadius);
			App::roundRect(p, topPart, st::emojiPanBg, ImageRoundRadius::Small, RectPart::FullTop | RectPart::NoTopBottom);
		} else {
			auto topPart = QRect(0, 0, width(), 3 * _roundRadius);
			App::roundRect(p, topPart, st::emojiPanBg, ImageRoundRadius::Small, RectPart::FullTop);
		}

		auto bottomPart = QRect(0, _footerTop - _roundRadius, width(), st::emojiFooterHeight + _roundRadius);
		auto bottomParts = RectPart::NoTopBottom | RectPart::FullBottom;
		App::roundRect(p, bottomPart, bottomBg, ImageRoundRadius::Small, bottomParts);
	} else {
		if (full()) {
			p.fillRect(0, 0, width(), _tabsSlider->height(), st::emojiPanBg);
		}
		p.fillRect(0, _footerTop, width(), st::emojiFooterHeight, bottomBg);
	}

	auto sidesTop = marginTop();
	auto sidesHeight = height() - sidesTop - marginBottom();
	if (_restrictedLabel) {
		p.fillRect(0, sidesTop, width(), sidesHeight, st::emojiPanBg);
	} else {
		p.fillRect(myrtlrect(width() - st::emojiScroll.width, sidesTop, st::emojiScroll.width, sidesHeight), st::emojiPanBg);
		p.fillRect(myrtlrect(0, sidesTop, st::buttonRadius, sidesHeight), st::emojiPanBg);
	}
}

int BettergramTabbedSelector::marginTop() const {
	return full() ? (_tabsSlider->height() - st::lineWidth) : _roundRadius;
}

int BettergramTabbedSelector::scrollTop() const {
	return full() ? marginTop() : 0;
}

int BettergramTabbedSelector::marginBottom() const {
	return st::emojiFooterHeight;
}

bool BettergramTabbedSelector::preventAutoHide() const {
	return false;
}

QImage BettergramTabbedSelector::grabForAnimation() {
	auto slideAnimationData = base::take(_slideAnimation);
	auto slideAnimation = base::take(_a_slide);

	showAll();
	if (full()) {
		_topShadow->hide();
		_tabsSlider->hide();
	}
	Ui::SendPendingMoveResizeEvents(this);

	auto result = QImage(size() * cIntRetinaFactor(), QImage::Format_ARGB32_Premultiplied);
	result.setDevicePixelRatio(cRetinaFactor());
	result.fill(Qt::transparent);
	render(&result);

	_a_slide = base::take(slideAnimation);
	_slideAnimation = base::take(slideAnimationData);

	return result;
}

bool BettergramTabbedSelector::wheelEventFromFloatPlayer(QEvent *e) {
	return _scroll->viewportEvent(e);
}

QRect BettergramTabbedSelector::rectForFloatPlayer() const {
	return mapToGlobal(_scroll->geometry());
}

BettergramTabbedSelector::~BettergramTabbedSelector() = default;

void BettergramTabbedSelector::hideFinished() {
	for (auto &tab : _tabs) {
		if (!tab.widget()) {
			continue;
		}
		tab.widget()->panelHideFinished();
	}
	_a_slide.finish();
	_slideAnimation.reset();
}

void BettergramTabbedSelector::showStarted() {
	if (full()) {
		Auth().api().updateStickers();
	}
	currentTab()->widget()->refreshRecent();
	currentTab()->widget()->preloadImages();
	_a_slide.finish();
	_slideAnimation.reset();
	showAll();
}

void BettergramTabbedSelector::beforeHiding() {
	if (!_scroll->isHidden()) {
		currentTab()->widget()->beforeHiding();
		if (_beforeHidingCallback) {
			_beforeHidingCallback(_currentTabType);
		}
	}
}

void BettergramTabbedSelector::afterShown() {
	if (!_a_slide.animating()) {
		showAll();
		currentTab()->widget()->afterShown();
		if (_afterShownCallback) {
			_afterShownCallback(_currentTabType);
		}
	}
}

void BettergramTabbedSelector::showAll() {
	currentTab()->footer()->show();
	_scroll->show();
	_bottomShadow->setVisible(false);

	if (full()) {
		_topShadow->show();
		_tabsSlider->show();
	}
}

void BettergramTabbedSelector::hideForSliding() {
	hideChildren();
	if (full()) {
		_topShadow->show();
		_tabsSlider->show();
	}
	currentTab()->widget()->clearSelection();
}

void BettergramTabbedSelector::handleScroll() {
	auto scrollTop = _scroll->scrollTop();
	auto scrollBottom = scrollTop + _scroll->height();
	currentTab()->widget()->setVisibleTopBottom(scrollTop, scrollBottom);
}

void BettergramTabbedSelector::setRoundRadius(int radius) {
	_roundRadius = radius;
	if (full()) {
		_tabsSlider->setRippleTopRoundRadius(_roundRadius);
	}
}

void BettergramTabbedSelector::createTabsSlider() {
	if (!full()) {
		return;
	}

	_tabsSlider.create(this, st::emojiTabs);

	auto sections = QStringList();
	sections.push_back(lang(lng_switch_prices).toUpper());
	sections.push_back(lang(lng_switch_news).toUpper());
	sections.push_back(lang(lng_switch_videos).toUpper());
	//sections.push_back(lang(lng_switch_icos).toUpper());
	sections.push_back(lang(lng_switch_resources).toUpper());
	_tabsSlider->setSections(sections);

	_tabsSlider->setActiveSectionFast(static_cast<int>(_currentTabType));
	_tabsSlider->sectionActivated(
	) | rpl::start_with_next(
		[this] { switchTab(); },
		lifetime());
}

void BettergramTabbedSelector::switchTab() {
	Expects(full());

	auto tab = _tabsSlider->activeSection();
	Assert(tab >= 0 && tab < Tab::kCount);
	auto newTabType = static_cast<BettergramSelectorTab>(tab);
	if (_currentTabType == newTabType) {
		return;
	}

	auto wasTab = _currentTabType;
	currentTab()->saveScrollTop();

	beforeHiding();

	auto wasCache = grabForAnimation();

	auto widget = _scroll->takeWidget<TabbedSelector::Inner>();
	widget->setParent(this);
	widget->hide();
	currentTab()->footer()->hide();
	currentTab()->returnWidget(std::move(widget));

	_currentTabType = newTabType;
	_restrictedLabel.destroy();

	currentTab()->widget()->refreshRecent();
	currentTab()->widget()->preloadImages();
	setWidgetToScrollArea();

	auto nowCache = grabForAnimation();

	auto direction = (wasTab > _currentTabType) ? TabbedSelector::SlideAnimation::Direction::LeftToRight : TabbedSelector::SlideAnimation::Direction::RightToLeft;
	if (direction == TabbedSelector::SlideAnimation::Direction::LeftToRight) {
		std::swap(wasCache, nowCache);
	}
	_slideAnimation = std::make_unique<TabbedSelector::SlideAnimation>();
	auto slidingRect = QRect(0, _scroll->y() * cIntRetinaFactor(), width() * cIntRetinaFactor(), (height() - _scroll->y()) * cIntRetinaFactor());
	_slideAnimation->setFinalImages(direction, std::move(wasCache), std::move(nowCache), slidingRect, false);
	auto corners = App::cornersMask(ImageRoundRadius::Small);
	_slideAnimation->setCornerMasks(corners[0], corners[1], corners[2], corners[3]);
	_slideAnimation->start();

	hideForSliding();

	getTab(wasTab)->widget()->hideFinished();

	_a_slide.start([this] { update(); }, 0., 1., st::emojiPanSlideDuration, anim::linear);
	update();

	if (full()) {
		Auth().settings().setBettergramSelectorTab(_currentTabType);
		Auth().saveSettingsDelayed();
	}
}

void BettergramTabbedSelector::setWidgetToScrollArea() {
	auto inner = _scroll->setOwnedWidget(currentTab()->takeWidget());
	auto innerWidth = _scroll->width() - st::emojiScroll.width;
	auto scrollHeight = _scroll->height();
	inner->setMinimalHeight(innerWidth, scrollHeight);
	inner->moveToLeft(0, 0);
	inner->show();

	_scroll->disableScroll(false);
	scrollToY(currentTab()->getScrollTop());
	handleScroll();
}

void BettergramTabbedSelector::scrollToY(int y) {
	_scroll->scrollToY(y);

	// Qt render glitch workaround, shadow sometimes disappears if we just scroll to y.
	if (full()) {
		_topShadow->update();
	}
}
} // namespace ChatHelpers
