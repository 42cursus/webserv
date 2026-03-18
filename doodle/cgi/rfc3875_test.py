#!/usr/bin/env python3
import os


def v(name):
    return os.environ.get(name, "")


print("REQUEST_METHOD=" + v("REQUEST_METHOD"))
print("SCRIPT_NAME=" + v("SCRIPT_NAME"))
print("PATH_INFO=" + v("PATH_INFO"))
print("QUERY_STRING=" + v("QUERY_STRING"))
print("CONTENT_TYPE=" + v("CONTENT_TYPE"))
print("CONTENT_LENGTH=" + v("CONTENT_LENGTH"))
