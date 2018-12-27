/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#include <QtCore/QCoreApplication>

#include "codegen/numbers/options.h"
#include "codegen/numbers/processor.h"

int main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);

	auto options = codegen::numbers::parseOptions();
	if (options.inputPath.isEmpty()) {
		return -1;
	}

	codegen::numbers::Processor processor(options);
	return processor.launch();
}
