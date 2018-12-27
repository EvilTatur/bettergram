/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#include "mtproto/type_utils.h"

const MTPReplyMarkup MTPnullMarkup = MTP_replyKeyboardMarkup(
	MTP_flags(0),
	MTP_vector<MTPKeyboardButtonRow>(0));
const MTPVector<MTPMessageEntity> MTPnullEntities = MTP_vector<MTPMessageEntity>(0);
const MTPMessageFwdHeader MTPnullFwdHeader = MTP_messageFwdHeader(
	MTP_flags(0),
	MTPint(),
	MTPint(),
	MTPint(),
	MTPint(),
	MTPstring(),
	MTPPeer(),
	MTPint());
