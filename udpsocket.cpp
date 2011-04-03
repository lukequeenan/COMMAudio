#include "udpsocket.h"
#include "buffer.h"

UDPSocket::UDPSocket(HWND hWnd)
: Socket(hWnd, AF_INET, SOCK_DGRAM, IPPROTO_UDP) { }

bool UDPSocket::open(OpenMode mode) {
    int err = 0;
    int flags = FD_CLOSE;

    switch (mode) {
        case QIODevice::ReadOnly:
            flags |= FD_READ;
            break;

        case QIODevice::WriteOnly:
            flags |= FD_WRITE;
            break;

        case QIODevice::ReadWrite:
            flags |= FD_READ | FD_WRITE;
            break;

        case QIODevice::NotOpen:
            flags = 0;
            break;

        case QIODevice::Append:
        case QIODevice::Truncate:
        case QIODevice::Text:
        case QIODevice::Unbuffered:
        default:
            return false;
            break;
    }

    if ((err = WSAAsyncSelect(socket_, hWnd_, WM_WSAASYNC_UDP, flags))
                              == SOCKET_ERROR) {
        qDebug("UDPSocket::open(): Error setting up async select.");
        return false;
    }

    QIODevice::connect(this, SIGNAL(readyWrite(qint64)),
                       this, SLOT(slotWriteData(qint64)));

    return QIODevice::open(mode);
}

void UDPSocket::send(PMSG pMsg) {
    int err = 0;
    int result = 0;
    DWORD numSent = 0;
    int num = 0;
    size_t bytesToRead = PACKETSIZE;

    WSABUF winsockBuff;

    if (nextTxBuff_ == NULL && loadBuffer(bytesToRead) == 0) {
        qDebug("UDPSocket::send(); Nothing to send.");
        return;
    }

    winsockBuff.buf = nextTxBuff_->data();
    winsockBuff.len = nextTxBuff_->size();

    while (TRUE) {
        result = WSASendTo(pMsg->wParam, &winsockBuff, 1, &numSent, 0,
                           (PSOCKADDR) &serverSockAddrIn_, sizeof(serverSockAddrIn_), 
                           NULL, NULL);

        if ((err = WSAGetLastError()) > 0 && err != ERROR_IO_PENDING) {
            qDebug("UDPSocket::send(); Error: %d", err);
            return;
        }
        if (result == WSAEWOULDBLOCK) {
            qDebug("UDPSocket::send(); Socket buffer full: WSAEWOULDBLOCK");
            return;
        }

        delete nextTxBuff_;
        nextTxBuff_ = NULL;
        if ((num = loadBuffer(bytesToRead)) <= 0) {
            qDebug("TCPSocket::send(); Finishing...");
            break;
        }
        winsockBuff.len = num;
    }
}

void UDPSocket::receive(PMSG pMsg) {
    int err = 0;
    DWORD flags = 0;
    DWORD numReceived = 0;
    int bytesWritten = 0;
    WSABUF winsockBuff;

    winsockBuff.len = MAXUDPDGRAMSIZE;
    winsockBuff.buf = (char*) calloc(winsockBuff.len, sizeof(char));

    if (WSARecvFrom(pMsg->wParam, &(winsockBuff), 1, &numReceived, &flags,
                NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        if ((err = WSAGetLastError()) != WSA_IO_PENDING) {
            qDebug("UDPSocket::receive(): WSARecv() failed with error %d",
                   err);
            return;
        }
    }
    
    if (numReceived == 0) {
        return;
    }

    QByteArray writeData(winsockBuff.buf, numReceived);

    // CRITICAL SECTION: Lock mutex here.
    QMutexLocker locker(receiveLock_);
    inputBuffer_->write(writeData);
    locker.unlock();
    // END CRITICAL SECTION: Unlock mutex.

    delete[] winsockBuff.buf;

    emit readyRead();
    emit signalDataReceived(this);
}

void UDPSocket::slotProcessWSAEvent(int socket, int lParam) {
    MSG msg;
    msg.wParam = socket;
    msg.lParam = lParam;
    PMSG pMsg = &msg;

    // Filtering out messages for other sockets.
    if (pMsg->wParam != socket_) {
        return;
    }

    if (WSAGETSELECTERROR(pMsg->lParam)) {
        qDebug("UDPSocket::slotProcessWSAEvent(): %d: Socket failed. Error: %d",
              (int) pMsg->wParam, WSAGETSELECTERROR(pMsg->lParam));
        return;
    }

    switch (WSAGETSELECTEVENT(pMsg->lParam)) {
        case FD_READ:
            qDebug("UDPSocket::slotProcessWSAEvent: %d: FD_READ.",
                   (int) pMsg->wParam);
            receive(pMsg);
            break;

        case FD_WRITE:
            qDebug("UDPSocket::slotProcessWSAEvent: %d: FD_WRITE.",
                   (int) pMsg->wParam);
            send(pMsg);
            break;

        default:
            Socket::slotProcessWSAEvent(pMsg);
            break;
    }

    return;
}
