#include <QtGlobal>
#if QT_FEATURE_dtls != -1

#include "dtlscommon.h"
#include <QDebug>

namespace DtlsCommon
{
    const quint32 STUN_MAGIC = 0x42A41221;

    QString getDTLSLevelStr(const quint8 &level)
    {
        QString alertLevel;
        switch (level) {
            case TLS_WARNING:
                alertLevel = "Warning";
                break;
            case TLS_FATAL:
                alertLevel = "Fatal";
                break;
            default:
                alertLevel = QString("Unknown (%1)").arg(level);
        }
        return alertLevel;
    }

    QString getDTLSAlertStr(const quint8 &description)
    {
        QString alertDescription;
        switch (description) {
            case TLS_close_notify:
                alertDescription = "close_notify";
                break;
            case TLS_unexpected_message:
                alertDescription = "unexpected_message";
                break;
            case TLS_bad_record_mac:
                alertDescription = "bad_record_mac";
                break;
            case TLS_decryption_failed_RESERVED:
                alertDescription = "decryption_failed_RESERVED";
                break;
            case TLS_record_overflow:
                alertDescription = "record_overflow";
                break;
            case TLS_decompression_failure_RESERVED:
                alertDescription = "decompression_failure_RESERVED";
                break;
            case TLS_handshake_failure:
                alertDescription = "handshake_failure";
                break;
            case TLS_no_certificate_RESERVED:
                alertDescription = "no_certificate_RESERVED";
                break;
            case TLS_bad_certificate:
                alertDescription = "bad_certificate";
                break;
            case TLS_unsupported_certificate:
                alertDescription = "unsupported_certificate";
                break;
            case TLS_certificate_revoked:
                alertDescription = "certificate_revoked";
                break;
            case TLS_certificate_expired:
                alertDescription = "certificate_expired";
                break;
            case TLS_certificate_unknown:
                alertDescription = "certificate_unknown";
                break;
            case TLS_illegal_parameter:
                alertDescription = "illegal_parameter";
                break;
            case TLS_unknown_ca:
                alertDescription = "unknown_ca";
                break;
            case TLS_access_denied:
                alertDescription = "access_denied";
                break;
            case TLS_decode_error:
                alertDescription = "decode_error";
                break;
            case TLS_decrypt_error:
                alertDescription = "decrypt_error";
                break;
            case TLS_export_restriction_RESERVED:
                alertDescription = "export_restriction_RESERVED";
                break;
            case TLS_protocol_version:
                alertDescription = "protocol_version";
                break;
            case TLS_insufficient_security:
                alertDescription = "insufficient_security";
                break;
            case TLS_internal_error:
                alertDescription = "internal_error";
                break;
            case TLS_inappropriate_fallback:
                alertDescription = "inappropriate_fallback";
                break;
            case TLS_user_canceled:
                alertDescription = "user_canceled";
                break;
            case TLS_no_renegotiation_RESERVED:
                alertDescription = "no_renegotiation_RESERVED";
                break;
            case TLS_missing_extension:
                alertDescription = "missing_extension";
                break;
            case TLS_unsupported_extension:
                alertDescription = "unsupported_extension";
                break;
            case TLS_certificate_unobtainable_RESERVED:
                alertDescription = "certificate_unobtainable_RESERVED";
                break;
            case TLS_unrecognized_name:
                alertDescription = "unrecognized_name";
                break;
            case TLS_bad_certificate_status_response:
                alertDescription = "bad_certificate_status_response";
                break;
            case TLS_bad_certificate_hash_value_RESERVED:
                alertDescription = "bad_certificate_hash_value_RESERVED";
                break;
            case TLS_unknown_psk_identity:
                alertDescription = "unknown_psk_identity";
                break;
            case TLS_certificate_required:
                alertDescription = "certificate_required";
                break;
            case TLS_no_application_protocol:
                alertDescription = "no_application_protocol";
                break;
            default:
                alertDescription = QString("Unknown (%1)").arg(description);
        }
        return alertDescription;
    }

    QString getDTLSRecordTypeStr(const quint8 &type)
    {
        QString typestr;
        switch (type) {
            case CHANGE_CIPHER_SPEC:
                typestr = "CHANGE_CIPHER_SPEC";
                break;
            case ALERT:
                typestr = "ALERT";
                break;
            case HANDSHAKE:
                typestr = "HANDSHAKE";
                break;
            case APPLICATION_DATA:
                typestr = "APPLICATION_DATA";
                break;
            default:
                typestr = QString("Unknown (%1)").arg(type);
        }

        return typestr;
    }

    void printSslErrors(QVector<QSslError> errors)
    {
        for (int i = 0; i < errors.size(); i++) {
            QSslError err = errors.at(i);
            qWarning() << qPrintable(QString("TLS error: %1 (%2)").arg(err.errorString()).arg(err.error()));
        }
    }
}
#endif
