﻿#pragma once

#include <vector>
#include <algorithm>

#include <QVector>
#include <QAbstractTableModel>

struct ModelData
{
	QString m_filepath;
	QString m_filename;
	QString m_fileversion;
	QString m_md5;
	QString m_zipmd5;
	QString m_filesize;
	QString m_zipfilesize;
	QString m_file_type;
	QString m_check;
	QString m_url;
};

class Datamodel : public QAbstractTableModel
{
	Q_OBJECT

public:
	Datamodel(QObject *parent = 0);

	void insertData(const ModelData& data);
	void insertData(const std::vector<ModelData>& data);
	void deleteAllData();

	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;

	QString columnData(const ModelData& data, int index) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const override;
	virtual QVariant data(const QModelIndex& index, int role /* = Qt::DisplayRole */) const override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
	std::vector<ModelData> m_data;
};
