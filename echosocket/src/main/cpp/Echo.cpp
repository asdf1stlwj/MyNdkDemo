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

static void BindSocketToPort(JNIEnv* env,jobject obj,int sd, unsigned short port){
    struct sockaddr_in address;
    //绑定socket地址
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