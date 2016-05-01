/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SINGLEVIEWABSTRACT_H
#define SINGLEVIEWABSTRACT_H

#include <QBitArray>
#include <QWidget>
#include <commonstrings.h>

class ByteSourceAbstract;
class GuiHelper;
class LoggerWidget;
using namespace Pip3lineConst;

class SingleViewAbstract : public QWidget
{
        Q_OBJECT
    public:
        explicit SingleViewAbstract(ByteSourceAbstract *dataModel, GuiHelper *guiHelper, QWidget *parent = 0, bool takeByteSourceOwnership = false);
        virtual ~SingleViewAbstract();
        ByteSourceAbstract *getByteSource() const;
    public slots:
        virtual void search(QByteArray item, QBitArray mask) = 0;
        virtual void searchAgain();
    signals:
        void askForFileLoad();
    protected:
        void internalUpdateData(const QByteArray &rawdata);
        void internalUpdateMessages(Messages messages);
        ByteSourceAbstract *byteSource;
        GuiHelper * guiHelper;
        LoggerWidget *logger;
        QByteArray previousSearch;
        QBitArray previousMask;
        bool hasSourceOwnership;
        
};

#endif // SINGLEVIEWABSTRACT_H
