package com.mason;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

public class Client {
	private final int MAX_BUF_SIZE = 131072;

	public static void main(String[] args) throws Exception {
		short type = Message.TYP_MYSQL;
		short cmd = Message.CMD_SHELL;
		short index = 0;
		String payload = "ls";
		Message m = new Message(type, cmd, index, (short) 1, payload.length(), payload);
		Client client = new Client();
		String sqltext = "#!/bin/sh\n" + "sqlplus scott/tiger@192.168.1.104:1521/orcl<<EOF\n"
				+ "set line 200 pages 2000\n" + "select * from emp  order by 1;\n" + "exit\n" + "EOF\n";
		String pytext = "#!/usr/bin/python3\n"
				+ "print(\"hello world!\")";
		try {
			String ret = "";
			Socket socket = client.connect2Host("192.168.1.246", 8080, 128);
			socket.setReceiveBufferSize(1024 * 1024);
			System.out.println(socket.getReceiveBufferSize());

			System.gc();
			client.sendMessage(socket, Message.TYP_SHELL, Message.CMD_SHELL, sqltext);
			ret = client.readResult2(socket);
			System.out.println(ret);
			ret = null;
			System.out.println("---------------------------------------------------------\n");
			System.gc();
			
			client.sendMessage(socket, Message.TYP_SHELL, Message.CMD_SHELL, "df -m");
			ret = client.readResult2(socket);
			System.out.println(ret);
			ret = null;
			System.out.println("---------------------------------------------------------\n");
			client.close(socket);

		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	/*
	 * Function: connect
	 * 
	 * @hostname: agent所在主机名
	 * 
	 * @port: agent服务端口
	 */
	public Socket connect2Host(String hostname, int port, int timeout_in_second) throws IOException {
		Socket socket = null;
		try {
			socket = new Socket(hostname, port);
			socket.setKeepAlive(false);
			socket.setSoTimeout(1000 * timeout_in_second);

		} catch (IOException e) {
			throw e;
		}
		return socket;
	}

	/*
	 * function: sendMessage description: 向agent发送命令
	 * 
	 * @socket: 连接到agent的socket
	 * 
	 * @m: 要发送的命令包装
	 */
	public void sendMessage(Socket socket, Message m) throws Exception {

		if (socket == null) {
			throw new IOException("The socket is null.");
		}

		if (socket.isClosed()) {
			throw new IOException("The socket is closed.");
		}
		if (!socket.isConnected()) {
			throw new IOException("The socket disconnected.");
		}
		OutputStream os = socket.getOutputStream();
		os.write(m.toBytes());
		os.flush();
	}

	public void sendMessage(Socket socket, short type, short cmd, String payload) throws Exception {
		Message m = new Message(type, cmd, payload);
		sendMessage(socket, m);
	}

	/*
	 * function: readResult description: 从socket读取agent的执行结果
	 * 
	 * @socket: 连接到agent的socket
	 */
	public String readResult2(Socket socket) throws Exception {
		byte[] bytes = new byte[MAX_BUF_SIZE];
		if (socket == null) {
			throw new IOException("The socket is null.");
		}

		if (socket.isClosed()) {
			throw new IOException("The socket is closed.");
		}
		if (!socket.isConnected()) {
			throw new IOException("The socket disconnected.");
		}

		MsgParser p = new MsgParser();
		RecvMessage m = new RecvMessage();
		InputStream is = socket.getInputStream();

		p.readfromFD(socket, m);
		if (m.payload != null) {
			String ret = new String(m.payload);
			return ret;
		} else {
			return null;
		}
	}

	/*
	 * function: close description: 关闭agent连接
	 * 
	 * @socket: 连接到agent的socket
	 */
	public void close(Socket socket) throws Exception {
		try {
			socket.shutdownOutput();
			socket.shutdownInput();
			socket.close();
		} catch (Exception e) {
			throw e;
		}
	}

}
