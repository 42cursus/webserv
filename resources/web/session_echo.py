#!/usr/bin/env python3
import os
import time


def parse_cookie(header):
    cookies = {}
    if not header:
        return cookies
    parts = header.split(";")
    for part in parts:
        if "=" not in part:
            continue
        k, v = part.strip().split("=", 1)
        cookies[k] = v
    return cookies


cookie_header = os.environ.get("HTTP_COOKIE", "")
cookies = parse_cookie(cookie_header)
sid = cookies.get("ws_sid")
new_sid = False
if not sid:
    sid = str(int(time.time()))
    new_sid = True

body = []
body.append("session_echo.py")
body.append("sid=" + sid)
body.append("cookie_header=" + (cookie_header if cookie_header else "(none)"))
payload = "\n".join(body) + "\n"

print("Status: 200 OK")
print("Content-Type: text/plain")
print("Content-Length: " + str(len(payload)))
if new_sid:
    print("Set-Cookie: ws_sid=" + sid + "; Path=/; HttpOnly; SameSite=Lax")
print("")
print(payload, end="")
