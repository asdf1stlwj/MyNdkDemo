#include <jni.h>
#include <string>
#include <malloc.h>
// va_list,vsnprintf
#include <stdarg.h>

//errno
#include <errno.h>

//strerror_t,memset
#include <string.h>

//socket,bind,getsocketname,listen,accept,recv,send,connect
#include <sys/types.h>
#include <sys/socket.h>

//socketaddr_un
#include <sys/un.h>

//htons,sockaddr_in
#include <netinet/in.h>

//inet_ntop
#include <arpa/inet.h>

//close,unlink
#include <unistd.h>

//offsetof
#include <stddef.h>

//最大日志消息长度
#define MAX_LOG_MESSAGE_LENGTH 256

//最大数据缓冲区大小
#define MAX_BUFFER_SIZE 80
#include <android/log.h>
#include "com_asdf_echosocket_EchoServerActivity.h"
#define LOG_TAG "System.out.c"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


/**
 * 返回值 char* 这个代表char数组的首地址
 *  Jstring2CStr 把java中的jstring的类型转化成一个c语言中的char 字符串
 */
char*   Jstring2CStr(JNIEnv *env,   jstring   jstr){
    char*   rtn   =   NULL;
    jclass   clsstring   =   env->FindClass("java/lang/String"); //String
    jstring   strencode   =   (env)->NewStringUTF("GB2312");  // 得到一个java字符串 "GB2312"
    jmethodID   mid   =   (env)->GetMethodID(clsstring,   "getBytes",   "(Ljava/lang/String;)[B"); //[ String.getBytes("gb2312");
    jbyteArray   barr=   (jbyteArray)(env)->CallObjectMethod(jstr,mid,strencode); // String .getByte("GB2312");
    jsize   alen   =   (env)->GetArrayLength(barr); // byte数组的长度
    jbyte*   ba   =   (env)->GetByteArrayElements(barr,JNI_FALSE);
    if(alen   >   0)
    {
        rtn   =   (char*)malloc(alen+1);         //"\0"
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }
    (env)->ReleaseByteArrayElements(barr,ba,0);  //
    return rtn;
}

/**
 * 将给定的消息记录到应用程序
 */
static void LogMessage(JNIEnv* env,jobject obj,const char* format,...){
    //缓存日志方法ID
    static jmethodID methodID=NULL;

    //如果方法ID未缓存
    if (NULL==methodID){
        //从对象获取类
        jclass clazz=env->GetObjectClass(obj);
        //获取方法id
        methodID=env->GetMethodID(clazz,"logMessage","(Ljava/lang/String;)V");
        //释放类引用
        env->DeleteLocalRef(clazz);
    }

    //如果找到方法
    if (NULL!=methodID){
        //格式化日志信息
        char buffer[MAX_LOG_MESSAGE_LENGTH];

        va_list ap;
        va_start(ap,format);
        vsnprintf(buffer,MAX_LOG_MESSAGE_LENGTH,format,ap);
        va_end(ap);

        //将缓冲区转换成Java字符串
        jstring message=env->NewStringUTF(buffer);

        //如何字符串构造成功
        if (NULL!=message){
            //记录信息
            env->CallVoidMethod(obj,methodID,message);
            //释放消息应用
            env->DeleteGlobalRef(message);
        }
    }
}

static void ThrowException(JNIEnv* env,const char* className,const char* message){
    //获取异常类
    jclass clazz=env->FindClass(className);
    if (NULL!=clazz){
        env->ThrowNew(clazz,message);
        //释放原生类引用
        env->DeleteLocalRef(clazz);
    }
}

static void ThrowErrnoException(JNIEnv* env,const char* className,int errnum){
    char buffer[MAX_LOG_MESSAGE_LENGTH];

    //获取错误号信息
    if (-1==strerror_r(errnum,buffer,MAX_LOG_MESSAGE_LENGTH)){
        strerror_r(errno,buffer,MAX_LOG_MESSAGE_LENGTH);
    }
    //抛出异常
    ThrowException(env,className,buffer);
}

/**
 * 构造新的tcp socket
 */
static int NewTcpSocket(JNIEnv* env,jobject obj){
    //构造socket
    LogMessage(env,obj,"Constructing a new TCP socket...");
    int tcpsocket=socket(PF_INET,SOCK_STREAM,0);
    //检查构造socket是否正常
    if (-1==tcpsocket){
        //抛出带错误号的异常
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
    return tcpsocket;
}

/**
 * 将socket绑定到某一端口号
 */
static void BindSocketToPort(JNIEnv* env,jobject obj,int sd, unsigned short port){
    struct sockaddr_in address;
    //绑定socket地址
    //memset的作用是在一段内存块中填充某个给定的值,它是对较大的结构体或数组进行清零操作的一种最快方法
    memset(&address,0, sizeof(address));
    address.sin_family=PF_INET;
    //绑定到所有地址
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    //将端口转换为网络字节顺序
    address.sin_port=htons(port);
    //绑定socket
    LogMessage(env,obj,"Binding to port %hu.",port);
    if (-1==bind(sd, (const sockaddr *) &address, sizeof(address))){
        //抛出带错误号的异常
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
}

/**
 * 获取当前绑定的端口号socket
 */
static unsigned short GetSocketPort(JNIEnv* env,jobject obj,int sd){
    unsigned short port=0;
    struct sockaddr_in address;
    socklen_t addressLength= sizeof(address);
    //获取socket地址
    if (-1==getsockname(sd, (sockaddr *) &address, &addressLength)){
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        port=ntohs(address.sin_port);
        LogMessage(env,obj,"Bind to random port %hu.",port);
    }
    return port;
}

/**
 * 监听指定的待处理连接的backlog的socket,当backlog已满时拒绝新的连接
 */
static void ListenOnSocket(JNIEnv* env,jobject obj,int sd,int backlog){
    //监听给定backlog的socket
    LogMessage(env,obj,"Listening on socket with a backlog of %d pending connection",backlog);
    //通过listen函数监听输入连接只是简单地输入连接放进一个队列并等待应用程序显式地接受他们
    if (-1==listen(sd,backlog)){
        ThrowErrnoException(env,"java/io/IOException",errno);
    }

}

/**
 * 记录给定地址的ip地址和端口号
 */
static void LogAddress(JNIEnv* env,jobject obj,const char* message,const struct sockaddr_in* address){
    char ip[INET_ADDRSTRLEN];
    //将ip地址转换为字符串
    if (NULL==inet_ntop(PF_INET,&(address->sin_addr),ip,INET_ADDRSTRLEN)){
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        //将端口转换成主机字节顺序
        unsigned short port=ntohs(address->sin_port);
        //记录地址
        LogMessage(env,obj,"%s %s:%hu.",message,ip,port);
    }
}

/**
 * 在给定的socket上阻塞和等待进来的客户连接
 */
static int AcceptOnSocket(JNIEnv* env,jobject obj,int sd){
    struct sockaddr_in address;
    socklen_t addressLength= sizeof(address);
    //阻塞和等待进来的客户连接
    //并且接受它
    LogMessage(env,obj,"Waiting for a client connection...");
    int clientSocket=accept(sd, (sockaddr *) &address, &addressLength);
    //如果客户socket无效
    if (-1==clientSocket){
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        //记录地址
        LogAddress(env,obj,"Client connection from ",&address);
    }
    return clientSocket;
}

/**
 * 阻塞并接收来自socket的数据放进缓冲区
 */
static ssize_t ReceiveFromSocket(JNIEnv* env,jobject obj,int sd, char* buffer,size_t bufferSize){
    //阻塞并接收来自socket的数据放进缓冲区
    LogMessage(env,obj,"Receiving from the socket...");
    ssize_t recvSize=recv(sd,buffer,bufferSize-1,0);
    //如果接收失败
    if (-1==recvSize){
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        //以NULL结尾缓冲区形成一个字符串
        buffer[recvSize]=NULL;
        //如果接收成功
        if (recvSize>0){
            LogMessage(env,obj,"Receive %d bytes: %s",recvSize,buffer);
        } else{
            LogMessage(env,obj,"Client disconnected.");
        }
    }
    return recvSize;

}

static ssize_t SendToSocket(JNIEnv* env,jobject obj,int sd,const char* buffer,size_t bufferSize){
    //将数据缓冲区发送到socket
    LogMessage(env,obj,"Sending to the socket...");
    ssize_t sentSize=send(sd,buffer,bufferSize,0);
    //如果发送失败
    if (-1==sentSize){
        //抛出带错误号的异常
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        if (sentSize>0){
            LogMessage(env,obj,"Sent %d bytes: %s",sentSize,buffer);
        } else{
            LogMessage(env,obj,"Client disconnected.");
        }
    }
    return sentSize;
}

/*
 * Class:     com_asdf_echosocket_EchoServerActivity
 * Method:    nativeStartTcpServer
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_asdf_echosocket_EchoServerActivity_nativeStartTcpServer
        (JNIEnv* env, jobject obj, jint port){
    //构造新的tcp socket
    int serverSocket=NewTcpSocket(env,obj);
    if (NULL!=env->ExceptionOccurred()){
        //将socket绑定到某端口号
        BindSocketToPort(env,obj,serverSocket,port);
        if (NULL!=env->ExceptionOccurred())
            goto exit;
        //如果请求了随机端口号
        if (0==port){
            //获取当前绑定的端口号socket
            GetSocketPort(env,obj,serverSocket);
            if (NULL!=env->ExceptionOccurred())
                goto exit;
        }
        //监听4个等待连接的backlog的socket
        ListenOnSocket(env,obj,serverSocket,4);
        if (NULL!=env->ExceptionOccurred())
            goto exit;
        //接收socket的一个客户连接
        int clientSocket=AcceptOnSocket(env,obj,serverSocket);
        if (NULL!=env->ExceptionOccurred())
            goto exit;
        char buffer[MAX_BUFFER_SIZE];
        ssize_t recvSize;
        ssize_t sentSize;
        //接收并发送回数据
        while (1){
            //从socket中接收
            recvSize=ReceiveFromSocket(env,obj,clientSocket,buffer,MAX_BUFFER_SIZE);
            if ((0==recvSize) || (NULL!=env->ExceptionOccurred()))
                break;
            //发送给socket
            sentSize=SendToSocket(env,obj,clientSocket,buffer,(size_t)recvSize);
            if ((0==sentSize)||(NULL!=env->ExceptionOccurred()))
                break;
        }
        //关闭客户端socket
        close(clientSocket);

    }
    exit:
        if (serverSocket>0){
            close(serverSocket);
        }
}