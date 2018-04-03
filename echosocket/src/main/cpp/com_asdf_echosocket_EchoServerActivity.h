#include <jni.h>
/* Header for class com_asdf_echosocket_EchoServerActivity */

#ifndef _Included_com_asdf_echosocket_EchoServerActivity
#define _Included_com_asdf_echosocket_EchoServerActivity
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_asdf_echosocket_EchoServerActivity
 * Method:    nativeStartTcpServer
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_asdf_echosocket_EchoServerActivity_nativeStartTcpServer
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_asdf_echosocket_EchoServerActivity
 * Method:    nativeStartUdpServer
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_asdf_echosocket_EchoServerActivity_nativeStartUdpServer
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif
