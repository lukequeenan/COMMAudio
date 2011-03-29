#ifndef COLORS_H
#define COLORS_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
    class Colors;
}

class Colors : public QDialog
{
    Q_OBJECT

public:
    explicit Colors(QWidget *parent = 0);
    ~Colors();

    QString getFont();
    QString getBackground();
private slots:
    void on_textWidget_itemDoubleClicked(QListWidgetItem* item);

    void on_backgroundWidget_itemDoubleClicked(QListWidgetItem* item);

    void on_defaultButton_clicked();

    void on_tigerButton_clicked();

private:
    Ui::Colors *ui;
    QString backgroundColor_;
    QString fontColor_;
};

#endif // COLORS_H
