package db

import (
	"fmt"
	"net"
)

type KeyValueDb struct {
	host string
	conn net.Conn
}

func ok(e error) {
	if e != nil {
		panic(e)
	}
}

func NewKeyValueDb(host string) (db *KeyValueDb, err error) {
	conn, err := net.Dial("tcp", host)

	return &KeyValueDb{
		host: host,
		conn: conn,
	}, err
}

func (db *KeyValueDb) ParseResponse() (resp_buf []byte, err error) {

	// initialized data
	var resp_len int64 = 0
	var resp_len_buf []byte = make([]byte, 8)
	var n int

	n, err = db.conn.Read(resp_len_buf)
	if err != nil {
		return nil, fmt.Errorf("failed to read response length: %w", err)
	}
	// error check
	if n != 8 {
		return nil, fmt.Errorf("failed to read response length")
	}

	// big endian -> little endian
	for i := 7; i >= 0; i-- {
		resp_len = (resp_len << 8) | int64(resp_len_buf[i])
	}

	// error check
	if resp_len <= 0 {
		return nil, fmt.Errorf("invalid response length")
	}

	// allocate a response buffer
	resp_buf = make([]byte, resp_len)

	// receive the response
	n, err = db.conn.Read(resp_buf)
	ok(err)

	// error check
	if int64(n) != resp_len {
		return nil, fmt.Errorf("incomplete response")
	}

	return resp_buf, err
}

func (db *KeyValueDb) Get(key string) (response []byte, err error) {

	// error check
	if db.conn == nil {
		const maxRetries = 3
		for i := 0; i < maxRetries; i++ {
			if err := db.Reconnect(); err == nil {
				break
			}
			if i == maxRetries-1 {
				return nil, fmt.Errorf("no active connection")
			}
		}
	}

	// construct the get command
	req := serialize_request(fmt.Sprintf("get %s", key))

	// Send the request to the server
	_, err = db.conn.Write(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send request: %w", err)
	}

	// read the response from the server
	buf, err := db.ParseResponse()
	if err != nil {
		return nil, fmt.Errorf("failed to parse response: %w", err)
	}

	return buf, nil
}

func (db *KeyValueDb) Set(key string, value string) (response []byte, err error) {

	// error check
	if db.conn == nil {
		const maxRetries = 3
		for i := 0; i < maxRetries; i++ {
			if err := db.Reconnect(); err == nil {
				break
			}
			if i == maxRetries-1 {
				return nil, fmt.Errorf("no active connection")
			}
		}
	}

	// construct the set command
	req := serialize_request(fmt.Sprintf("set %s %s", key, value))

	// Send the request to the server
	_, err = db.conn.Write(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send request: %w", err)
	}

	// read the response from the server
	buf, err := db.ParseResponse()
	if err != nil {
		return nil, fmt.Errorf("failed to parse response: %w", err)
	}

	return buf, nil
}

func (db *KeyValueDb) Reconnect() error {

	var err error = nil

	db.conn, err = net.Dial("tcp", db.host)

	// done
	return err
}

func (db *KeyValueDb) Close() error {

	// error check
	if db.conn == nil {
		return fmt.Errorf("no active connection")
	}

	// construct the exit command
	req := serialize_request("exit")

	// send the exit command to the server
	_, err := db.conn.Write(req)
	if err != nil {
		return fmt.Errorf("failed to send exit command: %w", err)
	}

	// close the connection
	return nil
}

func serialize_request(command string) []byte {
	req_len := int64(len(command))
	req_len_buf := make([]byte, 8)
	for i := 0; i < 8; i++ {
		req_len_buf[i] = byte(req_len & 0xFF)
		req_len >>= 8
	}
	return append(req_len_buf, []byte(command)...)
}
