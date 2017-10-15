#ifndef INTERMEDIATESOURCE_H
#define INTERMEDIATESOURCE_H

#include <QObject>
#include "basicsource.h"
#include <commonstrings.h>
#include <transformabstract.h>
#include <climits>

class TransformAbstract;
class TransformRequest;
class GuiHelper;

using namespace Pip3lineConst;
/*
 * the transform cannot be shared , it will be executed in another thread
 */

class IntermediateSource : public BasicSource
{
        Q_OBJECT
    public:
        explicit IntermediateSource(GuiHelper * guiHelper,
                                    ByteSourceAbstract *originalSource,
                                    TransformAbstract * transform = nullptr,
                                    quint64 oriStartOffset = 0,
                                    quint64 oriEndOffset = ULLONG_MAX,
                                    QObject *parent = 0);
        ~IntermediateSource();
        void setData(QByteArray data, quintptr source = INVALID_SOURCE);
        void replace(quint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        void insert(quint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        void remove(quint64 offset, int length, quintptr source = INVALID_SOURCE);
        void clear(quintptr source = INVALID_SOURCE);
        void fromLocalFile(QString fileName);
    public slots:
        void optionGuiRequested();
    signals:
        void sendRequest(TransformRequest *);
    private slots:
        void onOriginalUpdated(quintptr source = 0);
        void onCurrentUpdated();
        void inboundProcessingFinished(QByteArray data, Messages messages);
        void outboundProcessingFinished(QByteArray data, Messages messages);
        void onOriginalReadOnlyChanged(bool val);
        void onOriginalSizeChanged();
    private:
        GuiHelper * guiHelper;
        ByteSourceAbstract *original;
        TransformAbstract *wrapperTransform;
        TransformAbstract::Way originalWay;
        quint64 startOffset;
        quint64 endOffset;
        int length;
};

#endif // INTERMEDIATESOURCE_H
