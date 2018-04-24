#include "RichTextItemDelegate.h"

#include <QDebug>
#include <QPainter>
#include <QTextDocument>

RichTextItemDelegate::RichTextItemDelegate(QObject *parent) : QItemDelegate(parent)
{
}

void RichTextItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    _paintingIndex = index;
    QItemDelegate::paint(painter, option, index);
}

void RichTextItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    Q_UNUSED(option);
    Q_UNUSED(text);
    Q_ASSERT(_paintingIndex.isValid());

    QTextDocument *doc = document(option, _paintingIndex);

    painter->save();
    QRectF textClipRect;
    qreal textOffsetX;
    if (Qt::AlignHCenter & option.displayAlignment)
    {
        QSizeF docSize = doc->size();
        textOffsetX = (rect.width() - docSize.width())/2;
        textClipRect.setRect(0, 0, docSize.width(), rect.height());
    }
    else // we don't use another alignments, extend when needed
    {
        textOffsetX = 0;
        textClipRect.setRect(0, 0, rect.width(), rect.height());
    }
    qreal textOffsetY = 0;
#if defined(Q_OS_WIN) or defined(Q_OS_MAC)
    // Workaround for an issue when rich text is rendered slightly upper
    // then text in neighbour cell having the same font but not using rich-text delegate.
    // For example look at elemnets table, columns Label (simple) and Parameters (rich-text).
    textOffsetY = 1;
#endif
    painter->translate(rect.left() + textOffsetX,  rect.top() + textOffsetY);
    doc->drawContents(painter, textClipRect);
    painter->restore();
    delete doc;
}

QSize RichTextItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);

    QTextDocument *doc = document(option, index);

    QRect decorationRect = rect(option, index, Qt::DecorationRole);
    QRect displayRect = QRect(0, 0, doc->size().width(), doc->size().height());
    QRect checkRect = rect(option, index, Qt::CheckStateRole);

    doLayout(option, &checkRect, &decorationRect, &displayRect, true);

    delete doc;

    return (decorationRect|displayRect|checkRect).size();
}

QTextDocument* RichTextItemDelegate::document(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    QFont font = qvariant_cast<QFont>(index.data(Qt::FontRole)).resolve(option.font);

    QTextDocument *doc = new QTextDocument;
    doc->setDefaultFont(font);
    doc->setTextWidth(-1);
    QString text = index.data(Qt::DisplayRole).toString();
    if ((QStyle::State_Selected & option.state) && (QStyle::State_Active & option.state))
    {
        // change any explicitly specified color to highlighted text color
        auto color = option.palette.color(QPalette::HighlightedText);
        auto colorStyle = QStringLiteral("color:%1").arg(color.name(QColor::HexRgb));
        static QRegExp colorEntry("color: *#[a-fA-F\\d]+"); // only #rrggbb color format is replaced
        text.replace(colorEntry, colorStyle);
        text = QStringLiteral("<span style='%1'>%2</span>").arg(colorStyle, text);
    }
    doc->setHtml(text);
    return doc;
}