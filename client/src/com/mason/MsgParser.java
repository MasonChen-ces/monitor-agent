package com.mason;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;

public class MsgParser {
	private boolean header;
	private ByteArrayOutputStream cache;

	public MsgParser() {
		this.header = false;
		this.cache = new ByteArrayOutputStream();
	}

	private boolean isServerClose(Socket socket) {
		try {
			socket.sendUrgentData(0xFF);
			return false;
		} catch (Exception e) {
			e.printStackTrace();
			return true;
		}
	}

	public static int bytesToInt(byte[] bytes) {
		if (bytes.length != 4)
			return 0;
		int ret = bytes[3] & 0xFF | 
				(bytes[2] & 0xFF) << 8 | 
				(bytes[1] & 0xFF) << 16 | 
				(bytes[0] & 0xFF) << 24;
		return ret;
	}

	public void readfromFD(Socket socket, RecvMessage message) throws Exception {
		if (socket == null) {
			throw new IOException("The socket is null.");
		}

		if (socket.isClosed()) {
			throw new IOException("The socket is closed.");
		}
		if (!socket.isConnected()) {
			throw new IOException("The socket disconnected.");
		}
		if (socket.isInputShutdown()) {
			throw new IOException("The socket disconnected.");
		}
		InputStream is = socket.getInputStream();
		if (is == null) {
			return;
		}
		if (isServerClose(socket)) {
			throw new IOException("The server socket disconnected.");
		}
		if (!this.header) {
			byte bytes[] = new byte[socket.getReceiveBufferSize()];
			
			int l = is.read(bytes);
			if (l >= 4) {
				byte tmp[] = new byte[4];
				System.arraycopy(bytes, 0, tmp, 0, 4);
				message.len = bytesToInt(tmp);
				this.header = true;
				if(l>4) {
					tmp = new byte[l-4];
					System.arraycopy(bytes, 4, tmp, 0, l-4);
					this.cache.write(new String(tmp,0,l-4).getBytes());
				}

			}else {
				System.out.println(socket.getReceiveBufferSize());
			}
		} else {
			if (this.cache.size() >= message.len) {
				message.payload = new byte[this.cache.size()];
				System.arraycopy(this.cache.toByteArray(), 0, message.payload, 0, this.cache.size());
				this.cache.close();
				return;
			} else {
				byte bytes[] = new byte[message.len - this.cache.size()];
				int l = is.read(bytes);
				if (l > 0) {
					this.cache.write((new String(bytes, 0, l)).getBytes());
				}
			}
		}
		Thread.sleep(20);
		readfromFD(socket, message);
	}
}