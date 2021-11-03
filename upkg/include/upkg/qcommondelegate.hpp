#pragma once
#include <QProxyStyle>
#include <QStyledItemDelegate>
#include <QCheckBox>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

extern QFont* globalDefaultFont;

class QCheckBoxProxy : public QProxyStyle
{
	QRect subElementRect(QStyle::SubElement element, const QStyleOption* option, const QWidget* widget) const override
	{
		QRect baseRect = QProxyStyle::subElementRect(element, option, widget);
		if (element == QStyle::SE_ItemViewItemCheckIndicator)
		{
			auto itemRect = option->rect;
			QPoint center;
			center.setX(itemRect.x() + itemRect.width() / 2);
			center.setY(itemRect.y() + itemRect.height() / 2);

			baseRect.moveCenter(center);
		}

		if (element == QStyle::SE_ItemViewItemFocusRect)
			return option->rect;

		return baseRect;
	}
};

class QCommonDelegate : public QStyledItemDelegate
{
	Q_OBJECT

private:
	QCheckBoxProxy checkBoxProxy;

public:
	explicit QCommonDelegate(QObject* parent) : QStyledItemDelegate(parent) {}
	~QCommonDelegate() = default;

private:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyleOptionViewItem viewOption{ option };
		initStyleOption(&viewOption, index);

		// Remove Focus...
 		if (option.state.testFlag(QStyle::State_HasFocus))
 			viewOption.state = viewOption.state ^ QStyle::State_HasFocus;

		if (index.column() == 3)
		{
			// Remove check box title.
 			viewOption.text = tr("");

			// Use checkBoxProxy.drawControl...
 			const QWidget* widget = viewOption.widget;
			checkBoxProxy.drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, widget);
		}
		else
		{
			QStyledItemDelegate::paint(painter, viewOption, index);
		}
	}

	bool editorEvent(QEvent* event, QAbstractItemModel* model,
		const QStyleOptionViewItem& option, const QModelIndex& index) override
	{
		QRect decorationRect = option.rect;
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if ((event->type() == QEvent::MouseButtonPress)
			&& decorationRect.contains(mouseEvent->pos()))
		{
			if (index.column() == 3)
			{
				auto data = model->data(index, Qt::CheckStateRole).toInt();
				model->setData(index, data ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
			}
		}
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
};

