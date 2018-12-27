/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "styles/style_widgets.h"

namespace Ui {

class CrossAnimation {
public:
	static void paint(
		Painter &p,
		const style::CrossAnimation &st,
		style::color color,
		int x,
		int y,
		int outerWidth,
		float64 shown,
		float64 loading = 0.);
	static void paintStaticLoading(
		Painter &p,
		const style::CrossAnimation &st,
		style::color color,
		int x,
		int y,
		int outerWidth,
		float64 shown);

};

} // namespace Ui
