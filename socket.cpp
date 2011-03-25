#include "socket.h"

Socket::Socket(HWND hWnd, int addressFamily, int connectionType, int protocol)
: hWnd_(hWnd), outputBuffer_(new QBuffer()), inputBuffer_(new QBuffer()) {

    if ((socket_ = WSASocket(addressFamily, connectionType, protocol, NULL, 0,
                             WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
        int err = WSAGetLastError();
        qDebug("Socket::Socket(): Can't create socket. (%d)", err);
        throw "Socket::Socket(); Can't create socket.";
    }

    connect(this, SIGNAL(signalSocketClosed()),
            this, SLOT(deleteLater()));
}

Socket::Socket(SOCKET socket, HWND hWnd)
: socket_(socket), hWnd_(hWnd), outputBuffer_(new QBuffer()), 
  inputBuffer_(new QBuffer()) { }

qint64 Socket::readData(char * data, qint64 maxSize) {
    qint64 bytesRead = 0;

    // Mutex lock here.
    inputBuffer_->open(QBuffer::ReadOnly);
    bytesRead = inputBuffer_->read(data, maxSize);
    inputBuffer_->close();
    // Mutex unlock.

    return bytesRead;
}

qint64 Socket::writeData(const char * data, qint64 maxSize) {
    qint64 bytesWritten = 0;

    // Mutex lock here.
    outputBuffer_->open(QBuffer::WriteOnly);
    bytesWritten = outputBuffer_->write(data, maxSize);
    outputBuffer_->close();
    // Mutex unlock.

    return bytesWritten;
}

bool Socket::listen(PSOCKADDR_IN pSockAddr) {
    int err = 0;

    if ((err = bind(socket_, (PSOCKADDR) pSockAddr, sizeof(SOCKADDR))
        == SOCKET_ERROR)) {
        qDebug("Socket::startServer(): Can't bind to socket. Error: %d",
               WSAGetLastError());
        return false;
    }

    return true;
}

void Socket::close(PMSG pMsg) {
    shutdown(pMsg->wParam, SD_SEND);
    emit signalSocketClosed();
}



bool Socket::slotProcessWSAEvent(PMSG pMsg) {
    if (WSAGETSELECTERROR(pMsg->lParam)) {
        qDebug("Socket::slotProcessWSAEvent(): %d: Socket failed with error %d",
              (int) pMsg->wParam, WSAGETSELECTERROR(pMsg->lParam));
        return false;
    }

    switch (WSAGETSELECTEVENT(pMsg->lParam)) {

        case FD_CLOSE:
            qDebug("Socket::slotProcessWSAEvent: %d: FD_CLOSE.", (int) pMsg->wParam);
            close(pMsg);
            break;
    }

    return true;
}
