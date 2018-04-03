#include <jni.h>
#include <string>
#include <string.h>
#include <malloc.h>
#include <android/log.h>
#include "com_asdf_myndkdemo_DataProvider.h"
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

JNIEXPORT jint JNICALL
Java_com_asdf_myndkdemo_DataProvider_add(JNIEnv *env, jobject instance, jint x, jint y) {
    LOGD("x=%d",x);
    LOGD("y=%d",y);
    return x+y;
}

JNIEXPORT jstring JNICALL
Java_com_asdf_myndkdemo_DataProvider_sayHelloInC(JNIEnv *env, jobject instance, jstring str) {
    //c语言没有java的String类型
    char* cstr=Jstring2CStr(env,str);
    LOGD("cstr=%s",cstr);
    char hellostr[7]={' ','h','e','l','l','o','\0'};
    strcat(cstr,hellostr);
    LOGD("new cstr=%s",cstr);
    return (env)->NewStringUTF(cstr);
}

/**env java 虚拟机 结构体c实现的指针 包含的有很多jni方法
 *jobject obj 代表的是调用这个c代码的java对象 代表的是DataProider的对象
 */
JNIEXPORT jintArray JNICALL
Java_com_asdf_myndkdemo_DataProvider_intMethod(JNIEnv *env, jobject instance, jintArray arrs) {
    //1.知道该数组的长度
    int length=(env)->GetArrayLength(arrs);
    //2.操作数组的每一个值
    jint* shuzhu=(env)->GetIntArrayElements(arrs,0);
    int i=0;
    for(;i<length;i++){
        LOGD("shuzhu[%d]=%d",i,*(shuzhu+i));
        *(shuzhu+i)+=10;
    }
    //    void        (*ReleaseIntArrayElements)(JNIEnv*, jintArray,
    //                        jint*, jint);
    //

    //	(*env)->ReleaseIntArrayElements(env,arr,intarr,0); // c语言释放掉 刚才申请的内存空间
    return arrs;
}


/**
 * 代表的是调用c代码 的class类
 * jclass DataProvider  类
 */
JNIEXPORT jint JNICALL
Java_com_asdf_myndkdemo_DataProvider_sub(JNIEnv *env, jclass type, jint x, jint y) {
    LOGD("x=%d",x);
    LOGD("y=%d",y);
    return x-y;
}

JNIEXPORT void JNICALL Java_com_asdf_myndkdemo_DataProvider_accessMethod
        (JNIEnv *env, jobject object){
    jthrowable ex;
    jclass theClass=env->GetObjectClass(object);
    jmethodID methodId=env->GetMethodID(theClass,"throwingMethod","()V");
    env->CallVoidMethod(object,methodId);
    ex=env->ExceptionOccurred();
    if (ex!=0){
        env->ExceptionClear();
    }
}

extern "C"
jstring
Java_com_asdf_myndkdemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


