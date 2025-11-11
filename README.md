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
docker exec -it -u root webserv-nginx-1 bash
```

```bash
echo 12346 | dd bs=1 count=3 status=none
```