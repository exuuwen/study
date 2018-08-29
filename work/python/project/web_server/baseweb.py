#!/usr/bin/python

import BaseHTTPServer
import os


class case_others(object):
    '''File or directory does not exist or others.'''

    def test(self, handler):
        return True #not os.path.isexist(handler.full_path):

    def act(self, handler):
        handler.handle_noexist(handler.full_path)

class case_file(object):
    '''File does exist.'''

    def test(self, handler):
        return os.path.isfile(handler.full_path)

    def act(self, handler):
        handler.handle_file(handler.full_path)

class case_dir(object):
    '''directory does exist.'''

    def test(self, handler):
        return os.path.isdir(handler.full_path)
 
    def act(self, handler):
        handler.handle_dir(handler.full_path)


class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    '''Handle HTTP requests by returning a fixed 'page'.'''

    # Page to send back.
    Page = '''\
<html>
<body>
<table>
<tr>  <td>Header</td>         <td>Value</td>          </tr>
<tr>  <td>Date and time</td>  <td>{date_time}</td>    </tr>
<tr>  <td>Client host</td>    <td>{client_host}</td>  </tr>
<tr>  <td>Client port</td>    <td>{client_port}</td> </tr>
<tr>  <td>Command</td>        <td>{command}</td>      </tr>
<tr>  <td>Path</td>           <td>{path}</td>         </tr>
</table>
</body>
</html>
'''

    Error_Page = """\
<html>
<body>
<h1>Error accessing {path}</h1>
<p>{msg}</p>
</body>
</html>
"""

    List_Page = """\
<html>
<body>
<ul>
{0}
</ul>
</body>
</html>
"""

    file_path = "/static"

    cases = [case_file(), case_dir(), case_others()]

# Handle a GET request.
    def do_GET(self):
        self.full_path = os.getcwd() + self.file_path + self.path
        for case in self.cases:
            if case.test(self):
                case.act(self)
                break

    def handle_file(self, full_path):
        try:
            with open(full_path, 'rb') as reader:
                content = reader.read()
            self.send_content(content)
        except  IOError as msg:
            msg = "'{0}' cannot be read: {1}".format(self.path, msg)
            self.handle_error(msg)
    
    def handle_noexist(self, full_path):
        content = self.create_content()
        self.send_content(content)

    def handle_dir(self, full_path):
        try:
            entries = os.listdir(full_path)
            bullets = ['<li>{0}</li>'.format(e) 
                for e in entries if not e.startswith('.')]
            content = self.List_Page.format('\n'.join(bullets))
            self.send_content(content)
        except OSError as msg:
            msg = "'{0}' cannot be listed: {1}".format(self.path, msg)
            self.handle_error(msg)

    def create_content(self):
        values = {
            'date_time'   : self.date_time_string(),
            'client_host' : self.client_address[0],
            'client_port' : self.client_address[1],
            'command'     : self.command,
            'path'        : self.path
        }
        content = self.Page.format(**values)
        return content

    def send_content(self, content, status=200):
        self.send_response(status)
        self.send_header("Content-Type", "text/html")
        self.send_header("Content-Length", str(len(content)))
        self.end_headers()
        self.wfile.write(content)

    def handle_error(self, msg):
        content = self.Error_Page.format(path=self.path, msg=msg)
        self.send_content(content, 404)


if __name__ == '__main__':
    serverAddress = ('', 3389)
    server = BaseHTTPServer.HTTPServer(serverAddress, RequestHandler)
    server.serve_forever()

