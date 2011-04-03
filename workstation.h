#include <QObject>
#include <QThread>
#include <QMap>
#include <QByteArray>

class MainWindow;
class Socket;
class TCPSocket;
class UDPSocket;
class FileData;

class Workstation : public QObject {
  Q_OBJECT

private:
    /**  */
    TCPSocket *tcpSocket_;

    /**  */
    UDPSocket *udpSocket_;

    QThread* socketThread_;

    /* Pointer to the main window. This is used to access the window handle for
    when we create new sockets.*/
    MainWindow *mainWindowPointer_;

    // Collection for file transfers
    QMap <Socket*, FileData*> currentTransfers;

    // Functions
    void sendFile(Socket*, QByteArray*);
    void acceptVoiceChat();

    bool processReceivingFile(Socket*, QByteArray*);
    bool processReceivingFileList(Socket*, QByteArray*);
    bool processReceivingFileRequest(Socket*, QByteArray*);


public slots:
    // Triggered by GUI buttons
    void connectToServer();
    void requestFile(QString, short, QString);
    void requestFileList(QString, short);
    // Triggered by sockets
    void processConnection(Socket*);
    void decodeControlMessage(Socket*);
    void receiveUDP();
    void receiveFileController(Socket*);
    void receiveFileListController(Socket*);
    void requestFileListController(Socket*);
    void sendFileController(Socket*);
    void startVoiceStream(short port, QString hostAddr);
    void stopVoiceStream();

signals:
    void signalFileListUpdate(QStringList*);

public:
    Workstation(MainWindow *mainWindow);
    virtual ~Workstation();

};
