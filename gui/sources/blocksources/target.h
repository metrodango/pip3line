#ifndef TARGET_H
#define TARGET_H

#include <QDateTime>
#include <QString>

template<class T>
class Target
{
    public:
        static const int INVALID_TARGET = -1;
        explicit Target() : connectionID(-1) { }
        Target(const Target<T>& other) {
            this->connectionID = other.connectionID;
            this->description = other.description;
            this->source = other.source;
            this->timestamp = other.timestamp;
        }

        Target<T>& operator = (const Target<T>& other) {
            this->connectionID = other.connectionID;
            this->description = other.description;
            this->source = other.source;
            this->timestamp = other.timestamp;
            return *this;
        }

        Target<T> &operator== (const Target<T>& other) {
            return (this->connectionID = other.connectionID && this->source == other.source && this->timestamp == this->timestamp);
        }

        QString getDescription() const { return description; }
        void setDescription(const QString &value) { description = value; }
        QDateTime getTimestamp() const { return timestamp; }
        void setTimestamp(const QDateTime &value){ timestamp = value; }
        int getConnectionID() const { return connectionID; }
        void setConnectionID(int value){ connectionID = value; }
        T getSource() const{ return source; }
        void setSource(T value) {source = value;}
    private:
        T source{};
        int connectionID;
        QString description;
        QDateTime timestamp{QDateTime::currentDateTime()};
};

#endif // TARGET_H
