#ifndef UPLOAD_HTML_H
#define UPLOAD_HTML_H

const char upload_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>File Uploader</title>
    <meta name="theme-color" content="#00878f">
    <meta content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0' name='viewport'>
    <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
    <style>
        body {
            background: #f4f8fb;
            font-family: 'Roboto', sans-serif;
            margin: 0;
        }
        .controller-card {
            max-width: 420px;
            margin: 30px auto;
            background: #fff;
            border-radius: 18px;
            box-shadow: 0 4px 24px #002F7A22;
            padding: 24px 18px 18px 18px;
        }
        header {
            background-color: #002F7A;
            color: white;
            padding: 12px;
            font-family: 'Roboto', sans-serif;
            box-shadow: 1px 1px 5px #555555;
            border-radius: 12px 12px 0 0;
            text-align: center;
        }
        h1 {
            margin: 0px;
            font-size: 28px;
            font-weight: 600;
        }
        .info-text {
            text-align: left;
            color: #444;
            margin-bottom: 18px;
            font-size: 15px;
        }
        form {
            margin: 0px auto 8px auto;
            display: flex;
            flex-direction: column;
            gap: 12px;
            align-items: center;
        }
        input[type="file"] {
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 14px;
            background: #f8f9fa;
        }
        .button {
            text-decoration: none;
            border: none;
            color: white;
            background-color: #002F7A;
            padding: 10px 32px;
            font-size: 16px;
            cursor: pointer;
            box-shadow: 1px 1px 6px #555;
            outline: none;
            margin: 12px auto 0px auto;
            display: inline-block;
            border-radius: 8px;
            transition: background 0.2s;
        }
        .button:hover {
            background: #00878f;
        }
        @media (max-width: 500px) {
            .controller-card { max-width: 98vw; }
        }
    </style>
</head>
<body>
    <div class="controller-card">
        <header>
            <h1>File Uploader</h1>
        </header>
        <div>
            <p class="info-text">
                Use this page to upload new files to the ESP8266.<br/>
                You can use compressed (deflated) files (<b>.gz</b>) to save space and bandwidth.
            </p>
            <form method="post" enctype="multipart/form-data">
                <input type="file" name="Choose file" accept=".gz,.html,.ico,.js,.css">
                <input class="button" type="submit" value="Upload" name="submit">
            </form>
        </div>
    </div>
</body>
</html>
)rawliteral";

#endif