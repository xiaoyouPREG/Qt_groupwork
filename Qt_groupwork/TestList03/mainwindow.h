#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void addRootNode();
    void addChildNode();

    void onSendButtonClicked();  // 用户点击"发送"按钮时触发
    void onApiReplyFinished(QNetworkReply *reply);  // API返回数据时触发

    void on_pushButton_clicked();



private:
    QLineEdit* createLineEditForItem(const QString &initialText);

    QTreeWidget *treeWidget;
    QLineEdit *nodeNameEdit;

    QTextEdit *inputTextEdit;
    QTextEdit *outputTextEdit;

    QNetworkAccessManager *networkManager;  // 用于发送HTTP请求
};

#endif // MAINWINDOW_H
