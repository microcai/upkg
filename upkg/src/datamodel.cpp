#include "upkg/datamodel.hpp"

Datamodel::Datamodel(QObject *parent)
	: QAbstractTableModel(parent)
{
}

void Datamodel::insertData(const ModelData& data)
{
	beginResetModel();
	m_data.push_back(data);
	endResetModel();
}

void Datamodel::insertData(const std::vector<ModelData>& data)
{
	beginResetModel();
	for (const auto& d : data)
		m_data.push_back(d);
	endResetModel();
}

void Datamodel::deleteAllData()
{
	beginResetModel();
	m_data.clear();
	endResetModel();
}

int Datamodel::rowCount(const QModelIndex& parent) const
{
	return (int)m_data.size();
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
				return QStringLiteral("压缩文件MD5");
			case 4:
				return QStringLiteral("文件大小");
			case 5:
				return QStringLiteral("压缩后大小");
			case 6:
				return QStringLiteral("检查存在");
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
	if (role == Qt::DisplayRole)
		return columnData(m_data[index.row()], index.column());
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
		return data.m_check;
	case 7:
		return data.m_url;
	default:
		break;
	}

	return {};
}

void Datamodel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
	if (m_data.empty())
		return;

	const auto asc = order == Qt::AscendingOrder;

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
