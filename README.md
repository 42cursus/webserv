"In this project bugs usually come from lifetime and ownership confusion,
and storing references inside polymorphic objects is a great way to manufacture that confusion." (c)

```bash

chmod +x ubuntu_cgi_tester

curl -v -X POST http://127.0.0.1:8080/directory/youpi.bla \
  -H "Content-Type: plain/text" \
  --data "BODY IS HERE" 
  
dd if=/dev/zero bs=1M count=100 status=none | \
curl -v -X POST --data-binary @- \
     -H "Content-Type: application/octet-stream" \
     --no-progress-meter \
     http://127.0.0.1:8080/directory/youpi.bla | xxd
```

```bash
dd if=/dev/zero bs=100 count=2 status=none | \
  curl -v -X POST --data-binary @- \
  -H "Content-Type: application/octet-stream" \
  --no-progress-meter \
  http://127.0.0.1:8080/post_body
```

```bash
docker exec -it -u root webserv-nginx-1 bash
```

```bash
echo 12346 | dd bs=1 count=3 status=none
```

```bash
sudo apt install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

#

[RFC 7230 (HTTP/1.1 Message Syntax and Routing) 6.3.2.  Pipelining](https://datatracker.ietf.org/doc/html/rfc7230#section-6.3.2)

[RFC 9112 (HTTP/1.1) 9.3.2. Pipelining ](https://datatracker.ietf.org/doc/html/rfc9112#section-9.3.2)

```bash
printf 'HEAD / HTTP/1.1\r\nHost: www.google.com\r\nConnection: keep-alive\r\n\r\nHEAD /teapot HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n' \
  | nc -N google.com 80

printf '%s' \
  $'HEAD / HTTP/1.1\r\n'\
  $'Host: www.google.com\r\n'\
  $'Connection: keep-alive\r\n'\
  $'\r\n'\
  $'HEAD /teapot HTTP/1.1\r\n'\
  $'Host: www.google.com\r\n'\
  $'Connection: close\r\n'\
  $'\r\n' \
  | nc -N google.com 80

cat <<'EOF' | nc -N google.com 80
HEAD / HTTP/1.1
Host: www.google.com
Connection: keep-alive

HEAD /teapot HTTP/1.1
Host: www.google.com
Connection: close

EOF

printf 'GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\nGET /teapot HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n' \
  | nc -N 127.1 8080

curl --http1.1 -v http://127.0.0.1:8080/ http://127.0.0.1:8080/teapot
```

### Links:
- [Apache HTTP Server](https://github.com/apache/httpd)
- [NGINX Web Server](https://github.com/nginx/nginx)
- [CivetWeb](https://github.com/civetweb/civetweb)
- [Muduo C++ network library](https://github.com/chenshuo/muduo)
- [POCO (Portable Components)](https://github.com/pocoproject/poco)
- [TinyWebServer](https://github.com/qinguoyi/TinyWebServer)
- [markparticle's WebServer](https://github.com/markparticle/WebServer)
- [Libevent’s evbuffer functionality](https://libevent.org/libevent-book/Ref7_evbuffer.html)
- [HTTP built on Boost.Asio](https://github.com/boostorg/beast/blob/develop/example/http/server/async/http_server_async.cpp)