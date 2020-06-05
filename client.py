#!/usr/bin/env python3

import socket
import sys
import threading
import time

from datetime import datetime

SERVER_ADDRESS = '127.0.0.1'
SERVER_PORT = 3000

REQUEST_LINE_CHARS = 50
REQUEST_LINES = 5
REQUEST_LINE_PREFIX = "client {} - "


def send_message(client_id, client, content, wait_ack = True):
    # terminates each message with a "ETX" character
    message = (content + "\3").encode('ascii')
    client.send(message)
    print("%s | client %d | sent %s" % (datetime.now(), client_id, content,))

    # waits for a 3 character payload from the server with an ack
    if wait_ack:
        received_message = ""
        while len(received_message) < 3:
            segment = client.recv(1024)
            received_message += segment.decode('utf-8')

        print("%s | client %d | received ack: (%s)" % (datetime.now(), client_id, received_message,))


def launch_client(client_id, throttling):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect((SERVER_ADDRESS, SERVER_PORT))
        print("%s | client %d | client connected" % (datetime.now(), client_id))

        for index in range(0, REQUEST_LINES):
            # send a line of data to the server
            send_message(client_id, client, ("package %d" % (index,)))

            # if argv[2] was defined, wait x seconds before sending the next line
            if throttling > 0 and index < (REQUEST_LINES - 1):
                time.sleep(throttling)

        # send goodby to the server
        send_message(client_id, client, "goodbye", False)

    print("%s | client %d | done!" % (datetime.now(), client_id))


if __name__ == '__main__':
    num_threads = int(sys.argv[1]) if len(sys.argv) > 1 else 1
    req_throttling = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    threads = []

    for i in range(0, num_threads):
        thread = threading.Thread(target=launch_client, args=(i, req_throttling,))
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()
