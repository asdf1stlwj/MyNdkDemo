package com.asdf.ndkthread;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {
    EditText et_thread,et_iteration;
    Button btn_start;
    TextView tv_log;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initNative();
        et_thread= (EditText) findViewById(R.id.thread_edit);
        et_iteration= (EditText) findViewById(R.id.iterations_edit);
        btn_start= (Button) findViewById(R.id.start_button);
        tv_log= (TextView) findViewById(R.id.log_view);
        btn_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int threadc=getEditNum(et_thread,0);
                int iterationc=getEditNum(et_iteration,0);
                if (threadc>0 && iterationc>0){
                    startThreads(threadc,iterationc);
                }
            }
        });
    }

    private int getEditNum(EditText editText,int defalutValue){
        int value;
        try {
            value=Integer.valueOf(editText.getText().toString());
        }catch (NumberFormatException e){
            value=defalutValue;
        }
        return value;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        freeNative();
    }

    public void onNativeMessage(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                tv_log.append(message);
                tv_log.append("\n");
            }
        });

    }

    private void javaThread(int threadc, final int iteration){
        for (int i=0;i<threadc;i++){
            final int id = i;
            Thread thread=new Thread(new Runnable() {
                @Override
                public void run() {
                    nativeWorker(id,iteration);
                }
            });
            thread.start();
        }
    }

    /**
     * 启动给定个数的线程进行迭代
     * @param threrad
     * @param iteration
     */
    private void startThreads(int threrad,int iteration){
        posixThreads(threrad,iteration);
        //javaThread(threrad,iteration);
    }

    /**
     * 初始化原生代码
     */
    private native void initNative();

    /**
     * 释放原生资源
     */
    private native void freeNative();

    /**
     * 使用Posix 线程
     * @param threads
     * @param iteration
     */
    private native void posixThreads(int threads,int iteration);
    /**
     * 原生worker
     * @param id
     * @param iteration
     */
    private native void nativeWorker(int id,int iteration);

    static {
        System.loadLibrary("Thread");
    }
}
