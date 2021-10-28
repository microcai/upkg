#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QSettings>
#include <QScrollBar>
#include <QWheelEvent>
#include <QCryptographicHash>
#include <QFile>
#include <QtWidgets/QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFuture>
#include <QtConcurrent>
#include <QStyledItemDelegate>
#include <QProgressBar>

#include "ui_upkg.h"
#include "upkg/datamodel.hpp"


class upkg : public QMainWindow
{
	Q_OBJECT
public:
	upkg(QWidget *parent = Q_NULLPTR);
	~upkg();

public:

private:
	void loadSettings() noexcept;
	void saveSettings() noexcept;

	void loadDir() noexcept;

	QFileInfoList walkDir(const QDir& dir);

public:
signals:
	void scanDir(QDir);
	void initWork();
	void workDir();
	void workProgress(int value);

private:
	Ui::upkgClass m_ui;
	QSettings m_settings;
	Datamodel* m_datamodel;
	QProgressBar* m_progressBar;
	QLabel* m_statusLable;
	QFuture<void> m_scanning_thrd;
	QFuture<void> m_working_thrd;
	std::atomic_bool m_abort{ false };
};
