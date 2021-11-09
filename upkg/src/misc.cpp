#include "upkg/misc.hpp"
#include "fmt/chrono.h"

#include <sys/stat.h>

#include <ctime>
#include <boost/filesystem.hpp>

#ifdef _MSC_VER
#	include <windows.h>
#	pragma comment(lib, "Version")
#endif

namespace util {

	inline uLong tm2dosdate(const tm_zip* ptm)
	{
		uLong year = (uLong)ptm->tm_year;
		if (year >= 1980)
			year -= 1980;
		else if (year >= 80)
			year -= 80;
		return
			(uLong)(((ptm->tm_mday) + (32 * (ptm->tm_mon + 1)) + (512 * year)) << 16) |
			((ptm->tm_sec / 2) + (32 * ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
	}

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
		std::vector<uint8_t> zipbuffer(size_buf, 0);
		void* buf = (void*)zipbuffer.data();
		const char* password = NULL;
		FILE* fin = NULL;

#ifdef WIN32
		struct _stat s = { 0 };
		_stat(inFile, &s);
#elif __linux__
		struct stat s = { 0 };
		stat(inFile, &s);

		zi.external_fa = (s.st_mode << 16) | !(s.st_mode & S_IWRITE);

#define MSDOS_DIR_ATTR 0x10
#define UNX_IFDIR      0040000     /* Unix directory */
#define UNX_IFREG      0100000     /* Unix regular file */
#define UNX_IFSOCK     0140000     /* Unix socket (BSD, not SysV or Amiga) */
#define UNX_IFLNK      0120000     /* Unix symbolic link (not SysV, Amiga) */
#define UNX_IFBLK      0060000     /* Unix block special       (not Amiga) */
#define UNX_IFCHR      0020000     /* Unix character special   (not Amiga) */
#define UNX_IFIFO      0010000     /* Unix fifo    (BCC, not MSC or Amiga) */

		auto legacy_modes = s.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX);
		if (S_ISDIR(s.st_mode))
			legacy_modes |= UNX_IFDIR;
		if (S_ISREG(s.st_mode))
			legacy_modes |= UNX_IFREG;
		if (S_ISLNK(s.st_mode))
			legacy_modes |= UNX_IFLNK;
		if (S_ISBLK(s.st_mode))
			legacy_modes |= UNX_IFBLK;
		if (S_ISCHR(s.st_mode))
			legacy_modes |= UNX_IFCHR;
		if (S_ISFIFO(s.st_mode))
			legacy_modes |= UNX_IFIFO;
		if (S_ISSOCK(s.st_mode))
			legacy_modes |= UNX_IFSOCK;
		zi.external_fa = (legacy_modes << 16) | !(s.st_mode & S_IWRITE);

		if ((s.st_mode & S_IFMT) == S_IFDIR) {
			zi.external_fa |= MSDOS_DIR_ATTR;
		}
#endif // __linux__ / WIN32

		auto timeinfo = fmt::localtime(s.st_mtime);
		zi.tmz_date.tm_year = timeinfo.tm_year;
		zi.tmz_date.tm_mon = timeinfo.tm_mon;
		zi.tmz_date.tm_mday = timeinfo.tm_mday;
		zi.tmz_date.tm_hour = timeinfo.tm_hour;
		zi.tmz_date.tm_min = timeinfo.tm_min;
		zi.tmz_date.tm_sec = timeinfo.tm_sec;

		uint8_t* extra_field = nullptr;
		uInt extra_field_size = 0;
		uint madeby = 0;

#ifdef WIN32
		zi.external_fa = ::GetFileAttributesA(inFile);

		std::vector<uint8_t> extra_buffer(256, 0);
		extra_field = extra_buffer.data();
		*extra_field++ = 0x0a;// 0x55  0x0a
		*extra_field++ = 0x0; // 0x54  0x00
		*extra_field++ = 32;	// size
		*extra_field++ = 0;	// size

		extra_field += 4;	// Reserved
		*extra_field++ = 1;	// 0x01
		*extra_field++ = 0;	// 0x00

		*extra_field++ = 24;	// size
		*extra_field++ = 0;	// size

		uint64_t mtime = s.st_mtime, ctime = s.st_ctime, atime = s.st_atime;
		FILETIME Modft, Accft, Creft;
		HANDLE h = ::CreateFileA(inFile, FILE_READ_ATTRIBUTES, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			::GetFileTime(h, &Creft, &Accft, &Modft);
			::CloseHandle(h);

			ULARGE_INTEGER* u = (ULARGE_INTEGER*)&Creft;
			ctime = u->QuadPart;

			u = (ULARGE_INTEGER*)&Modft;
			mtime = u->QuadPart;

			u = (ULARGE_INTEGER*)&Accft;
			atime = u->QuadPart;
		}

		// ModTime
		*(ULONGLONG*)extra_field = mtime;
		extra_field += 8;
		// AcTime
		*(ULONGLONG*)extra_field = atime;
		extra_field += 8;
		// CrTime
		*(ULONGLONG*)extra_field = ctime;
		extra_field += 8;

		extra_field_size = extra_field - extra_buffer.data();
		extra_field = extra_buffer.data();
		madeby = 0x3f;
#else
		madeby = 0x31e;

#endif // WIN32

		int zip64 = 0;
		if (boost::filesystem::file_size(infile) > 0xffffffff)
			zip64 = 1;

		err = zipOpenNewFileInZip4_64(zf, szFname.c_str(), &zi,
			nullptr, 0, extra_field, extra_field_size, NULL /* comment*/,
			Z_DEFLATED,
			Z_DEFAULT_COMPRESSION, 0,
			/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
			-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
			password, crcFile, madeby, 0, zip64);
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
