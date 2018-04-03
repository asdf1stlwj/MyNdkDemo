package com.asdf.myndkdemo;

public class DataProvider {
     static {
          System.loadLibrary("native-lib");
     }
     public native int add(int x,int y);
     public native String sayHelloInC(String str);
     public native int[] intMethod(int[] arrs);
     public static native int sub(int x,int y);

     /**
      * 抛出方法
      * @throws NullPointerException
      */
     public void throwingMethod ()throws NullPointerException{
          throw new NullPointerException("pointer null");
     }

     public native void accessMethod();
}
