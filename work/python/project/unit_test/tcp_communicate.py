import argparse
import re
import socket
import socketserver
import time
import threading

def communicate(host, port, request):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s.send(request.encode())
    response = s.recv(1024)
    s.close()

    return response.decode()

def runner_checker(host, port):
    while True:
        time.sleep(1)
        try:
            response = communicate(host, port, "ping")
            if response != "pong":
                print("peer is removed")
                break;
            else:
                print("peer is alive continue")
        except socket.error as e:
            print("err %s"%e)
            break
        

class ThreadingTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    dead = False # Indicate to other threads that we are no longer running


class DispatcherHandler(socketserver.BaseRequestHandler):
    """
    The RequestHandler class for our dispatcher.
    This will dispatch test runners against the incoming commit
    and handle their requests and test results
    """

    command_re = re.compile(r"(\w+)*")
    BUF_SIZE = 1024

    def handle(self):
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(self.BUF_SIZE).strip()
        command_groups = self.command_re.match(self.data.decode())
        if not command_groups:
            self.request.sendall("Invalid command")
            return
        command = command_groups.group(1)
        if command == "ping":
            print("in ping")
            resp = 'pong'
            self.request.sendall(resp.encode())

def serve(host, port):
    # Create the server
    socketserver.TCPServer.allow_reuse_address = True
    server = ThreadingTCPServer((host, port), DispatcherHandler)
    print('serving on %s:%s'% (args.host, int(args.port)))
    try:
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl+C or Cmd+C
        server.serve_forever()
    except (KeyboardInterrupt, Exception):
        # if any exception occurs, kill the thread
        server.dead = True

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument( "mode", choices=['server', 'client'], help="work mode server | client") 
    parser.add_argument("--host",
                        help="dispatcher's host, by default it uses localhost",
                        default="localhost",
                        action="store")
    parser.add_argument("--port",
                        help="dispatcher's port, by default it uses 8888",
                        default=8888,
                        action="store")
    args = parser.parse_args()


    if args.mode == "server":
        serve(args.host, int(args.port))
    else:
        runner_heartbeat = threading.Thread(target=runner_checker, args=(args.host, int(args.port)))
        try:
            runner_heartbeat.start()
            # Activate the server; this will keep running until you
            # interrupt the program with Ctrl+C or Cmd+C
        except (KeyboardInterrupt, Exception):
            # if any exception occurs, kill the thread
            runner_heartbeat.join()


    
