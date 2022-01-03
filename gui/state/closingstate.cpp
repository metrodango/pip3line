/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "closingstate.h"
#include <QSettings>
#include <QTime>
#include <QXmlStreamWriter>
#include <QDebug>
#include "shared/guiconst.h"

ClosingState::ClosingState(QString expected)
{
    //name = metaObject()->className();
    closingTokenExpected = expected;
 //   qDebug() << "Create" << this;
}

ClosingState::~ClosingState()
{

}

void ClosingState::run()
{
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeEndElement();
    } else if (closingTokenExpected.isEmpty()) { // just read the next end element
        QXmlStreamReader::TokenType token = reader->readNext();
        while ( token != QXmlStreamReader::EndElement && token != QXmlStreamReader::Invalid) {
            token = reader->readNext();
        }
    } else {
        if (reader->name() != closingTokenExpected || reader->tokenType() != QXmlStreamReader::EndElement) {
            //only read the next end element if the current one is not the expected one and an EndElement
            QXmlStreamReader::TokenType token = reader->readNext();
            while ( token != QXmlStreamReader::EndElement && token != QXmlStreamReader::Invalid) {
                token = reader->readNext();
            }
        }
    }
}
