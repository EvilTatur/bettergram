/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

namespace Core {

class MainQueueProcessor : public QObject {
public:
	MainQueueProcessor();
	~MainQueueProcessor();

protected:
	bool event(QEvent *event) override;

private:
	void acquire();
	void release();

};

} // namespace Core
