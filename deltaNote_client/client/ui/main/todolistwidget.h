/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#ifndef NOTE_CLIENT_TODO_LIST_WINWIDGET_H
#define NOTE_CLIENT_TODO_LIST_WINWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QCursor>

class TodoListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit TodoListWidget(QWidget *parent = nullptr);
    ~TodoListWidget();

signals:
    void addSignal();
    void todoListCursorLoseSignal();

private:
    void mousePressEvent(QMouseEvent *event);
};

#endif // NOTE_CLIENT_TODO_LIST_WINWIDGET_H
