/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "qdatastream.h"

#include "Empath.h"
#include "EmpathInterface.h"
#include "EmpathIndexRecord.h"
#include "RMM_Message.h"

enum EmpathFn {
    EmpathFnInboxURL,
    EmpathFnOutboxURL,
    EmpathFnSentURL,
    EmpathFnDraftsURL,
    EmpathFnTrashURL,
    EmpathFnMessage,
    EmpathFnQueue,
    EmpathFnSend,
    EmpathFnSendQueued,
    EmpathFnCheckMail,
    EmpathFnCompose,
    EmpathFnReply,
    EmpathFnReplyAll,
    EmpathFnForward,
    EmpathFnBounce,
    EmpathFnCreateFolder,
    EmpathFnRemoveFolder,
    EmpathFnCopy,
    EmpathFnMove,
    EmpathFnRetrieve,
    EmpathFnWrite,
    EmpathFnRemove,
    EmpathFnRemoveMany,
    EmpathFnMark,
    EmpathFnMarkMany
};

EmpathInterface::EmpathInterface()
    :   DCOPObject()
{
    fnDict_.setAutoDelete(true);

    fnDict_.insert(
        QString::fromUtf8("QString inboxURL()"),
        new int(EmpathFnInboxURL)
    );

    fnDict_.insert(
        QString::fromUtf8("QString outboxURL()"),
        new int(EmpathFnOutboxURL)
    );

    fnDict_.insert(
        QString::fromUtf8("QString sentURL()"),
        new int(EmpathFnSentURL)
    );

    fnDict_.insert(
        QString::fromUtf8("QString draftsURL()"),
        new int(EmpathFnDraftsURL)
    );

    fnDict_.insert(
        QString::fromUtf8("QString trashURL()"),
        new int(EmpathFnTrashURL)
    );

    fnDict_.insert(
        QString::fromUtf8("QByteArray message(QString)"),
        new int(EmpathFnMessage)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int queue(QByteArray)"),
        new int(EmpathFnQueue)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int send(QByteArray)"),
        new int(EmpathFnSend)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int sendQueued()"),
        new int(EmpathFnSendQueued)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int checkMail()"),
        new int(EmpathFnCheckMail)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int compose(QString)"),
        new int(EmpathFnCompose)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int reply(QString)"),
        new int(EmpathFnReply)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int replyAll(QString)"),
        new int(EmpathFnReplyAll)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int forward(QString)"),
        new int(EmpathFnForward)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int bounce(QString)"),
        new int(EmpathFnBounce)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int createFolder(QString)"),
        new int(EmpathFnCreateFolder)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int removeFolder(QString)"),
        new int(EmpathFnRemoveFolder)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int copy(QString, QString)"),
        new int(EmpathFnCopy)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int move(QString, QString)"),
        new int(EmpathFnMove)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int retrieve(QString)"),
        new int(EmpathFnRetrieve)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int write(QByteArray, QString)"),
        new int(EmpathFnWrite)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int remove(QString)"),
        new int(EmpathFnRemove)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int remove(QString, QStringList)"),
        new int(EmpathFnRemoveMany)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int mark(QString, int)"),
        new int(EmpathFnMark)
    );

    fnDict_.insert(
        QString::fromUtf8("unsigned int mark(QString, QStringList, int)"),
        new int(EmpathFnMarkMany)
    );
}

    bool
EmpathInterface::process(
    const QCString & cmd, const QByteArray & argData,
    QCString & replyType, QByteArray & replyData)
{
    bool retval = true;

    QDataStream inStream(argData, IO_ReadOnly);
    QDataStream outStream(replyData, IO_WriteOnly);

    int * cmdNamePtr = fnDict_[cmd];

    if (cmdNamePtr == 0)
        return false;

    switch (*(cmdNamePtr)) {

        case EmpathFnInboxURL:
            replyType = "QString";
            outStream << inboxURL();
            break;

        case EmpathFnOutboxURL:
            replyType = "QString";
            outStream << outboxURL();
            break;

        case EmpathFnSentURL:
            replyType = "QString";
            outStream << sentURL();
            break;

        case EmpathFnDraftsURL:
            replyType = "QString";
            outStream << draftsURL();
            break;

        case EmpathFnTrashURL:
            replyType = "QString";
            outStream << draftsURL();
            break;

        case EmpathFnMessage:
            {
                replyType = "QByteArray";
                QString arg;
                inStream >> arg;
                outStream << message(arg);
            }
            break;

        case EmpathFnSend:
            {
                replyType = "void";
                QByteArray arg;
                QDataStream intoArg(arg, IO_WriteOnly);
                inStream >> arg;
                RMM::RMessage msg(QCString(arg.data()));
                empath->send(msg);
            }
            break;

        case EmpathFnQueue:
            {
                replyType = "void";
                QByteArray arg;
                inStream >> arg;
                RMM::RMessage msg(QCString(arg.data()));
                empath->queue(msg);
            }
            break;

        case EmpathFnSendQueued:
            replyType = "void";
            empath->sendQueued();
            break;

        case EmpathFnCheckMail:
            replyType = "void";
            empath->s_checkMail();
            break;

        case EmpathFnCompose:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                empath->s_composeTo(arg);
            }
            break;

        case EmpathFnReply:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                empath->s_reply(EmpathURL(arg));
            }
            break;

        case EmpathFnReplyAll:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                empath->s_replyAll(EmpathURL(arg));
            }
            break;

        case EmpathFnForward:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                empath->s_forward(EmpathURL(arg));
            }
            break;

        case EmpathFnBounce:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                empath->s_bounce(EmpathURL(arg));
            }
            break;

        case EmpathFnCreateFolder:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                outStream << empath->createFolder(EmpathURL(arg));
            }
            break;

        case EmpathFnRemoveFolder:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                outStream << empath->removeFolder(EmpathURL(arg));
            }
            break;

        case EmpathFnCopy:
            {
                replyType = "void";
                QString arg1, arg2;
                inStream >> arg1;
                inStream >> arg2;
                outStream << empath->copy(EmpathURL(arg1), EmpathURL(arg2));
            }
            break;

        case EmpathFnMove:
            {
                replyType = "void";
                QString arg1, arg2;
                inStream >> arg1;
                inStream >> arg2;
                outStream << empath->move(EmpathURL(arg1), EmpathURL(arg2));
            }
            break;

        case EmpathFnRetrieve:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                outStream << empath->retrieve(EmpathURL(arg));
            }
            break;

        case EmpathFnWrite:
            {
                replyType = "void";
                QString arg1;
                QByteArray arg2;
                inStream >> arg1;
                inStream >> arg2;
                RMM::RMessage msg(QCString(arg2.data()));
                outStream << empath->write(msg, EmpathURL(arg1));
            }
            break;

        case EmpathFnRemove:
            {
                replyType = "void";
                QString arg;
                inStream >> arg;
                outStream << empath->remove(EmpathURL(arg));
            }
            break;

        case EmpathFnRemoveMany:
            {
                replyType = "void";
                QString arg1;
                QStringList arg2;
                inStream >> arg1;
                inStream >> arg2;
                outStream << empath->remove(EmpathURL(arg1), arg2);
            }
            break;

        case EmpathFnMark:
            {
                replyType = "void";
                QString arg1;
                int arg2;
                inStream >> arg1;
                inStream >> arg2;
                outStream << empath->mark(
                    EmpathURL(arg1),
                    EmpathIndexRecord::Status(arg2)
                    );
            }
            break;

        case EmpathFnMarkMany:
            {
                replyType = "void";
                QString arg1;
                QStringList arg2;
                int arg3;
                inStream >> arg1;
                inStream >> arg2;
                inStream >> arg3;
                outStream << empath->mark(
                    EmpathURL(arg1),
                    arg2,
                    EmpathIndexRecord::Status(arg3));
            }
            break;

        default:
            retval = false;
            break;
    }

    return retval;
}


QCString EmpathInterface::functions()
{
    return DCOPObject::functions() 
 + 
	"inboxURL();"
	"outboxURL();"
	"sentURL();"
	"draftsURL();"
	"trashURL();"
	"message(QString);"
	"queue(QByteArray);"
	"send(QByteArray);"
	"sendQueued();"
	"checkMail();"
	"compose(QString);"
	"reply(QString);"
	"replyAll(QString);"
	"forward(QString);"
	"bounce(QString);"
	"createFolder(QString);"
	"removeFolder(QString);"
	"copy(QString,QString);"
	"move(QString,QString);"
	"retrieve(QString);"
	"write(QByteArray,QString);"
	"remove(QString);"
	"remove(QString,QStringList);"
	"mark(QString,int);"
	"mark(QString,QStringList,int);"
	;
}

