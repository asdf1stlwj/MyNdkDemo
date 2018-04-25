package com.asdf.echosocket;

public class EchoServerActivity extends AbstractSocketActivity {
    @Override
    protected int getLayoutId() {
        return R.layout.activity_echo_server;
    }

    @Override
    protected void onStartButtonClicked() {
        Integer port=getPort();
        if (port!=null){
            ServerTask serverTask=new ServerTask(port);
            serverTask.start();
        }
    }

    /**
     * 根据给定端口启动TCP服务
     * @param port 端口号
     * @throws Exception
     */
    private native void nativeStartTcpServer(int port) throws Exception;

    /**
     * 根据给定端口启动UDP服务
     * @param port
     * @throws Exception
     */
    private native void nativeStartUdpServer(int port) throws Exception;


    private class ServerTask extends AbstractSocketActivity.AbstractEchoTask {
        //端口号
        private final int port;

        private ServerTask(int port){
            this.port=port;
        }

        @Override
        protected void onBackground() {
            logMessage("Starting Server");
            try {
                //nativeStartTcpServer(port);
                nativeStartUdpServer(port);
            }catch (Exception e){
                logMessage(e.getMessage());
            }
            logMessage("Server terminated");
        }
    }
}
