#!/bin/bash
# Second CGI type: this one is executed by /bin/bash, not by python3.
# The server picks the interpreter from the file extension alone
# (cgi_extension .sh /bin/bash), so the exact same /cgi-bin route
# serves both .py and .sh scripts.

# A CGI answers with its own headers, a blank line, then the body.
body_in=""
if [ "$REQUEST_METHOD" = "POST" ] && [ "${CONTENT_LENGTH:-0}" -gt 0 ]; then
	read -r -N "$CONTENT_LENGTH" body_in
fi

# Read a file by relative path: only works because the server chdir()s
# into the script's directory before exec'ing it.
relative_read=$(cat data.txt 2>/dev/null | head -1)

printf 'Content-Type: text/html\r\n'
printf '\r\n'

cat <<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI — info.sh</title>
    <style>
        body { font-family: 'Segoe UI', sans-serif; background:#0f1117; color:#e2e8f0; padding:40px 20px; }
        .container { max-width: 640px; margin: 0 auto; }
        h1 { color: #f6ad55; }
        .badge { display:inline-block; background:#1a202c; border:1px solid #7b341e;
                 border-radius:6px; padding:2px 10px; font-size:0.75rem; color:#f6ad55; margin-bottom:20px; }
        p { color:#a0aec0; line-height:1.7; }
        table { width:100%; border-collapse:collapse; font-size:0.85rem; margin-top:20px; }
        th { text-align:left; color:#718096; padding:6px 12px; border-bottom:1px solid #2d3748; }
        td { padding:8px 12px; border-bottom:1px solid #1a202c; font-family:monospace; }
        .key { color:#90cdf4; } .val { color:#fbb6ce; }
        a { display:inline-block; margin-top:24px; color:#63b3ed; text-decoration:none;
            border:1px solid #2b4c7e; padding:6px 16px; border-radius:6px; }
    </style>
</head>
<body>
<div class="container">
    <div class="badge">CGI ✓ Bash ${BASH_VERSION%%(*}</div>
    <h1>info.sh executed successfully</h1>
    <p>Same <code>/cgi-bin</code> route, different interpreter: the server matched
       the <code>.sh</code> extension and exec'd <code>/bin/bash</code>, while
       <code>hello.py</code> on the very same route is exec'd through
       <code>/usr/bin/python3</code>.</p>
    <table>
        <tr><th>Variable</th><th>Value</th></tr>
        <tr><td class="key">Interpreter</td><td class="val">/bin/bash ${BASH_VERSION}</td></tr>
        <tr><td class="key">REQUEST_METHOD</td><td class="val">${REQUEST_METHOD:-(empty)}</td></tr>
        <tr><td class="key">QUERY_STRING</td><td class="val">${QUERY_STRING:-(empty)}</td></tr>
        <tr><td class="key">PATH_INFO</td><td class="val">${PATH_INFO:-(empty)}</td></tr>
        <tr><td class="key">SERVER_PROTOCOL</td><td class="val">${SERVER_PROTOCOL:-(empty)}</td></tr>
        <tr><td class="key">SERVER_SOFTWARE</td><td class="val">${SERVER_SOFTWARE:-(empty)}</td></tr>
        <tr><td class="key">POST body</td><td class="val">${body_in:-(empty)}</td></tr>
        <tr><td class="key">cat data.txt</td><td class="val">${relative_read:-(failed)}</td></tr>
    </table>
    <a href="/">← Back to home</a>
</div>
</body>
</html>
HTML
