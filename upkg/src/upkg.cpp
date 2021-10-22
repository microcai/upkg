#include "upkg/upkg.hpp"
#include <QCryptographicHash>

upkg::upkg(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// QString blah = QString(QCryptographicHash::hash(("1"), QCryptographicHash::Md5).toHex());
	// this->setWindowTitle(blah);

/*
	ui.inputDirEdit->setReadOnly(true);
	QObject::connect(ui.InputDirBtn, &QPushButton::clicked, [this]() mutable
	{
		QString path = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Input Dir"), QDir::currentPath()));
		ui.inputDirEdit->setText(path);
	});
*/
}

upkg::~upkg()
{
}
