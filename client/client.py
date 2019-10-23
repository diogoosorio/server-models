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
REQUEST_THROTTLING = 2
REQUEST_LINE_PREFIX = "thread {} - "

def format_message(message, thread_id):
    prefix_size = len(REQUEST_LINE_PREFIX)
    message_size = len(message)

    if (message_size + prefix_size > REQUEST_LINE_CHARS):
        message = message[0:(REQUEST_LINE_CHARS - prefix_size)]

    message = REQUEST_LINE_PREFIX.format(thread_id) + message
    padding = REQUEST_LINE_CHARS - len(message)
    padded_message = message + (' ' * padding)

    return padded_message.encode('ascii')


def launch_client(thread_id):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect((SERVER_ADDRESS, SERVER_PORT))
        print("%s | thread %d | client connected" % (datetime.now(), thread_id))

        hello_message = format_message("start of message", thread_id);
        client.send(hello_message)
        print("%s | thread %d | sent hello" % (datetime.now(), thread_id,))

        for index in range(0, REQUEST_LINES):
            client.send(format_message(str(index) * REQUEST_LINE_CHARS, thread_id))
            print("%s | thread %d | sent line %d" % (datetime.now(), thread_id, index,))
            time.sleep(REQUEST_THROTTLING)

        goodbye_message = format_message("goodbye", thread_id)
        client.send(goodbye_message)
        print("%s | thread %d | sent goodbye" % (datetime.now(), thread_id))

    print("%s | thread %d | server closed connection" % (datetime.now(), thread_id))


if __name__ == '__main__':
    num_threads = int(sys.argv[1]) if len(sys.argv) > 1 else 1
    threads = []

    for i in range(0, num_threads):
        thread = threading.Thread(target=launch_client, args=(i,))
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()
