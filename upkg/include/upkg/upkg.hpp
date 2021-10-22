#pragma once
// #pragma execution_character_set("utf-8")

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

#include "ui_upkg.h"

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

private:
	Ui::upkgClass m_ui;
	QSettings m_settings;
};
