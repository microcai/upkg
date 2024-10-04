#include "upkg/datamodel.hpp"

#include "upkg/qcommondelegate.hpp"
#include "upkg/url_view.hpp"
#include "upkg/misc.hpp"
#include "upkg/upkg.hpp"

#include <fstream>
#include <boost/json/src.hpp>

#include <QLocale>
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

int Datamodel::work(const QString& url,
	const QDir& inputDir, const QDir& outputDir,
	const QString& xmlFileName,
	QProgressBar* progressBar,
	std::atomic_bool& abort)
{
	QFileInfo xmlPath(xmlFileName);
	QString ext = xmlPath.completeSuffix();
	if (ext != "xml") ext = QStringLiteral(".xml"); else ext = QStringLiteral("");
	QFile file(outputDir.absolutePath() + QStringLiteral("/") + xmlFileName + ext);
	file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);

	using boost::json::object;
	using boost::json::value;
	using boost::json::array;

	object obj;

	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();

	int count = 0;

	for (size_t i = 0; i < m_data_count; i++)
	{
		std::shared_lock lock(m_lock);
		const auto& data = m_data[i];
		if (data.m_remove)
			continue;
		count++;
	}

	stream.writeStartElement("update_root");
	stream.writeAttribute("count", QString::number(count));
	obj["count"] = count;

	auto time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
	stream.writeAttribute("createtime", time);
	obj["createtime"] = time.toStdString();

	Q_EMIT mainWindow->workProgress(0);

	array ar;
	for (size_t i = 0; i < m_data_count; i++)
	{
		if (abort)
			break;

		ModelData data;

		{
			std::unique_lock lock(m_lock);
			data = m_data[i];

			if (data.m_remove)
				continue;
		}

		QDir dir(data.m_filepath);
		QString relative = data.m_filepath;
		relative.replace(inputDir.absolutePath(), "");
		QString ext;
		if (data.m_compress)
			ext = tr(".") + data.m_file_type;
		data.m_zipfilepath = dir.absolutePath().replace(inputDir.absolutePath(), outputDir.absolutePath()) + ext;
		auto mdir = std::filesystem::path(data.m_zipfilepath.toStdWString()).parent_path();
		std::error_code ignore_ec;
		std::filesystem::create_directories(mdir, ignore_ec);
		QFile::remove(data.m_zipfilepath);
		if (data.m_compress)
			util::compress_zip(data.m_filepath.toLocal8Bit().data(), data.m_zipfilepath.toLocal8Bit().data());
		else
			QFile::copy(data.m_filepath, data.m_zipfilepath);
		data.m_zipmd5 = util::md5sum(data.m_zipfilepath, abort);
		data.m_zipfilesize = QFile(data.m_zipfilepath).size();
		QString prefix = "/";
		QString baseUrl = url;
		if (baseUrl.right(1) == "/") baseUrl = baseUrl.left(baseUrl.size() - 1);
		if (relative.left(1) == "/") relative = relative.mid(1);
		data.m_url = baseUrl + prefix + relative + ext;

		updateData(data);

		object item;
		stream.writeStartElement("file");
		stream.writeAttribute("name", relative);
		item["name"] = relative.toStdString();
		stream.writeAttribute("version", data.m_fileversion);
		item["version"] = data.m_fileversion.toStdString();
		stream.writeAttribute("size", QString::number(data.m_filesize));
		item["size"] = data.m_filesize;
		stream.writeAttribute("zipsize", QString::number(data.m_zipfilesize));
		item["zipsize"] = data.m_zipfilesize;
		stream.writeAttribute("md5", data.m_md5);
		item["md5"] = data.m_md5.toStdString();
		stream.writeAttribute("filehash", data.m_zipmd5);
		item["filehash"] = data.m_zipmd5.toStdString();
		if (data.m_compress)
		{
			stream.writeAttribute("compress", data.m_file_type);
			item["compress"] = data.m_file_type.toStdString();
		}
		else
		{
			stream.writeAttribute("compress", "none");
			item["compress"] = "none";
		}
		stream.writeAttribute("url", data.m_url);
		item["url"] = data.m_url.toStdString();
		stream.writeEndElement();

		ar.push_back(item);

		dataChanged(index(static_cast<int>(i), 0), index(static_cast<int>(i), 6));

		Q_EMIT mainWindow->workProgress(static_cast<int>(i + 1));
	}

	obj["files"] = ar;

	auto json_filename = std::filesystem::path(outputDir.absolutePath().toStdWString());
	json_filename /= xmlPath.baseName().toStdWString() + L".json";

	std::ofstream f(std::filesystem::path(json_filename), std::ios_base::trunc);
	f << obj;
	f.close();

	stream.writeEndElement();
	stream.writeEndDocument();
	file.close();

	return count;
}

int Datamodel::rowCount(const QModelIndex&/* parent*/) const
{
	return (int)m_data_count;
}

int Datamodel::columnCount(const QModelIndex& parent) const
{
	return 8;
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
				return QStringLiteral("是否压缩");
			case 4:
				return QStringLiteral("压缩文件MD5");
			case 5:
				return QStringLiteral("文件大小");
			case 6:
				return QStringLiteral("压缩后大小");
			case 7:
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
	int row = index.row();
	int col = index.column();

	if (role == Qt::DisplayRole)
	{
		std::shared_lock lock(m_lock);
		return columnData(m_data[row], col);
	}

	if (role == Qt::UserRole)
	{
		std::shared_lock lock(m_lock);
		return m_data[row].m_filepath;
	}

	if (role == Qt::CheckStateRole)
	{
		if (col == 0)
		{
			std::shared_lock lock(m_lock);
			if (!m_data[row].m_remove)
				return Qt::Checked;
			return Qt::Unchecked;
		}

		if (col == 3)
		{
			std::shared_lock lock(m_lock);
			if (m_data[row].m_compress)
				return Qt::Checked;
			return Qt::Unchecked;
		}
	}

	if (role == Qt::ToolTipRole)
	{
		if (col == 0)
		{
			std::shared_lock lock(m_lock);
#ifdef WIN32
			return QDir::toNativeSeparators(QDir(m_data[row].m_filepath).absolutePath());
#else
			return m_data[row].m_filepath;
#endif
		}
		else if (col == 3)
		{
			std::shared_lock lock(m_lock);
			if (columnData(m_data[row], col).toBool())
				return tr("需要压缩");
			return tr("不用压缩");
		}
		else if (col == 5 || col == 6)
		{
			QLocale locale{QLocale::Chinese};
			auto size = columnData(m_data[row], col).toString().toULongLong();
			QString valueText = locale.formattedDataSize(size);
			return valueText;
		}
		else
		{
			std::shared_lock lock(m_lock);
			return columnData(m_data[row], col);
		}
	}

	if (role == Qt::EditRole)
	{
		if (col == 7)
		{
			std::shared_lock lock(m_lock);
			return columnData(m_data[row], col);
		}
	}

	return {};
}

bool Datamodel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
		return false;

	int row = index.row();
	int col = index.column();

	if (role == Qt::CheckStateRole)
	{
		if (col == 0)
		{
			if ((Qt::CheckState)value.toInt() == Qt::Checked)
			{
				std::unique_lock lock(m_lock);
				m_data[row].m_remove = false;
			}
			else
			{
				std::unique_lock lock(m_lock);
				m_data[row].m_remove = true;
			}
		}

		if (col == 3)
		{
			if ((Qt::CheckState)value.toInt() == Qt::Checked)
			{
				std::unique_lock lock(m_lock);
				m_data[row].m_compress = true;
			}
			else
			{
				std::unique_lock lock(m_lock);
				m_data[row].m_compress = false;
			}
		}
	}

	if (role == Qt::EditRole)
	{
		std::unique_lock lock(m_lock);
		m_data[row].m_url = value.toString();
	}

	emit dataChanged(index, index);
	return true;
}

QVariant Datamodel::columnData(const ModelData& data, int index) const
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
		return QString::number(data.m_compress);
    case 4:
		return data.m_zipmd5;
	case 5:
		return data.m_filesize;
	case 6:
		return data.m_zipfilesize;
	case 7:
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

	QString tmp;
	const auto columnMember = [&tmp](const ModelData& data, int index) -> const QString& {
		switch (index)
		{
		case 0: return data.m_filename;
		case 1: return data.m_fileversion;
		case 2: return data.m_md5;
		case 4: return data.m_zipmd5;
		case 7: return data.m_url;
		default:
			break;
		}

		Q_ASSERT(false && "columnMember index incorrect!");
		return tmp;
	};

	std::sort(m_data.begin(), m_data.end(), [column, asc, columnMember, this](const auto& left, const auto& right)
	{
		if (column == 3 || column == 5 || column == 6)
		{
			int64_t i = 0;
			int64_t j = 0;
			switch (column)
			{
			case 3:
				i = left.m_compress ? 1 : 0;
				j = right.m_compress ? 1 : 0;
				break;
			case 5:
				i = left.m_filesize;
				j = right.m_filesize;
				break;
			case 6:
				i = left.m_zipfilesize;
				j = right.m_zipfilesize;
				break;
			default:
				break;
			}

			return asc ? (i < j) : (i > j);
		}

		const auto& lvar = columnMember(left, column);
		const auto& rvar = columnMember(right, column);

		return asc ? (lvar < rvar) : (lvar > rvar);
	});

	dataChanged(index(0, 0), index((int)m_data.size() - 1, 7));
}

Qt::ItemFlags Datamodel::flags(const QModelIndex& index) const
{
	int col = index.column();
	if (col == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
	if (col == 3)
		return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
	if (col == 7)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	return QAbstractTableModel::flags(index);
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
