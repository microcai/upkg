#include "upkg/upkg.hpp"
#include <QCryptographicHash>

upkg::upkg(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.InputDirEdit->setReadOnly(true);
	QObject::connect(ui.InputDirBtn, &QPushButton::clicked, [this]() mutable
	{
		QString path = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("请选择源目录"), QDir::currentPath()));
		ui.InputDirEdit->setText(path);
	});

	ui.OutputDirEdit->setReadOnly(true);
	QObject::connect(ui.OutputDirBtn, &QPushButton::clicked, [this]() mutable
	{
		QString path = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("请选择存放目录"), QDir::currentPath()));
		ui.OutputDirEdit->setText(path);
	});
}

upkg::~upkg()
{
}
