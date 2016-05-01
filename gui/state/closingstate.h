/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CLOSINGSTATE_H
#define CLOSINGSTATE_H

#include "basestateabstract.h"

/**
 * @brief The ClosingState class
 *
 * Utility class used to perform close actions such as closing group
 * or array open on the state file, or clearing the state file object
 *
 */

class ClosingState : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit ClosingState(QString expected = QString());
        virtual ~ClosingState();
        virtual void run();
    private:
        QString closingTokenExpected;
};

#endif // CLOSINGSTATE_H
