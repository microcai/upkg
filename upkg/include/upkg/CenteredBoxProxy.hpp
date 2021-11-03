
#include <QApplication>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QProxyStyle>
#include <QStyleOptionViewItem>

enum {
  CheckAlignmentRole = Qt::UserRole + Qt::CheckStateRole + Qt::TextAlignmentRole
};

class CenteredBoxProxy : public QProxyStyle{
public:
    QRect subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const override{
        const QRect baseRes = QProxyStyle::subElementRect(element, option, widget);
        if(element==SE_ItemViewItemCheckIndicator){
            const QStyleOptionViewItem* const itemOpt = qstyleoption_cast<const QStyleOptionViewItem*>(option) ;
            Q_ASSERT(itemOpt);
            const QVariant alignData = itemOpt->index.data(CheckAlignmentRole);
            if(alignData.isNull())
               return baseRes;
            const QRect itemRect = option->rect;
            Q_ASSERT(itemRect.width()>baseRes.width() && itemRect.height()>baseRes.height());
            const int alignFlag = alignData.toInt();
            int x=0,y=0;
            if(alignFlag & Qt::AlignLeft){
                x=baseRes.x();
            }
            else if(alignFlag & Qt::AlignRight){
                x=itemRect.x() + itemRect.width() - (baseRes.x()-itemRect.x())-baseRes.width();
            }
            else if(alignFlag & Qt::AlignHCenter){
                x=itemRect.x() + (itemRect.width()/2)-(baseRes.width()/2);
            }
            return QRect(QPoint(x,baseRes.y()), baseRes.size());
        }
        return baseRes;
    }
};

