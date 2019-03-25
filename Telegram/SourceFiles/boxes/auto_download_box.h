/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "boxes/abstract_box.h"

namespace Data {
namespace AutoDownload {
enum class Source;
} // namespace AutoDownload
} // namespace Data

class AutoDownloadBox : public BoxContent {
public:
	AutoDownloadBox(QWidget*, Data::AutoDownload::Source source);

protected:
	void prepare() override;

private:
	void setupContent();

	Data::AutoDownload::Source _source;

};
