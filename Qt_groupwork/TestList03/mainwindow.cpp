#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
 , networkManager(new QNetworkAccessManager(this))
{
    // 创建主部件和布局
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    QVBoxLayout *leftLayout = new QVBoxLayout(centralWidget);
    QVBoxLayout *rightLayout = new QVBoxLayout(centralWidget);

    // 创建树控件
    treeWidget = new QTreeWidget(this);
    treeWidget->setHeaderLabel("Tree Structure");

    // 创建按钮和输入框
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addRootButton = new QPushButton("Add Root Node", this);
    QPushButton *addChildButton = new QPushButton("Add Leaf Node", this);
    QPushButton *pictureButton = new QPushButton("Make Mind Map",this);
    nodeNameEdit = new QLineEdit(this);
    nodeNameEdit->setPlaceholderText("Enter node name");

    QHBoxLayout *rbuttonLayout = new QHBoxLayout();
    outputTextEdit = new QTextEdit(this);
    QPushButton *sendButton = new QPushButton("Send", this);
    QPushButton *pushButton = new QPushButton("Clear", this);
    inputTextEdit = new QTextEdit(this);

    buttonLayout->addWidget(nodeNameEdit);
    buttonLayout->addWidget(addRootButton);
    buttonLayout->addWidget(addChildButton);
    buttonLayout->addWidget(pictureButton);

    rbuttonLayout->addWidget(sendButton);
    rbuttonLayout->addWidget(pushButton);

    // 添加到主布局
    leftLayout->addWidget(treeWidget);
    leftLayout->addLayout(buttonLayout);
    rightLayout->addWidget(inputTextEdit);
    rightLayout->addLayout(rbuttonLayout);
    rightLayout->addWidget(outputTextEdit);

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    setCentralWidget(centralWidget);

    inputTextEdit->setText("chat with deepseek!");

    // 连接信号和槽
    connect(addRootButton, &QPushButton::clicked, this, &MainWindow::addRootNode);
    connect(addChildButton, &QPushButton::clicked, this, &MainWindow::addChildNode);

    // 连接按钮点击事件
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);

    // 连接网络请求完成信号
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onApiReplyFinished);

}

void MainWindow::addRootNode()
{
    QString name = nodeNameEdit->text().isEmpty() ? "Root Node" : nodeNameEdit->text();
    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
    item->setText(0, name);

    // 为根节点添加可编辑的 QLineEdit
    treeWidget->setItemWidget(item, 0, createLineEditForItem(name));
}

void MainWindow::addChildNode()
{
    QTreeWidgetItem *selectedItem = treeWidget->currentItem();
    if (!selectedItem) {
        addRootNode();
        return;
    }

    QString name = nodeNameEdit->text().isEmpty() ? "Child Node" : nodeNameEdit->text();
    QTreeWidgetItem *childItem = new QTreeWidgetItem();
    childItem->setText(0, name);
    selectedItem->addChild(childItem);

    // 展开父节点以显示子节点
    selectedItem->setExpanded(true);

    // 为子节点添加可编辑的 QLineEdit
    treeWidget->setItemWidget(childItem, 0, createLineEditForItem(name));
}

QLineEdit* MainWindow::createLineEditForItem(const QString &initialText)
{
    QLineEdit *lineEdit = new QLineEdit(initialText);
    lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");

    // 当编辑完成时更新节点文本
    connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit]() {
        QTreeWidgetItem *item = treeWidget->currentItem();
        if (item) {
            item->setText(0, lineEdit->text());
        }
    });

    return lineEdit;
}


void MainWindow::onSendButtonClicked()
{
    QString originalInput = inputTextEdit->toPlainText().trimmed();
    QString hint ="我需要生成一个思维导图，思维导图的形式是树 ，有至少两个层级,请务必严格按照以下格式输出：按照顺序依次输出各个节点,在每个节点前用#的数量来表示层级.例如：#根节点\n##父节点1\n###子节点1-1\n###子节点1-2\n##父节点2\n###子节点2-1\n####子子节点2-1-1\n且务必千万不要输出任何其他内容!以下为思维导图的生成提示：\"";

    QString userInput = hint + originalInput +"\"";
    if (userInput.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入内容！");
        return;
    }

    // DeepSeek API 请求URL（请替换为实际的API地址）
    QString apiUrl = "https://api.deepseek.com/v1/chat/completions";  // 示例URL，请参考DeepSeek官方文档

    // 构造请求头
    QNetworkRequest request;
    request.setUrl(QUrl(apiUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer ##############################");  // 替换为你的DeepSeek API Key

    // 构造请求体（JSON格式）
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";  // 根据DeepSeek API文档调整
    requestBody["messages"] = QJsonArray{
        QJsonObject{
            {"role", "user"},
            {"content", userInput}
        }
    };

    QJsonDocument doc(requestBody);
    QByteArray data = doc.toJson();





    // 发送POST请求
    networkManager->post(request, data);
}

void MainWindow::onApiReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(this, "错误", "API请求失败: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    // 读取API返回的JSON数据
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();

    // 提取DeepSeek的回复（假设API返回格式为 {"choices": [{"message": {"content": "..."}}]}）
    QString replyText = jsonObj["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();

    // 在输出框中显示回复
    outputTextEdit->append("DeepSeek: " + replyText);
    QStringList parts = replyText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int prev_jing = 0;
    QTreeWidgetItem *curr_Item = nullptr;

    for(QString q : parts) {
        int num_jing = 0;
        while(!q.isEmpty() && q[0] == '#') {
            num_jing++;
            q.remove(0, 1);
        }

        if(q.isEmpty() || num_jing == 0) continue;
        if(num_jing > prev_jing + 1) {
            qDebug() << "Invalid level jump from" << prev_jing << "to" << num_jing;
            continue;
        }
        if(num_jing <= prev_jing) {
            int steps_back = prev_jing - num_jing + 1;
            while (steps_back-- > 0 && curr_Item) {
                curr_Item = curr_Item->parent();
            }
        }

        QTreeWidgetItem *newItem = new QTreeWidgetItem();
        newItem->setText(0, q);
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

        if(num_jing == 1) {
            treeWidget->addTopLevelItem(newItem);
        } else {
            if(curr_Item) curr_Item->addChild(newItem);
        }

        curr_Item = newItem;
        prev_jing = num_jing;
    }
    treeWidget->expandAll();

    reply->deleteLater();
}

void MainWindow::on_pushButton_clicked()
{
    outputTextEdit->clear();
}
