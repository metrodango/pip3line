#ifndef CRC32WIDGET_H
#define CRC32WIDGET_H

#include <QWidget>
#include "../crc32.h"

namespace Ui {
    class Crc32Widget;
}

class Crc32Widget : public QWidget
{
        Q_OBJECT

    public:
        explicit Crc32Widget(Crc32 *transform, QWidget *parent = 0);
        ~Crc32Widget();
    private slots:
        void onLittleendianToggled(bool checked);
        void onHexaRadioButtonToggled(bool checked);
        void onAppendCheckBoxToggled(bool checked);
    private:
        Ui::Crc32Widget *ui;
        Crc32 *transform;
};

#endif // CRC32WIDGET_H
