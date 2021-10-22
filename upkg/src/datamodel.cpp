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
	return 9;
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
				return QStringLiteral("压缩后文件大小");
			case 6:
				return QStringLiteral("压缩方式");
			case 7:
				return QStringLiteral("检查存在");
			case 8:
				return QStringLiteral("自定义URL");
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
		switch (index.column())
		{
		case 0:
			return m_data[index.row()].m_filename;
		case 1:
			return m_data[index.row()].m_fileversion;
		case 2:
			return m_data[index.row()].m_md5;
		case 3:
			return m_data[index.row()].m_zipmd5;
		case 4:
			return m_data[index.row()].m_filesize;
		case 5:
			return m_data[index.row()].m_zipfilesize;
		case 6:
			return m_data[index.row()].m_file_type;
		case 7:
			return m_data[index.row()].m_check;
		case 8:
			return m_data[index.row()].m_url;
		default:
			break;
		}
	}

	return {};
}
