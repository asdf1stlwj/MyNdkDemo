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


    // adb -s <模拟器识别号> forward tcp:<pc端口号> tcp:<模拟器端口号>
    //通过这条命令使数据通过端口号从pc转发到模拟器,实现与模拟器的通信
    //模拟器识别号:端口转发需指定的socket服务端所在模拟器的识别号,可用adb devices查看当前连接的所有模拟器识别号
    //pc端口号:模拟器的宿主的一个端口号,随便指定,只要该端口号没被占用即可,可用 netstat -an |findstr <端口号> 来确认改端口是否被占用
    //模拟器端口号:这由之前的socket服务端决定,为socket服务端的socket绑定的端口号
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
