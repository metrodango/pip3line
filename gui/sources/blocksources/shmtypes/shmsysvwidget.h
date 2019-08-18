#ifndef SHMSYSVWIDGET_H
#define SHMSYSVWIDGET_H



#include <QValidator>
#include <QWidget>

#ifdef Q_OS_UNIX

class SysV_Shm;
namespace Ui {
    class ShmSysVWidget;
}

class KeyValidator : public QValidator
{
        Q_OBJECT
    public:
        explicit KeyValidator(QObject *parent = nullptr);
        QValidator::State validate(QString & input, int & pos) const override;
        static int convert(QString input, bool *ok);
    private:
        Q_DISABLE_COPY(KeyValidator)
};


class ShmSysVWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ShmSysVWidget(SysV_Shm *obj, QWidget *parent = nullptr);
        ~ShmSysVWidget() override;
    public slots:
        void onApply();
        void onFilenameChanged(const QString &value);
    signals:
        void requestReset();
    private:
        Q_DISABLE_COPY(ShmSysVWidget)
        void showEvent(QShowEvent *event) override;
        Ui::ShmSysVWidget *ui;
        SysV_Shm *connector;
        KeyValidator numberVal;
};

#endif //Q_OS_UNIX

#endif // SHMSYSVWIDGET_H
