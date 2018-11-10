#include "tds7_enc.h"

const QString TDS7_ENC::id = "TDS7";

TDS7_ENC::TDS7_ENC()
{

}

TDS7_ENC::~TDS7_ENC()
{

}

QString TDS7_ENC::name() const
{
    return id;
}

QString TDS7_ENC::description() const
{
    return tr("TDS 7 password encoding");
}

void TDS7_ENC::transform(const QByteArray &input, QByteArray &output)
{
    char xorEnc = '\xA5';
    int size = 0;
    if (wayValue == INBOUND) {
        if (input.size() > 128) {
            emit warning("Input is bigger than 128 (maximum defined by standard) truncating", id);
            size = 128;
        } else {
            size = input.size();
        }
        output = QByteArray(size * 2, ('\x00' ^ xorEnc));

        for (int i = 0; i < size; i++) {
            output[i * 2] =  uchar(uchar(input[i]) >> uchar(4) | uchar(input[i]) << uchar(4)) ^ xorEnc;
        }

    } else {
        size = input.size();
        if (input.size() > 256)  {// standard only allow 128 UNICODE character , but that's fake it really only is 128 * 2
            emit warning("Input is bigger than 128 * 2 (maximum defined by standard) truncating", id);
            output = input.mid(0,256);
        } else {
            output.resize(size);
        }

        for (int i = 0; i < size; i++) {
            output[i] = input[i] ^ xorEnc;
        }

        if (output.size() % 2 != 0) {
            output.append('\x00');
            emit warning("Input is not of even size, adding one zero byte at the end", id);
        }

        for (int i = 0; i < size; i++) {
            if (i % 2 == 0) {
                output[i/2] = char(uchar(uchar(output[i]) >> uchar(4)) | uchar(uchar(output[i]) << uchar(4)));
            }
        }

        output.resize(output.size() / 2);
    }
}

bool TDS7_ENC::isTwoWays()
{
    return true;
}

QString TDS7_ENC::help() const
{
    return tr("TDS 7 password encoding used by MS SQL databases connections. Useful when the connection is unencrypted or one can intercept the TLS connection.");
}
