package com.asdf.echosocket;

import android.os.Bundle;
import android.widget.EditText;

public class EchoClientActivity extends AbstractSocketActivity {
    //id地址
    private EditText ipEdit;

    //消息编辑
    private EditText messageEdit;

    @Override
    protected int getLayoutId() {
        return R.layout.activity_echo_client;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ipEdit= (EditText) findViewById(R.id.ip_edit);
        messageEdit= (EditText) findViewById(R.id.message_edit);
    }

    @Override
    protected void onStartButtonClicked() {
        String ip=ipEdit.getText().toString();
        Integer port=getPort();
        String message=messageEdit.getText().toString();
        if ((ip.length()!=0)&& (port!=null) && (message.length()!=0)){
            ClientTask clientTask=new ClientTask(ip,port,message);
            clientTask.start();
        }
    }

    private native void nativeStartTcpClient(String ip,int port,String message)
            throws Exception;

    private class ClientTask extends AbstractEchoTask{
        //连接的ip地址
        String ip;
        //端口号
        Integer port;
        //发送的文本信息
        String message;
        public ClientTask(String ip, Integer port, String message) {
            this.ip=ip;
            this.port=port;
            this.message=message;
        }

        protected void onBackground(){
            logMessage("Starting client.");
            try {
                nativeStartTcpClient(ip,port,message);
            } catch (Throwable e) {
                //e.printStackTrace();
                logMessage(e.getMessage());
            }
            logMessage("Client terminated");
        }
    }
}
