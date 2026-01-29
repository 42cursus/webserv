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


### Links:
- [Apache HTTP Server](https://github.com/apache/httpd)
- [NGINX Web Server](https://github.com/nginx/nginx)
- [Muduo C++ network library](https://github.com/chenshuo/muduo)
- [POCO (Portable Components)](https://github.com/pocoproject/poco)
- [TinyWebServer](https://github.com/qinguoyi/TinyWebServer)
- [markparticle's WebServer](https://github.com/markparticle/WebServer)