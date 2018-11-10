#include "intermediatesource.h"
#include <threadedprocessor.h>
#include "guihelper.h"
#include <transformmgmt.h>
#include "../quickviewitemconfig.h"

IntermediateSource::IntermediateSource(GuiHelper *guiHelper,
                                       ByteSourceAbstract *originalSource,
                                       TransformAbstract * transform,
                                       quint64 oriStartOffset,
                                       quint64 oriEndOffset,
                                       QObject *parent) :
    BasicSource(parent),
    guiHelper(guiHelper),
    original(originalSource),
    wrapperTransform(transform) // Taking ownership of the instance
{

    if (oriStartOffset < oriEndOffset) { // stupidity check
        startOffset = oriStartOffset;
        endOffset = oriEndOffset;
    } else {
        endOffset = oriStartOffset;
        startOffset = oriEndOffset;
    }
    onOriginalSizeChanged();


    if (wrapperTransform != nullptr) {
        _readonly == wrapperTransform->isTwoWays() || original->isReadonly();
        originalWay = wrapperTransform->way();
        connect(original, &ByteSourceAbstract::readOnlyChanged, this, &IntermediateSource::onOriginalReadOnlyChanged);
        connect(wrapperTransform, &TransformAbstract::confUpdated,this, [=](void) {onOriginalUpdated();});
    }

    capabilities = originalSource->getCapabilities() & static_cast<quint32>(~(CAP_TRANSFORM | CAP_LOADFILE)) ;
    connect(this, &IntermediateSource::sendRequest, guiHelper->getCentralTransProc(), &ThreadedProcessor::processRequest);
    connect(original, &ByteSourceAbstract::updated, this, &IntermediateSource::onOriginalUpdated);
    connect(original, &ByteSourceAbstract::sizeChanged, this, &IntermediateSource::onOriginalSizeChanged);
    onOriginalUpdated(INVALID_SOURCE);
}

IntermediateSource::~IntermediateSource()
{
    delete wrapperTransform;
    qDebug() << "Destroyed" << this;
}

void IntermediateSource::setData(QByteArray data, quintptr source)
{
    if (!checkReadOnly()) {
        if (rawData != data) {
            rawData = data;
            if (!rawData.isEmpty()) {
                onCurrentUpdated();
            }
            emit updated(source);
            emit sizeChanged();
            emit reset();
        }
    }
}

void IntermediateSource::replace(quint64 offset, int length, QByteArray repData, quintptr source)
{
    if (!checkReadOnly()) {
        BasicSource::replace(offset,length, repData,source);
        onCurrentUpdated();
    }
}

void IntermediateSource::insert(quint64 offset, QByteArray repData, quintptr source)
{
    if (!checkReadOnly()) {
        BasicSource::insert(offset,repData,source);
        onCurrentUpdated();
    }
}

void IntermediateSource::remove(quint64 offset, int length, quintptr source)
{
    if (!checkReadOnly()) {
        BasicSource::remove(offset,length,source);
        onCurrentUpdated();
    }
}

void IntermediateSource::clear(quintptr source)
{
    if (!checkReadOnly()) {
        BasicSource::clear(source);
        onCurrentUpdated();
    }
}

void IntermediateSource::fromLocalFile(QString )
{
    emit log(tr("Load file not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void IntermediateSource::optionGuiRequested()
{
    QuickViewItemConfig * guiConfig= new(std::nothrow) QuickViewItemConfig(guiHelper);

    if (guiConfig == nullptr) {
        qFatal("Cannot allocate memory for guiConfig X{");
    }

    guiConfig->setTransform(wrapperTransform);
    guiConfig->adjustSize();

    int ret = guiConfig->exec();
    if (ret == QDialog::Accepted) {
        delete wrapperTransform;
        wrapperTransform = guiConfig->getTransform();
        if (wrapperTransform != nullptr) {
            onCurrentUpdated();
        }
    }
}

void IntermediateSource::onOriginalUpdated(quintptr source)
{
    if (source == reinterpret_cast<quintptr>(this))
        return;

    onOriginalSizeChanged();

    QByteArray oriData = original->extract(startOffset, length);
    if (wrapperTransform != nullptr && !oriData.isEmpty()) {
        TransformAbstract * ta = guiHelper->getTransformFactory()->cloneTransform(wrapperTransform);
        ta->setWay(originalWay);
        TransformRequest *tr = new(std::nothrow) TransformRequest(
                    ta,
                    oriData,
                    reinterpret_cast<quintptr>(this));

        if (tr == nullptr) {
            qFatal("Cannot allocate memory for TransformRequest X{");
        }

        //connect(tr, qOverload<QByteArray,Messages>(&TransformRequest::finishedProcessing), this, &IntermediateSource::inboundProcessingFinished,Qt::QueuedConnection);
        connect(tr, SIGNAL(finishedProcessing(QByteArray,Messages)), this, SLOT(inboundProcessingFinished(QByteArray,Messages)), Qt::QueuedConnection);
        emit sendRequest(tr);
    } else {
        rawData = oriData;
    }
}

void IntermediateSource::onCurrentUpdated()
{
    if (wrapperTransform != nullptr) {
        TransformAbstract * ta = guiHelper->getTransformFactory()->cloneTransform(wrapperTransform);
        if (ta != nullptr) {
            if (originalWay == TransformAbstract::INBOUND)
                ta->setWay(TransformAbstract::OUTBOUND);
            else
                ta->setWay(TransformAbstract::INBOUND);
            TransformRequest *tr = new(std::nothrow) TransformRequest(
                        ta,
                        rawData,
                        reinterpret_cast<quintptr>(this));

            if (tr == nullptr) {
                qFatal("Cannot allocate memory for TransformRequest X{");
            }

            //connect(tr, qOverload<QByteArray,Messages>(&TransformRequest::finishedProcessing), this, &IntermediateSource::outboundProcessingFinished);
            connect(tr, SIGNAL(finishedProcessing(QByteArray,Messages)), this, SLOT(outboundProcessingFinished(QByteArray,Messages)));
            emit sendRequest(tr);
        } else {
            qCritical() << tr("[IntermediateSource::onCurrentUpdated] Cloning failed T_T");
        }
    } else {
        original->replace(startOffset,length,rawData,reinterpret_cast<quintptr>(this));
    }
}

void IntermediateSource::inboundProcessingFinished(QByteArray data, Messages messages)
{
    rawData = data;
    for (int i = 0; i < messages.size() ; i++) {
        emit log(messages.at(i).message,messages.at(i).source, messages.at(i).level);
    }
    emit updated(INVALID_SOURCE); // this will reinitialise all views using this source
}

void IntermediateSource::outboundProcessingFinished(QByteArray data, Messages messages)
{
    original->replace(startOffset,length,data,reinterpret_cast<quintptr>(this));
    for (int i = 0; i < messages.size() ; i++) {
        emit log(messages.at(i).message,messages.at(i).source, messages.at(i).level);
    }
}

void IntermediateSource::onOriginalReadOnlyChanged(bool val)
{
    bool newValue = !wrapperTransform->isTwoWays() || val;
    if (newValue != _readonly) {
        _readonly = newValue;
        emit readOnlyChanged(_readonly);
    }
}

void IntermediateSource::onOriginalSizeChanged()
{
    quint64 temp = qMin(endOffset - startOffset,qMin(original->size(), static_cast<quint64>(INT_MAX)));

    if (temp > INT_MAX) {
        length = INT_MAX;
    } else {
        length = static_cast<int>(temp);
    }
}
