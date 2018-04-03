
package com.asdf.echosocket;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.PersistableBundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;

/**
 * 抽象 echo Activity
 * Created by User on 2018/4/2.
 */

public abstract class AbstractSocketActivity extends Activity implements View.OnClickListener{
    //端口号
    protected EditText portEdit;

    //服务按钮
    protected Button startButton;

    //日志滚动
    protected ScrollView logScroll;

    //日志视图
    protected TextView logView;

    //布局ID
    private final int layoutID;

    /**
     *
     * @param layoutID
     */
    protected AbstractSocketActivity(int layoutID) {
        this.layoutID = layoutID;
    }

    @Override
    public void onCreate(Bundle savedInstanceState, PersistableBundle persistentState) {
        super.onCreate(savedInstanceState, persistentState);
        setContentView(layoutID);
        portEdit= ((EditText) findViewById(R.id.port_edit));
        startButton= ((Button) findViewById(R.id.start_button));
        logScroll= ((ScrollView) findViewById(R.id.log_scroll));
        logView= ((TextView) findViewById(R.id.log_view));
        startButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.start_button:
                onStartButtonClicked();
                break;
        }
    }

    protected abstract void onStartButtonClicked();

    //以整型获取端口号
    protected Integer getPort(){
        Integer port;
        try {
            port=Integer.valueOf(portEdit.getText().toString());
        }catch (NumberFormatException e){
            port=null;
        }
        return port;
    }

    protected void logMessage(final String  message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logMessageDirect(message);
            }
        });
    }

    /**
     * 直接记录给定信息
     * @param message
     */
    protected void logMessageDirect(final String message){
        logView.append(message);
        logView.append("\n");
        logScroll.fullScroll(View.FOCUS_DOWN);
    }

    protected abstract class AbstractEchoTask extends Thread{
        private final Handler handler;


        protected AbstractEchoTask() {
            handler = new Handler();
        }

        /**
         * 在调用线程中先执行回调
         */
        protected void onPreExecute(){
            startButton.setEnabled(false);
            logView.setText("");
        }

        @Override
        public synchronized void start(){
            onPreExecute();
            super.start();
        }

        public void run(){
            onBackground();
            handler.post(new Runnable() {
                @Override
                public void run() {
                   onPostExecute();
                }
            });
        }

        /**
         * 新线程中的背景回调
         */
        protected abstract void onBackground();

        /**
         * 在调用线程中执行回调
         */
        protected void onPostExecute(){
            startButton.setEnabled(true);
        }
    }

    static {
        System.loadLibrary("Echo");
    }
}
