#include <jni.h>
#include <string>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <android/log.h>
#include "com_asdf_ndkthread_MainActivity.h"
#define LOG_TAG "System.out.c"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
static jmethodID methodId=NULL;
static JavaVM* gvm=NULL;

//对象的全局引用
static jobject gobj=NULL;
//互斥实例
static pthread_mutex_t mutex;
struct NativeWorkerArgs{
    jint id;
    jint iteration;
};

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

jint JNI_OnLoad(JavaVM* vm, void* reserved){
    //缓存java虚拟机指针
    gvm=vm;
    return JNI_VERSION_1_4;

}

/*
 * Class:     com_asdf_ndkthread_MainActivity
 * Method:    initNative
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_asdf_ndkthread_MainActivity_initNative
        (JNIEnv* env, jobject obj){
     if (NULL==gobj){
        gobj=env->NewGlobalRef(obj);
         if (NULL==gobj){
             goto exit;
         }
     }
     if (methodId==NULL){
         jclass  clazz=env->GetObjectClass(obj);
         methodId=env->GetMethodID(clazz,"onNativeMessage","(Ljava/lang/String;)V");
         if (methodId==NULL){
             jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
             env->ThrowNew(exceptionClazz,"unable find the class");
         }
     }

     //初始化互斥
     if (0!=pthread_mutex_init(&mutex,NULL)){
         jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
         env->ThrowNew(exceptionClazz,"unable init mutex");
         goto exit;
     }
    exit:
    return;
}

/*
 * Class:     com_asdf_ndkthread_MainActivity
 * Method:    freeNative
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_asdf_ndkthread_MainActivity_freeNative
        (JNIEnv* env, jobject obj){
       if (gobj!=NULL){
           env->DeleteGlobalRef(gobj);
           gobj=NULL;
       }

    //销毁互斥锁
    if (0!=pthread_mutex_destroy(&mutex)){
        jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClazz,"unable destory mutex");
    }
}

/*
 * Class:     com_asdf_ndkthread_MainActivity
 * Method:    nativeWorker
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_asdf_ndkthread_MainActivity_nativeWorker
        (JNIEnv* env, jobject obj, jint id, jint iterations){
    //上锁
    if (0!=pthread_mutex_lock(&mutex)){
        jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClazz,"unable lock mutex");
        goto exit;
    }
    for (int i = 0; i < iterations; ++i) {
        char message[26];
        sprintf(message," Worker %d: Iteration %d",id,i);
        jstring messageString=env->NewStringUTF(message);
        env->CallVoidMethod(obj,methodId,messageString);
        if (NULL!=env->ExceptionOccurred())
            break;
        sleep(1);
    }
    //解锁
    if (0!=pthread_mutex_unlock(&mutex)){
        jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClazz,"unable unlock mutex");
        goto exit;
    }
    exit:
    return;
}

static void* nativeWorkerThread(void* args){
    JNIEnv* env=NULL;
    //将当前线程附着到虚拟机上,并且获得JNIEnv接口指针
    if(0==gvm->AttachCurrentThread(&env,NULL)){
        //获取原生worker线程参数
        NativeWorkerArgs* nativeWorkerArgs= (NativeWorkerArgs *) args;

        //在线程上下文中运行原生worker
        Java_com_asdf_ndkthread_MainActivity_nativeWorker(env,gobj,
                                                          nativeWorkerArgs->id,
                                                          nativeWorkerArgs->iteration);
        //释放原生worker线程参数
        delete  nativeWorkerArgs;

        //从Java虚拟机中分离当前线程
        gvm->DetachCurrentThread();
    }
    return (void *) 1;
}

JNIEXPORT void JNICALL Java_com_asdf_ndkthread_MainActivity_posixThreads
        (JNIEnv* env, jobject obj, jint threads, jint iterations){
    //线程句柄
    pthread_t* handles=new pthread_t[threads];
    //为每一个worker创建posix线程
    for (jint i=0;i<threads;i++){
        //原生worket线程参数
        NativeWorkerArgs* nativeWorkerArgs=new NativeWorkerArgs();
        nativeWorkerArgs->id=i;
        nativeWorkerArgs->iteration=iterations;
        //线程句柄
        pthread_t thread;
        //创建一个新线程
        int result=pthread_create(&handles[i],NULL,nativeWorkerThread,(void*)nativeWorkerArgs);
        if (0!=result){
            //获取异常类
            jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
            //抛出异常
            env->ThrowNew(exceptionClazz,"Unable to create thread");
            goto exit;
        }
    }
//    //等待线程结束
//    for (jint i=0;i<threads;i++){
//        void* result=NULL;
//        if (0!=pthread_join(handles[i],&result)){
//            jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
//            env->ThrowNew(exceptionClazz,"Unable to join thread");
//        } else{
//            char message[26];
//            sprintf(message," Worker %d: return %d",i,result);
//            jstring messageString=env->NewStringUTF(message);
//            env->CallVoidMethod(obj,methodId,messageString);
//            if (NULL!=env->ExceptionOccurred())
//                goto exit;
//        }
//
//
//    }
    exit:
    return;
}

//JNIEXPORT void JNICALL Java_com_asdf_ndkthread_MainActivity_posixThreads
//        (JNIEnv* env, jobject obj, jint threads, jint iterations){
//      //为每一个worker创建posix线程
//      for (jint i=0;i<threads;i++){
//          //原生worket线程参数
//          NativeWorkerArgs* nativeWorkerArgs=new NativeWorkerArgs();
//          nativeWorkerArgs->id=i;
//          nativeWorkerArgs->iteration=iterations;
//          //线程句柄
//          pthread_t thread;
//          //创建一个新线程
//          int result=pthread_create(&thread,NULL,nativeWorkerThread,(void*)nativeWorkerArgs);
//          if (0!=result){
//              //获取异常类
//              jclass exceptionClazz=env->FindClass("java/lang/RuntimeException");
//              //抛出异常
//              env->ThrowNew(exceptionClazz,"Unable to create thread");
//          }
//      }
//}

