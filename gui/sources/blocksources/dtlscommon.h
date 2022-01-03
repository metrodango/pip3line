#ifndef DTLSCOMMON_H
#define DTLSCOMMON_H

#include <QSslError>
#include <QString>



namespace DtlsCommon
{
    #pragma pack(push, 1)
    struct DTLSMessage {
            quint8 type;
            quint8 majorVersion;
            quint8 minorVersion;
            quint16 epoch;
            quint64 sequence_number_length;
    };

    struct DTLSAlert {
            quint8 level;
            quint8 description;
    };

    struct STUNMessage {
            quint16 type;
            quint16 length;
            quint32 magic;
            quint32 transactionId1;
            quint32 transactionId2;
            quint32 transactionId3;
    };
    #pragma pack(pop)

    enum TLSRecordTypes {
        CHANGE_CIPHER_SPEC = 20,
        ALERT = 21,
        HANDSHAKE = 22,
        APPLICATION_DATA = 23
    };

    enum TLSAlertLevel {
        TLS_WARNING = 1,
        TLS_FATAL = 2
    };

    enum TLSAlertDescription {
        TLS_close_notify = 0,
        TLS_unexpected_message = 10,
        TLS_bad_record_mac = 20,
        TLS_decryption_failed_RESERVED = 21,
        TLS_record_overflow = 22,
        TLS_decompression_failure_RESERVED = 30,
        TLS_handshake_failure = 40,
        TLS_no_certificate_RESERVED = 41,
        TLS_bad_certificate = 42,
        TLS_unsupported_certificate = 43,
        TLS_certificate_revoked = 44,
        TLS_certificate_expired = 45,
        TLS_certificate_unknown = 46,
        TLS_illegal_parameter = 47,
        TLS_unknown_ca = 48,
        TLS_access_denied = 49,
        TLS_decode_error = 50,
        TLS_decrypt_error = 51,
        TLS_export_restriction_RESERVED = 60,
        TLS_protocol_version = 70,
        TLS_insufficient_security = 71,
        TLS_internal_error = 80,
        TLS_inappropriate_fallback = 86,
        TLS_user_canceled = 90,
        TLS_no_renegotiation_RESERVED = 100,
        TLS_missing_extension = 109,
        TLS_unsupported_extension = 110,
        TLS_certificate_unobtainable_RESERVED = 111,
        TLS_unrecognized_name = 112,
        TLS_bad_certificate_status_response = 113,
        TLS_bad_certificate_hash_value_RESERVED = 114,
        TLS_unknown_psk_identity = 115,
        TLS_certificate_required = 116,
        TLS_no_application_protocol = 120
    };

    extern const quint32 STUN_MAGIC;

    QString getDTLSLevelStr(const quint8 &level);
    QString getDTLSAlertStr(const quint8 &description);
    QString getDTLSRecordTypeStr(const quint8 &type);

    void printSslErrors(QVector<QSslError> errors);
}

#endif // DTLSCOMMON_H
