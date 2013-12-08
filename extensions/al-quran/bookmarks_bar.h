#ifndef BOOKMARKSBAR_H
#define BOOKMARKSBAR_H

#include <QWidget>
#include <QVector>
#include "bookmark.h"

class QTreeWidgetItem;
namespace Ui {
class BookmarksBar;
}

class BookmarksBar : public QWidget
{
    Q_OBJECT
    
public:
    static const int kBookmarkBarWidth = 200;
    BookmarksBar(const QString& settingsKeyPrefix, QWidget *parent = 0);
    ~BookmarksBar();
signals:
    void selectionChanged(Bookmark* bookmark);
public slots:
    void onSelectionChanged(const QModelIndex&);
    void customContextMenuRequested(const QPoint &pos);
private:
    Ui::BookmarksBar *ui;
    QString m_settingsKeyPrefix;
    QVector<Bookmark> m_bookmarks;
};

#endif // BOOKMARKSBAR_H
