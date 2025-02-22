/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "info/settings/info_settings_widget.h"

#include "info/info_memento.h"
#include "info/info_controller.h"
#include "settings/settings_common.h"
#include "settings/settings_main.h"
#include "settings/settings_information.h"
#include "ui/ui_utility.h"

namespace Info {
namespace Settings {

Memento::Memento(not_null<UserData*> self, Type type)
: ContentMemento(Tag{ self })
, _type(type) {
}

Section Memento::section() const {
	return Section(_type);
}

object_ptr<ContentWidget> Memento::createWidget(
		QWidget *parent,
		not_null<Controller*> controller,
		const QRect &geometry) {
	auto result = object_ptr<Widget>(
		parent,
		controller);
	result->setInternalState(geometry, this);
	return result;
}

Memento::~Memento() = default;

Widget::Widget(
	QWidget *parent,
	not_null<Controller*> controller)
: ContentWidget(parent, controller)
, _self(controller->key().settingsSelf())
, _type(controller->section().settingsType())
, _inner(
	setInnerWidget(
		_type()->create(this, controller->parentController())))
, _pinnedToTop(_inner->createPinnedToTop(this)) {
	_inner->sectionShowOther(
	) | rpl::start_with_next([=](Type type) {
		controller->showSettings(type);
	}, _inner->lifetime());

	if (_pinnedToTop) {
		_inner->widthValue(
		) | rpl::start_with_next([=](int w) {
			_pinnedToTop->resizeToWidth(w);
			setScrollTopSkip(_pinnedToTop->height());
		}, _pinnedToTop->lifetime());

		_pinnedToTop->heightValue(
		) | rpl::start_with_next([=](int h) {
			setScrollTopSkip(h);
		}, _pinnedToTop->lifetime());
	}
}

Widget::~Widget() = default;

not_null<UserData*> Widget::self() const {
	return _self;
}

bool Widget::showInternal(not_null<ContentMemento*> memento) {
	//if (const auto myMemento = dynamic_cast<Memento*>(memento.get())) {
	//	Assert(myMemento->self() == self());

	//	if (_inner->showInternal(myMemento)) {
	//		return true;
	//	}
	//}
	return false;
}

void Widget::setInternalState(
		const QRect &geometry,
		not_null<Memento*> memento) {
	setGeometry(geometry);
	Ui::SendPendingMoveResizeEvents(this);
	restoreState(memento);
}

void Widget::saveChanges(FnMut<void()> done) {
	_inner->sectionSaveChanges(std::move(done));
}

void Widget::showFinished() {
	_inner->showFinished();
}

rpl::producer<bool> Widget::desiredShadowVisibility() const {
	return (_type == ::Settings::Main::Id()
		|| _type == ::Settings::Information::Id())
		? ContentWidget::desiredShadowVisibility()
		: rpl::single(true);
}

rpl::producer<QString> Widget::title() {
	return _inner->title();
}

std::shared_ptr<ContentMemento> Widget::doCreateMemento() {
	auto result = std::make_shared<Memento>(self(), _type);
	saveState(result.get());
	return result;
}

void Widget::saveState(not_null<Memento*> memento) {
	memento->setScrollTop(scrollTopSave());
}

void Widget::restoreState(not_null<Memento*> memento) {
	scrollTopRestore(memento->scrollTop());
}

} // namespace Settings
} // namespace Info
