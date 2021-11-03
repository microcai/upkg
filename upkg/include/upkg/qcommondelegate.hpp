#pragma once
#include <QStyledItemDelegate>
#include <QCheckBox>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

extern QFont* globalDefaultFont;
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

		if (index.column() == 3)
		{
// 			viewOption.text = tr("");
// 			const QWidget* widget = viewOption.widget;
// 			QStyle* style = widget ? widget->style() : QApplication::style();
// 			style->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, widget);

			bool data = index.model()->data(index, Qt::UserRole).toBool();
			QStyleOptionButton checkbox;

			checkbox.state = data ? QStyle::State_On : QStyle::State_Off;
			checkbox.state |= QStyle::State_Enabled;

			auto x = viewOption.fontMetrics.horizontalAdvance("A");
			viewOption.rect.setTop(viewOption.rect.top() + x);
			viewOption.rect.setBottom(viewOption.rect.bottom() - x);

			checkbox.rect = viewOption.rect;

			const QWidget* widget = viewOption.widget;
			QStyle* style = widget ? widget->style() : QApplication::style();

			QCheckBox qcb;
			qcb.setFont(*globalDefaultFont);
			qcb.setGeometry(0, 0, 10, 10);
			// qcb.setStyleSheet(QString("QCheckBox::indicator {width:10px;height: 10px;}"));
			style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkbox, painter, &qcb);

			// qDebug() << option.rect;

// 			painter->save();
// 			QBrush brush(QColor(255,0,255));
// 			QPoint topLeft = option.rect.topLeft();
// 			QPoint bottomRight = option.rect.topRight();
// 			QLinearGradient backgroundGradient(topLeft, bottomRight);
// 			backgroundGradient.setColorAt(0.0, QColor(Qt::yellow).lighter(190));
// 			backgroundGradient.setColorAt(1.0, Qt::white);
//
// 			painter->fillRect(option.rect, brush);
//
// 			painter->restore();
			// checkbox.palette.setBrush(QPalette::ColorRole::Button, brush);
			// checkbox.rect.setLeft(checkbox.rect.left() + checkbox.rect.width() / 2 - 20);

//  			const QWidget* widget = viewOption.widget;
//  			QStyle* style = widget ? widget->style() : QApplication::style();
//  			style->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, widget);

			// style->drawControl(QStyle::CE_CheckBox, &checkbox, painter);
			// QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkbox, painter);

// 			QStyleOptionButton checkBoxStyle;
// 			checkBoxStyle.state = data ? QStyle::State_On : QStyle::State_Off;
// 			checkBoxStyle.state |= QStyle::State_Enabled;
// 			checkBoxStyle.state |= (data ? QStyle::State_Selected : QStyle::State_None);
// 			checkBoxStyle.rect = option.rect;
// 			checkBoxStyle.rect.setTop(checkBoxStyle.rect.top() + 8);
// 			checkBoxStyle.rect.setLeft(checkBoxStyle.rect.left() + 8);
// 			checkBoxStyle.rect.setBottom(checkBoxStyle.rect.bottom() - 8);
// 			checkBoxStyle.rect.setRight(checkBoxStyle.rect.right() - 8);
// 			QCheckBox checkBox;
// 			QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxStyle, painter, &checkBox);
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
		if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
			&& decorationRect.contains(mouseEvent->pos()))
		{
			if (index.column() == 3)
			{
				auto data = model->data(index, Qt::UserRole).toInt();
				model->setData(index, data ? Qt::Unchecked : Qt::Checked, Qt::UserRole);
			}
		}
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
};

