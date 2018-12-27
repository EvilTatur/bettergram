/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include <rpl/event_stream.h>
#include "boxes/peers/manage_peer_box.h"

class EditPeerInfoBox : public BoxContent {
public:
	EditPeerInfoBox(QWidget*, not_null<PeerData*> peer);

	void setInnerFocus() override {
		_focusRequests.fire({});
	}

protected:
	void prepare() override;

private:
	not_null<PeerData*> _peer;
	rpl::event_stream<> _focusRequests;

};
