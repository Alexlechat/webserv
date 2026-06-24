#!/usr/bin/env python3
import os
import sys

# CGI scripts must print the full HTTP response headers first,
# then a blank line, then the body.
# Your server reads this stdout and forwards it to the browser.

method  = os.environ.get("REQUEST_METHOD", "GET")
query   = os.environ.get("QUERY_STRING", "")
path    = os.environ.get("PATH_INFO", "")

# Read POST body if present
body_in = ""
if method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0) or 0)
    if length > 0:
        body_in = sys.stdin.read(length)

print("Content-Type: text/html\r")
print("\r")
print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI — hello.py</title>
    <style>
        body {{ font-family: 'Segoe UI', sans-serif; background:#0f1117; color:#e2e8f0; padding:40px 20px; }}
        .container {{ max-width: 640px; margin: 0 auto; }}
        h1 {{ color: #68d391; }}
        .badge {{ display:inline-block; background:#1a202c; border:1px solid #276749;
                    border-radius:6px; padding:2px 10px; font-size:0.75rem; color:#68d391; margin-bottom:20px; }}
        table {{ width:100%; border-collapse:collapse; font-size:0.85rem; margin-top:20px; }}
        th {{ text-align:left; color:#718096; padding:6px 12px; border-bottom:1px solid #2d3748; }}
        td {{ padding:8px 12px; border-bottom:1px solid #1a202c; font-family:monospace; }}
        .key {{ color:#90cdf4; }} .val {{ color:#fbb6ce; }}
        a {{ display:inline-block; margin-top:24px; color:#63b3ed; text-decoration:none;
                border:1px solid #2b4c7e; padding:6px 16px; border-radius:6px; }}
    </style>
</head>
<body>
<div class="container">
    <div class="badge">CGI ✓ Python3</div>
    <h1>hello.py executed successfully</h1>
    <p style="color:#a0aec0">Your server forked, ran this Python script via execve, and piped its stdout back as the HTTP response.</p>
    <table>
        <tr><th>Variable</th><th>Value</th></tr>
        <tr><td class="key">REQUEST_METHOD</td><td class="val">{method}</td></tr>
        <tr><td class="key">QUERY_STRING</td><td class="val">{query}</td></tr>
        <tr><td class="key">PATH_INFO</td><td class="val">{path}</td></tr>
        <tr><td class="key">POST body</td><td class="val">{body}</td></tr>
    </table>
    <a href="/">← Back to home</a>
</div>
</body>
</html>""".format(method=method, query=query or "(empty)", path=path or "(empty)", body=body_in or "(empty)"))
