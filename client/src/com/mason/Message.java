package com.mason;

import java.io.ByteArrayOutputStream;
import java.io.Serializable;

/*
enum CMD{
    CMD_SHELL = 1,
    CMD_SQL
};

enum TYPE{
    TYP_ORACLE = 1,
    TYP_MYSQL,
    TYP_POSTGRESQL
};
*/

public class Message implements Serializable {
	/* 常量 */

	public static final short CMD_SHELL = 1;
	public static final short CMD_SQL = 2;
	public static final short TYP_ORACLE = 1;
	public static final short TYP_MYSQL = 1;
	public static final short TYP_POSTGRESQL = 1;
	public static final short TYP_SHELL = 0;
	/* 成员变量 */
	private short type; /* CMD_SHELL or CMD_SQL */
	private short cmd; /* TYP_ORACLE/TYP_MYSQL/TYP_POSTGRESQL */
	private short index;/* index of package */
	private short total;/* total number of package */
	private int len; /* the length of payload */
	private String payload;

	/*
	 * 构造函数
	 **/
	public Message(short type, short cmd, short index, short total, int len, String payload) {
		this.type = type;
		this.cmd = cmd;
		this.index = index;
		this.total = total;
		this.len = len;
		this.payload = payload;
	}

	/*
	 * 构造函数
	 */
	public Message(short type, short cmd, String payload) throws Exception {
		if (payload == null) {
			throw new Exception("paylaod is NULL.");
		}
		this.type = type;
		this.cmd = cmd;
		this.index = (short) 0;
		this.total = (short) 1;
		this.len = payload.length();
		this.payload = payload;
	}

	private byte[] intToBytes(int n) {
		byte[] b = new byte[4];
		b[3] = (byte) (n & 0xff);
		b[2] = (byte) (n >> 8 & 0xff);
		b[1] = (byte) (n >> 16 & 0xff);
		b[0] = (byte) (n >> 24 & 0xff);
		return b;
	}

	private byte[] shortToBytes(short n) {
		byte[] b = new byte[2];
		b[1] = (byte) (n & 0xff);
		b[0] = (byte) (n >> 8 & 0xff);
		return b;
	}

	/*
	 * 将Message对象转成BigEbian的字节码
	 */
	public byte[] toBytes() throws Exception {
		byte[] bytes;

		try {
			ByteArrayOutputStream bo = new ByteArrayOutputStream();
			bo.write(shortToBytes(this.type));
			bo.write(shortToBytes(this.cmd));
			bo.write(shortToBytes(this.index));
			bo.write(shortToBytes(this.total));
			bo.write(intToBytes(this.len));
			bo.write(this.payload.getBytes());
			bo.flush();
			bytes = bo.toByteArray();
			
			for(int i = 0;i<bytes.length;i++) {
				System.out.printf("%02x", bytes[i]);
			}
			System.out.println("");
		} catch (Exception e) {
			throw e;
		}
		return bytes;
	}
}
