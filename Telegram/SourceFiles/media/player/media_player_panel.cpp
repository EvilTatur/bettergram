/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#include "media/player/media_player_panel.h"

#include "media/player/media_player_instance.h"
#include "info/media/info_media_list_widget.h"
#include "history/history.h"
#include "history/history_item.h"
#include "data/data_document.h"
#include "data/data_media_types.h"
#include "data/data_channel.h"
#include "data/data_chat.h"
#include "ui/widgets/shadow.h"
#include "ui/widgets/scroll_area.h"
#include "mainwindow.h"
#include "styles/style_overview.h"
#include "styles/style_widgets.h"
#include "styles/style_media_player.h"
#include "styles/style_info.h"

namespace Media {
namespace Player {
namespace {

using ListWidget = Info::Media::ListWidget;

constexpr auto kPlaylistIdsLimit = 32;
constexpr auto kDelayedHideTimeout = crl::time(3000);

} // namespace

Panel::Panel(
	QWidget *parent,
	not_null<Window::Controller*> window)
: RpWidget(parent)
, AbstractController(window)
, _showTimer([this] { startShow(); })
, _hideTimer([this] { startHideChecked(); })
, _scroll(this, st::mediaPlayerScroll) {
	hide();
	updateSize();
}

bool Panel::overlaps(const QRect &globalRect) {
	if (isHidden() || _a_appearance.animating()) return false;

	auto marginLeft = rtl() ? contentRight() : contentLeft();
	auto marginRight = rtl() ? contentLeft() : contentRight();
	return rect().marginsRemoved(QMargins(marginLeft, contentTop(), marginRight, contentBottom())).contains(QRect(mapFromGlobal(globalRect.topLeft()), globalRect.size()));
}

void Panel::windowActiveChanged() {
	if (!App::wnd()->windowHandle()->isActive() && !isHidden()) {
		leaveEvent(nullptr);
	}
}

void Panel::resizeEvent(QResizeEvent *e) {
	updateControlsGeometry();
}

void Panel::listHeightUpdated(int newHeight) {
	if (newHeight > emptyInnerHeight()) {
		updateSize();
	} else {
		_hideTimer.callOnce(0);
	}
}

bool Panel::contentTooSmall() const {
	const auto innerHeight = _scroll->widget()
		? _scroll->widget()->height()
		: emptyInnerHeight();
	return (innerHeight <= emptyInnerHeight());
}

int Panel::emptyInnerHeight() const {
	return st::infoMediaMargin.top()
		+ st::overviewFileLayout.songPadding.top()
		+ st::overviewFileLayout.songThumbSize
		+ st::overviewFileLayout.songPadding.bottom()
		+ st::infoMediaMargin.bottom();
}

bool Panel::preventAutoHide() const {
	if (const auto list = static_cast<ListWidget*>(_scroll->widget())) {
		return list->preventAutoHide();
	}
	return false;
}

void Panel::updateControlsGeometry() {
	auto scrollTop = contentTop();
	auto width = contentWidth();
	auto scrollHeight = qMax(height() - scrollTop - contentBottom() - scrollMarginBottom(), 0);
	if (scrollHeight > 0) {
		_scroll->setGeometryToRight(contentRight(), scrollTop, width, scrollHeight);
	}
	if (auto widget = static_cast<TWidget*>(_scroll->widget())) {
		widget->resizeToWidth(width);
	}
}

int Panel::bestPositionFor(int left) const {
	left -= contentLeft();
	left -= st::mediaPlayerFileLayout.songPadding.left();
	left -= st::mediaPlayerFileLayout.songThumbSize / 2;
	return left;
}

void Panel::scrollPlaylistToCurrentTrack() {
	if (const auto list = static_cast<ListWidget*>(_scroll->widget())) {
		const auto rect = list->getCurrentSongGeometry();
		_scroll->scrollToY(rect.y() - st::infoMediaMargin.top());
	}
}

void Panel::updateSize() {
	auto width = contentLeft() + st::mediaPlayerPanelWidth + contentRight();
	auto height = contentTop();
	auto listHeight = 0;
	if (auto widget = _scroll->widget()) {
		listHeight = widget->height();
	}
	auto scrollVisible = (listHeight > 0);
	auto scrollHeight = scrollVisible ? (qMin(listHeight, st::mediaPlayerListHeightMax) + st::mediaPlayerListMarginBottom) : 0;
	height += scrollHeight + contentBottom();
	resize(width, height);
	_scroll->setVisible(scrollVisible);
}

void Panel::paintEvent(QPaintEvent *e) {
	Painter p(this);

	if (!_cache.isNull()) {
		bool animating = _a_appearance.animating(crl::now());
		if (animating) {
			p.setOpacity(_a_appearance.current(_hiding ? 0. : 1.));
		} else if (_hiding || isHidden()) {
			hideFinished();
			return;
		}
		p.drawPixmap(0, 0, _cache);
		if (!animating) {
			showChildren();
			_cache = QPixmap();
		}
		return;
	}

	// draw shadow
	auto shadowedRect = myrtlrect(contentLeft(), contentTop(), contentWidth(), contentHeight());
	auto shadowedSides = (rtl() ? RectPart::Right : RectPart::Left)
		| RectPart::Bottom
		| (rtl() ? RectPart::Left : RectPart::Right)
		| RectPart::Top;
	Ui::Shadow::paint(p, shadowedRect, width(), st::defaultRoundShadow, shadowedSides);
	auto parts = RectPart::Full;
	App::roundRect(p, shadowedRect, st::menuBg, MenuCorners, nullptr, parts);
}

void Panel::enterEventHook(QEvent *e) {
	if (_ignoringEnterEvents || contentTooSmall()) return;

	_hideTimer.cancel();
	if (_a_appearance.animating(crl::now())) {
		startShow();
	} else {
		_showTimer.callOnce(0);
	}
	return TWidget::enterEventHook(e);
}

void Panel::leaveEventHook(QEvent *e) {
	if (preventAutoHide()) {
		return;
	}
	_showTimer.cancel();
	if (_a_appearance.animating(crl::now())) {
		startHide();
	} else {
		_hideTimer.callOnce(300);
	}
	return TWidget::leaveEventHook(e);
}

void Panel::showFromOther() {
	_hideTimer.cancel();
	if (_a_appearance.animating(crl::now())) {
		startShow();
	} else {
		_showTimer.callOnce(300);
	}
}

void Panel::hideFromOther() {
	_showTimer.cancel();
	if (_a_appearance.animating(crl::now())) {
		startHide();
	} else {
		_hideTimer.callOnce(0);
	}
}

void Panel::ensureCreated() {
	if (_scroll->widget()) return;

	_refreshListLifetime = instance()->playlistChanges(
		AudioMsgId::Type::Song
	) | rpl::start_with_next([this] {
		refreshList();
	});
	refreshList();

	if (cPlatform() == dbipMac || cPlatform() == dbipMacOld) {
		if (const auto window = App::wnd()) {
			connect(
				window->windowHandle(),
				&QWindow::activeChanged,
				this,
				&Panel::windowActiveChanged);
		}
	}

	_ignoringEnterEvents = false;
}

void Panel::refreshList() {
	const auto current = instance()->current(AudioMsgId::Type::Song);
	const auto contextId = current.contextId();
	const auto peer = [&]() -> PeerData* {
		const auto item = contextId ? App::histItemById(contextId) : nullptr;
		const auto media = item ? item->media() : nullptr;
		const auto document = media ? media->document() : nullptr;
		if (!document || !document->isSharedMediaMusic()) {
			return nullptr;
		}
		const auto result = item->history()->peer;
		if (const auto migrated = result->migrateTo()) {
			return migrated;
		}
		return result;
	}();
	const auto migrated = peer ? peer->migrateFrom() : nullptr;
	if (_listPeer != peer || _listMigratedPeer != migrated) {
		_scroll->takeWidget<QWidget>().destroy();
		_listPeer = _listMigratedPeer = nullptr;
	}
	if (peer && !_listPeer) {
		_listPeer = peer;
		_listMigratedPeer = migrated;
		auto list = object_ptr<ListWidget>(this, infoController());

		const auto weak = _scroll->setOwnedWidget(std::move(list));

		updateSize();
		updateControlsGeometry();

		weak->checkForHide(
		) | rpl::start_with_next([this] {
			if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
				_hideTimer.callOnce(kDelayedHideTimeout);
			}
		}, weak->lifetime());

		weak->heightValue(
		) | rpl::start_with_next([this](int newHeight) {
			listHeightUpdated(newHeight);
		}, weak->lifetime());

		weak->scrollToRequests(
		) | rpl::start_with_next([this](int newScrollTop) {
			_scroll->scrollToY(newScrollTop);
		}, weak->lifetime());

		// MSVC BUG + REGRESSION rpl::mappers::tuple :(
		using namespace rpl::mappers;
		rpl::combine(
			_scroll->scrollTopValue(),
			_scroll->heightValue()
		) | rpl::start_with_next([=](int top, int height) {
			const auto bottom = top + height;
			weak->setVisibleTopBottom(top, bottom);
		}, weak->lifetime());

		auto memento = Info::Media::Memento(
			peerId(),
			migratedPeerId(),
			section().mediaType());
		memento.setAroundId(contextId);
		memento.setIdsLimit(kPlaylistIdsLimit);
		memento.setScrollTopItem(contextId);
		memento.setScrollTopShift(-st::infoMediaMargin.top());
		weak->restoreState(&memento);
	}
}

void Panel::performDestroy() {
	if (!_scroll->widget()) return;

	_scroll->takeWidget<QWidget>().destroy();
	_listPeer = _listMigratedPeer = nullptr;
	_refreshListLifetime.destroy();

	if (cPlatform() == dbipMac || cPlatform() == dbipMacOld) {
		if (const auto window = App::wnd()) {
			disconnect(
				window->windowHandle(),
				&QWindow::activeChanged,
				this,
				&Panel::windowActiveChanged);
		}
	}
}

Info::Key Panel::key() const {
	return Info::Key(_listPeer);
}

PeerData *Panel::migrated() const {
	return _listMigratedPeer;
}

Info::Section Panel::section() const {
	return Info::Section(Info::Section::MediaType::MusicFile);
}

void Panel::startShow() {
	ensureCreated();
	if (contentTooSmall()) {
		return;
	}

	if (isHidden()) {
		scrollPlaylistToCurrentTrack();
		show();
	} else if (!_hiding) {
		return;
	}
	_hiding = false;
	startAnimation();
}

void Panel::hideIgnoringEnterEvents() {
	_ignoringEnterEvents = true;
	if (isHidden()) {
		hideFinished();
	} else {
		startHide();
	}
}

void Panel::startHideChecked() {
	if (!contentTooSmall() && preventAutoHide()) {
		return;
	}
	if (isHidden()) {
		hideFinished();
	} else {
		startHide();
	}
}

void Panel::startHide() {
	if (_hiding || isHidden()) return;

	_hiding = true;
	startAnimation();
}

void Panel::startAnimation() {
	auto from = _hiding ? 1. : 0.;
	auto to = _hiding ? 0. : 1.;
	if (_cache.isNull()) {
		showChildren();
		_cache = Ui::GrabWidget(this);
	}
	hideChildren();
	_a_appearance.start([this] { appearanceCallback(); }, from, to, st::defaultInnerDropdown.duration);
}

void Panel::appearanceCallback() {
	if (!_a_appearance.animating() && _hiding) {
		_hiding = false;
		hideFinished();
	} else {
		update();
	}
}

void Panel::hideFinished() {
	hide();
	_cache = QPixmap();
	performDestroy();
}

int Panel::contentLeft() const {
	return st::mediaPlayerPanelMarginLeft;
}

int Panel::contentTop() const {
	return st::mediaPlayerPanelMarginLeft;
}

int Panel::contentRight() const {
	return st::mediaPlayerPanelMarginLeft;
}

int Panel::contentBottom() const {
	return st::mediaPlayerPanelMarginBottom;
}

int Panel::scrollMarginBottom() const {
	return 0;// st::mediaPlayerPanelMarginBottom;
}

} // namespace Player
} // namespace Media
