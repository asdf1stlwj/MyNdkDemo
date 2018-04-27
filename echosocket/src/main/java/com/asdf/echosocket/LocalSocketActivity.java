package com.asdf.echosocket;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.widget.EditText;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;

public class LocalSocketActivity extends AbstractSocketActivity {
    //消息编辑
    private EditText messageEdit;

    @Override
    protected int getLayoutId() {
        return R.layout.activity_echo_client;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        messageEdit= (EditText) findViewById(R.id.message_edit);
    }



    @Override
    protected void onStartButtonClicked() {
        String name=portEdit.getText().toString();
        String message=messageEdit.getText().toString();
        if ((name.length()>=0) && (message.length()!=0)){
            String socketName;
            //如果是filesystem socket,预先准备应用程序的文件目录
            if (isFilesystemSocket(name)){
                File file=new File(getFilesDir(),name);
                socketName=file.getAbsolutePath();
            }else {
                socketName=name;
            }
            ServerTask serverTask=new ServerTask(socketName);
            serverTask.start();
            ClientTask clientTask=new ClientTask(socketName,message);
            clientTask.start();
        }
    }

    private boolean isFilesystemSocket(String name) {
        return name.startsWith("/");
    }


    private class ClientTask extends AbstractEchoTask{
        //socket名称
        final String name;
        //发送的文本信息
        String message;
        public ClientTask(String name, String message) {
            this.name=name;
            this.message=message;
        }

        protected void onBackground(){
            logMessage("Starting client.");
            try {
                //nativeStartTcpClient(ip,port,message);
                //nativeStartUdpClient(ip,port,message);
                startLocalClient(name,message);
            } catch (Throwable e) {
                //e.printStackTrace();
                logMessage(e.getMessage());
            }
            logMessage("Client terminated");
        }
    }

    private class ServerTask extends AbstractSocketActivity.AbstractEchoTask {
        //端口号
        private final String  name;

        private ServerTask(String name){
            this.name=name;
        }

        @Override
        protected void onBackground() {
            logMessage("Starting Server");
            try {
                //nativeStartTcpServer(port);
                //nativeStartUdpServer(port);
                nativeStartLocalServer(name);
            }catch (Exception e){
                logMessage(e.getMessage());
            }
            logMessage("Server terminated");
        }
    }

    /**
     * 启动绑定到给定名称的本地UNIX socket服务器
     * @param name
     */
    private native void nativeStartLocalServer(String name)
                         throws Exception;

    private void startLocalClient(String name,String message)
                                 throws Exception{
        //构造一个本地socket
        LocalSocket clientSocket=new LocalSocket();
        try {
            //设置scket名称空间
            LocalSocketAddress.Namespace namespace;
            if (isFilesystemSocket(name)){
                namespace=LocalSocketAddress.Namespace.FILESYSTEM;
            }else {
                namespace=LocalSocketAddress.Namespace.ABSTRACT;
            }
            //构造本地socket地址
            LocalSocketAddress address=new LocalSocketAddress(name,namespace);
            //连接到本地socket
            logMessage("Connecting to "+name);
            clientSocket.connect(address);
            logMessage("Connected.");
            //以字节形式获取信息
            byte[] messageBytes=message.getBytes();
            //发送消息字节到socket
            logMessage("Sending to the socket...");
            OutputStream outputStream=clientSocket.getOutputStream();
            outputStream.write(messageBytes);
            logMessage(String.format("Sent %d bytes: %s",messageBytes.length,message));
            //从socket中接收消息返回
            logMessage("Receive frmo the socket...");
            InputStream inputStream=clientSocket.getInputStream();
            int readSize=inputStream.read(messageBytes);
            String receivedMessage=new String(messageBytes,0,readSize);
            logMessage(String.format("Received %d bytes: %s",readSize,receivedMessage));
            //关闭流
            outputStream.close();
            inputStream.close();
        }finally {
            //关闭本地socket
            clientSocket.close();
        }
    }
}
