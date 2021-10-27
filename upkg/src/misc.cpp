#include "upkg/misc.hpp"
#include <boost/filesystem.hpp>

#ifdef _MSC_VER
#	include <windows.h>
#	pragma comment(lib, "Version")
#endif

namespace util {

	std::tuple<bool, std::string> compress_gz(const char* inFile, const char* outFile)
	{
		bool ret = false;
		auto const do_return =
			[&ret](std::string error_info = "") -> std::tuple<bool, std::string>
		{
			return std::make_tuple(ret, error_info);
		};

		FILE* in = fopen(inFile, "rb");
		if (in)
		{
			gzFile out;
			char outmode[20];

			strcpy(outmode, "wb6f");
			out = gzopen(outFile, outmode);
			if (out)
			{
				char buf[16384];
				int len;

				do
				{
					len = (int)fread(buf, 1, sizeof(buf), in);
					if (ferror(in))
						return do_return("error in reading");

					if (len == 0)
						break;

					if (gzwrite(out, buf, (unsigned)len) != len)
						break;

				} while (1);

				fclose(in);
				if (gzclose(out) != Z_OK)
					return do_return("zipOpenNewFileInZip3 failed");

				ret = true;
				return do_return();
			}
		}
		return do_return("fopen failed");
	}

	std::tuple<bool, std::string> compress_zip(const char* inFile, const char* outFile)
	{
		bool ret = false;
		auto const do_return =
			[&ret](std::string error_info = "") -> std::tuple<bool, std::string>
		{
			return std::make_tuple(ret, error_info);
		};

		boost::filesystem::path infile(inFile);
		std::string szFname = infile.filename().string();

		zipFile zf;
		int errclose;
		zip_fileinfo zi = { 0 };

		zf = zipOpen(outFile, APPEND_STATUS_CREATE);
		if (!zf)
			return do_return();

		int err = 0;
		unsigned long crcFile = 0;
		int size_buf = 16384;
		int size_read = 0;
		void* buf = (void*)malloc(size_buf);
		const char* password = NULL;
		FILE* fin = NULL;

		err = zipOpenNewFileInZip3(zf, szFname.c_str(), &zi,
			NULL, 0, NULL, 0, NULL /* comment*/,
			Z_DEFLATED,
			Z_DEFAULT_COMPRESSION, 0,
			/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
			-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
			password, crcFile);
		if (err != ZIP_OK)
		{
			zipClose(zf, NULL);
			return do_return("zipOpenNewFileInZip3 failed");
		}
		else
		{
			fin = fopen(inFile, "rb");
			if (!fin)
			{
				zipClose(zf, NULL);
				return do_return("fopen failed");
			}
		}

		if (err == ZIP_OK)
		{
			do
			{
				err = ZIP_OK;
				size_read = (int)fread(buf, 1, size_buf, fin);
				if (size_read < size_buf)
					if (feof(fin) == 0)
					{
						err = ZIP_ERRNO;
						return do_return("error in reading");
					}

				if (size_read > 0)
				{
					err = zipWriteInFileInZip(zf, buf, size_read);
					if (err < 0)
						return do_return("error in writing in the zipfile");
				}
			} while ((err == ZIP_OK) && (size_read > 0));

			if (fin)
				fclose(fin);

			if (err < 0)
				err = ZIP_ERRNO;
			else
			{
				err = zipCloseFileInZip(zf);
				if (err != ZIP_OK)
					return do_return("error in closing in the zipfile");
			}
			errclose = zipClose(zf, NULL);
			if (errclose != ZIP_OK)
				return do_return("zipClose failed");;

			ret = true;
			return do_return();
		}

		return do_return("unkonw error");
	}


	QString GetFileVertion(QString fullName)
	{
		QString result = "0.0.0.0";
#ifdef WIN32
		DWORD dwlen = GetFileVersionInfoSizeA(fullName.toStdString().c_str(), 0);

		if (0 == dwlen)
			return result;

		std::string data;
		data.resize(dwlen + 1);
		BOOL bSuccess = GetFileVersionInfoA(fullName.toStdString().c_str(), 0, dwlen, data.data());

		if (false == bSuccess)
			return result;

		LPVOID lpBuffer = nullptr;
		UINT uLen = 0;
		struct LANGANDCODEPAGE
		{
			WORD wLanguage;
			WORD wCodePage;
		}*lpTranslate;

		bSuccess = VerQueryValue(data.data(), (TEXT("\\VarFileInfo\\Translation")), (LPVOID*)&lpTranslate, &uLen);
		if (false == bSuccess)
			return result;

		QString str1, str2;
		str1.setNum(lpTranslate->wLanguage, 16);
		str2.setNum(lpTranslate->wCodePage, 16);
		str1 = "000" + str1;
		str2 = "000" + str2;
		QString verPath = "\\StringFileInfo\\" + str1.right(4) + str2.right(4) + "\\FileVersion";
		bSuccess = VerQueryValueA(data.data(), (verPath.toStdString().c_str()), &lpBuffer, &uLen);
		if (false == bSuccess)
			return result;

		result = QString::fromLocal8Bit((char*)lpBuffer);
#endif
		return result;
	}

	QString md5sum(const QString& path, std::atomic_bool& abort)
	{
		const auto readBufferSize = 512 * 1024;
		static QByteArray buffer(readBufferSize, 0);

		QFile infile{ path };
		if (infile.open(QIODevice::ReadOnly))
		{
			QCryptographicHash hash(QCryptographicHash::Md5);
			if (!infile.atEnd())
			{
				for (; !abort;)
				{
					auto readBytes = infile.read(buffer.data(), readBufferSize);
					if (readBytes == 0)
						break;
					buffer.resize(readBytes);
					hash.addData(buffer);
				}

				return hash.result().toHex();
			}
		}

		return {};
	}

}
