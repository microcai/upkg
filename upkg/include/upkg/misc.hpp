#pragma once

#include <tuple>
#include <string>
#include <atomic>

#include <QString>
#include <QFile>
#include <QCryptographicHash>

#include "minizip/zip.h"

namespace util {
	std::tuple<bool, std::string> compress_gz(const char* inFile, const char* outFile);
	std::tuple<bool, std::string> compress_zip(const char* inFile, const char* outFile);

	QString GetFileVertion(QString fullName);
	QString md5sum(const QString& path, std::atomic_bool& abort);
}
