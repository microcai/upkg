#pragma once
#include <QStyledItemDelegate>

class QCommonDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit QCommonDelegate(QObject* parent) : QStyledItemDelegate(parent) {}
	~QCommonDelegate() = default;

private:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyleOptionViewItem viewOption{ option };
		initStyleOption(&viewOption, index);

		if (option.state.testFlag(QStyle::State_HasFocus))
			viewOption.state = viewOption.state ^ QStyle::State_HasFocus;

		QStyledItemDelegate::paint(painter, viewOption, index);
	}
};
