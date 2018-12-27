/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

namespace Ui {

class EmptyUserpic {
public:
	EmptyUserpic(const style::color &color, const QString &name);

	void paint(
		Painter &p,
		int x,
		int y,
		int outerWidth,
		int size) const;
	void paintRounded(
		Painter &p,
		int x,
		int y,
		int outerWidth,
		int size) const;
	void paintSquare(
		Painter &p,
		int x,
		int y,
		int outerWidth,
		int size) const;
	QPixmap generate(int size);
	StorageKey uniqueKey() const;

	static void PaintSavedMessages(
		Painter &p,
		int x,
		int y,
		int outerWidth,
		int size);
	static void PaintSavedMessages(
		Painter &p,
		int x,
		int y,
		int outerWidth,
		int size,
		const style::color &bg,
		const style::color &fg);

	~EmptyUserpic();

private:
	template <typename Callback>
	void paint(
		Painter &p,
		int x,
		int y,
		int outerWidth,
		int size,
		Callback paintBackground) const;

	void fillString(const QString &name);

	style::color _color;
	QString _string;

};

} // namespace Ui
