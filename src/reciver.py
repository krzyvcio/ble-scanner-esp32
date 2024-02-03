import json
import http.server
import sqlite3

conn = sqlite3.connect('ble.db')
c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS ble (id INTEGER PRIMARY KEY, data TEXT, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)''')
conn.commit()
conn.close()

class RequestHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        json_data = json.loads(post_data)
        print(json_data)
        conn = sqlite3.connect('ble.db')
        c = conn.cursor()
        c.execute("INSERT INTO ble (data) VALUES (?)", (json.dumps(json_data),))  # Convert dictionary to string using json.dumps()
        conn.commit()
        conn.close()

        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()

server_address = ('', 80)
httpd = http.server.HTTPServer(server_address, RequestHandler)
httpd.serve_forever()
