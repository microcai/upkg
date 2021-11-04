#pragma once

#include <vector>
#include <algorithm>
#include <shared_mutex>

#include <QVector>
#include <QAbstractTableModel>
#include <QProgressBar>
#include <QDir>

struct ModelData
{
	QString m_filepath;
	QString m_filename;
	QString m_fileversion;
	QString m_md5;
	QString m_zipmd5;
	QString m_zipfilepath;
	QString m_filesize;
	QString m_zipfilesize;
	QString m_file_type;
	bool m_compress{true};
	bool m_remove{false};
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

	int work(const QString& url,
		const QDir& inputDir, const QDir& outputDir,
		const QString& xmlFileName,
		QProgressBar* progressBar,
		std::atomic_bool& abort);

	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;

	QVariant columnData(const ModelData& data, int index) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const override;
	virtual QVariant data(const QModelIndex& index, int role /* = Qt::DisplayRole */) const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
	void updateData(const ModelData& data);

private:
	mutable std::shared_mutex m_lock;
	std::atomic_int m_data_count{ 0 };
	std::vector<ModelData> m_data;
};
