#ifndef CONNECTIONSINFOSWIDGET_H
#define CONNECTIONSINFOSWIDGET_H

#include <QWidget>
#include <QLabel>

namespace Ui {
    class ConnectionsInfosWidget;
}

class CertificatesModel;
class BlocksSource;
class QTableView;

class ConnectionsInfosWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ConnectionsInfosWidget(BlocksSource *listener, QWidget *parent = nullptr);
        ~ConnectionsInfosWidget();
    private slots:
        void onConnectionUpdated();
    private:
        Q_DISABLE_COPY(ConnectionsInfosWidget)
        Ui::ConnectionsInfosWidget *ui;
        CertificatesModel *peerCertsModel;
        bool enableSSLViews;
        BlocksSource *listener;
        QLabel *sslCipherLabel;
        QTableView * certTable;
};

#endif // CONNECTIONSINFOSWIDGET_H
