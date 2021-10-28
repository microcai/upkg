﻿#include "upkg/datamodel.hpp"

#include "upkg/qcommondelegate.hpp"
#include "upkg/url_parser.hpp"
#include "upkg/misc.hpp"
#include "upkg/upkg.hpp"

#include <QXmlStreamWriter>

extern upkg* mainWindow;

Datamodel::Datamodel(QObject *parent)
	: QAbstractTableModel(parent)
{
}

void Datamodel::insertData(const ModelData& data)
{
	std::unique_lock lock(m_lock);
	beginResetModel();
	m_data.push_back(data);
	endResetModel();
	m_data_count = (int)m_data.size();
}

void Datamodel::insertData(const std::vector<ModelData>& data)
{
	std::unique_lock lock(m_lock);
	beginResetModel();
	for (const auto& d : data)
		m_data.push_back(d);
	endResetModel();
	m_data_count = (int)m_data.size();
}

void Datamodel::deleteAllData()
{
	std::unique_lock lock(m_lock);
	beginResetModel();
	m_data.clear();
	endResetModel();
	m_data_count = (int)m_data.size();
}

void Datamodel::work(const QString& url,
	const QDir& inputDir, const QDir& outputDir,
	const QString& xmlFileName,
	QProgressBar* progressBar,
	std::atomic_bool& abort)
{
	QFileInfo xmlPath(xmlFileName);
	QString ext = xmlPath.completeSuffix();
	if (ext != "xml") ext = tr(".xml"); else ext = tr("");
	QFile file(outputDir.absolutePath() + "/" + xmlFileName + ext);
	file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);

	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();

	stream.writeStartElement("update_root");
	stream.writeAttribute("count", QString::number(m_data_count));

	auto time = QDateTime::currentDateTime();
	stream.writeAttribute("createtime", time.toString("yyyy-MM-dd hh:mm:ss.zzz"));

	Q_EMIT mainWindow->workProgress(0);

	for (size_t i = 0; i < m_data_count; i++)
	{
		if (abort)
			break;

		ModelData data;

		{
			std::unique_lock lock(m_lock);
			data = m_data[i];
		}

		QDir dir(data.m_filepath);
		QString relative = data.m_filepath;
		relative.replace(inputDir.absolutePath(), "");

		data.m_zipfilepath = dir.absolutePath().replace(inputDir.absolutePath(), outputDir.absolutePath()) + tr(".") + data.m_file_type;
		auto mdir = std::filesystem::path(data.m_zipfilepath.toStdWString()).parent_path();
		std::error_code ignore_ec;
		std::filesystem::create_directories(mdir, ignore_ec);

		util::compress_zip(data.m_filepath.toLocal8Bit().data(), data.m_zipfilepath.toLocal8Bit().data());
		data.m_zipmd5 = util::md5sum(data.m_zipfilepath, abort);
		data.m_zipfilesize = QString::number(QFile(data.m_zipfilepath).size());
		QString prefix = "/";
		QString baseUrl = url;
		if (baseUrl.right(1) == "/") baseUrl = baseUrl.left(baseUrl.size() - 1);
		if (relative.left(1) == "/") relative = relative.mid(1);
		data.m_url = baseUrl + prefix + relative + tr(".") + data.m_file_type;

		updateData(data);

		stream.writeStartElement("file");
		stream.writeAttribute("name", relative);
		stream.writeAttribute("version", data.m_fileversion);
		stream.writeAttribute("size", data.m_filesize);
		stream.writeAttribute("zipsize", data.m_zipfilesize);
		stream.writeAttribute("md5", data.m_md5);
		stream.writeAttribute("filehash", data.m_zipmd5);
		stream.writeAttribute("compress", data.m_file_type);
		stream.writeAttribute("url", data.m_url);
		stream.writeEndElement();

		dataChanged(index(static_cast<int>(i), 0), index(static_cast<int>(i), 6));

		Q_EMIT mainWindow->workProgress(static_cast<int>(i + 1));
	}

	stream.writeEndElement();
	stream.writeEndDocument();
	file.close();
}

int Datamodel::rowCount(const QModelIndex&/* parent*/) const
{
	return (int)m_data_count;
}

int Datamodel::columnCount(const QModelIndex& parent) const
{
	return 7;
}

QVariant Datamodel::headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const
{
	if (orientation == Qt::Horizontal)
	{
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
			case 0:
				return QStringLiteral("文件名");
			case 1:
				return QStringLiteral("文件版本");
			case 2:
				return QStringLiteral("文件MD5");
			case 3:
				return QStringLiteral("压缩文件MD5");
			case 4:
				return QStringLiteral("文件大小");
			case 5:
				return QStringLiteral("压缩后大小");
			case 6:
				return QStringLiteral("URL");
			default:
				break;
			}

		}
	}
	return {};
}

QVariant Datamodel::data(const QModelIndex& index, int role /* = Qt::DisplayRole */) const
{
	if (role == Qt::DisplayRole)
	{
		qDebug() << "DisplayRole";
		std::shared_lock lock(m_lock);
		return columnData(m_data[index.row()], index.column());
	}
	return {};
}

QString Datamodel::columnData(const ModelData& data, int index) const
{
	switch (index)
	{
	case 0:
		return data.m_filename;
	case 1:
		return data.m_fileversion;
	case 2:
		return data.m_md5;
	case 3:
		return data.m_zipmd5;
	case 4:
		return data.m_filesize;
	case 5:
		return data.m_zipfilesize;
	case 6:
		return data.m_url;
	default:
		break;
	}

	return {};
}

void Datamodel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
	const auto asc = order == Qt::AscendingOrder;

	std::unique_lock lock(m_lock);
	if (m_data.empty())
		return;

	std::sort(m_data.begin(), m_data.end(), [column, asc, this](const auto& left, const auto& right)
	{
		const auto& lvar = columnData(left, column);
		const auto& rvar = columnData(right, column);

		if (column == 4) // file size
		{
			auto i = lvar.toInt();
			auto j = rvar.toInt();

			return asc ? (i < j) : (i > j);
		}

		return asc ? (lvar < rvar) : (lvar > rvar);
	});

	dataChanged(index(0, 0), index((int)m_data.size() - 1, 7));
}

void Datamodel::updateData(const ModelData& data)
{
	std::unique_lock lock(m_lock);
	for (auto& d : m_data)
	{
		if (d.m_filepath == data.m_filepath)
		{
			d = data;
			break;
		}
	}
}