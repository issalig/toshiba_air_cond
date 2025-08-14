#ifndef FILE_MANAGER_HTML_H
#define FILE_MANAGER_HTML_H

const char file_manager_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>File Manager</title>
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
        .tabs {
            display: flex;
            margin: 20px 0 0 0;
            border-bottom: 2px solid #eee;
        }
        .tab {
            flex: 1;
            padding: 12px;
            text-align: center;
            cursor: pointer;
            background: #f8f9fa;
            border: none;
            font-size: 16px;
            transition: background 0.2s;
        }
        .tab.active {
            background: #002F7A;
            color: white;
        }
        .tab:hover:not(.active) {
            background: #e9ecef;
        }
        .tab-content {
            display: none;
            padding: 20px 0;
        }
        .tab-content.active {
            display: block;
        }
        .info-text {
            text-align: left;
            color: #444;
            margin-bottom: 18px;
            font-size: 15px;
        }
        .file-list {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            margin: 15px 0;
            max-height: 200px;
            overflow-y: auto;
            font-family: monospace;
            font-size: 14px;
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
        input[type="text"] {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 8px;
            font-size: 16px;
            margin-bottom: 15px;
            box-sizing: border-box;
        }
        .button {
            text-decoration: none;
            border: none;
            color: white;
            padding: 10px 32px;
            font-size: 16px;
            cursor: pointer;
            box-shadow: 1px 1px 6px #555;
            outline: none;
            margin: 8px;
            display: inline-block;
            border-radius: 8px;
            transition: background 0.2s;
        }
        .btn-primary {
            background-color: #002F7A;
        }
        .btn-primary:hover {
            background: #00878f;
        }
        .btn-delete {
            background-color: #dc3545;
        }
        .btn-delete:hover {
            background: #c82333;
        }
        .button-container {
            text-align: center;
            margin: 15px 0;
        }
        .message {
            padding: 10px;
            margin: 10px 0;
            border-radius: 8px;
            display: none;
        }
        .success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        @media (max-width: 500px) {
            .controller-card { max-width: 98vw; }
        }
    </style>
</head>
<body>
    <div class="controller-card">
        <header>
            <h1>File Manager</h1>
        </header>
        
        <div class="tabs">
            <button class="tab active" onclick="switchTab('upload')">Upload</button>
            <button class="tab" onclick="switchTab('delete')">Delete</button>
        </div>

        <div class="button-container">
            <button class="button btn-primary" onclick="refreshFiles()">Refresh Files</button>
            <button class="button btn-primary" onclick="window.location.href='/'">Home</button>
        </div>
        
        <div class="file-list" id="fileList">
            Loading files...
        </div>

        <!-- Upload Tab -->
        <div id="upload-content" class="tab-content active">
            <p class="info-text">
                Use this section to upload new files to the ESP8266.<br/>
                You can use compressed (deflated) files (<b>.gz</b>) to save space and bandwidth.
            </p>
            
            <form method="post" enctype="multipart/form-data" action="/upload">
                <input type="file" name="Choose file" accept=".gz,.html,.ico,.js,.css">
                <input class="button btn-primary" type="submit" value="Upload File" name="submit">
            </form>
        </div>

        <!-- Delete Tab -->
        <div id="delete-content" class="tab-content">
            <p class="info-text">
                Enter the filename you want to delete. Be careful - this action cannot be undone!<br/>
            </p>
            
            <input type="text" id="filename" placeholder="Enter filename to delete (e.g. status.json)" />
            <div class="button-container">
                <button class="button btn-delete" onclick="deleteFile()">Delete File</button>
            </div>
            <div id="message" class="message"></div>
        </div>
    </div>

    <script>
        function switchTab(tabName) {
            // Hide all tab contents
            document.querySelectorAll('.tab-content').forEach(content => {
                content.classList.remove('active');
            });
            
            // Remove active class from all tabs
            document.querySelectorAll('.tab').forEach(tab => {
                tab.classList.remove('active');
            });
            
            // Show selected tab content
            document.getElementById(tabName + '-content').classList.add('active');
            
            // Add active class to clicked tab
            event.target.classList.add('active');
        }

        function refreshFiles() {
            document.getElementById('fileList').innerHTML = 'Loading...';
            fetch('/api/files')
                .then(r => r.json())
                .then(data => {
                    let html = '';
                    if (data.files && data.files.length > 0) {
                        data.files.forEach(f => {
                            html += f.name + ' (' + formatSize(f.size) + ')<br>';
                        });
                    } else {
                        html = 'No files found';
                    }
                    document.getElementById('fileList').innerHTML = html;
                })
                .catch(() => {
                    document.getElementById('fileList').innerHTML = 'Error loading files';
                });
        }

        function deleteFile() {
            const filename = document.getElementById('filename').value.trim();
            if (!filename) {
                showMessage('Please enter a filename', 'error');
                return;
            }
            if (!confirm('Delete "' + filename + '"? This cannot be undone!')) return;

            fetch('/api/delete', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({filename: filename})
            })
            .then(r => r.json())
            .then(data => {
                if (data.success) {
                    showMessage('File deleted successfully', 'success');
                    document.getElementById('filename').value = '';
                    refreshFiles();
                } else {
                    showMessage('Error: ' + (data.error || 'Unknown error'), 'error');
                }
            })
            .catch(e => {
                showMessage('Network error: ' + e.message, 'error');
            });
        }

        function showMessage(text, type) {
            const msg = document.getElementById('message');
            msg.className = 'message ' + type;
            msg.innerHTML = text;
            msg.style.display = 'block';
            setTimeout(() => msg.style.display = 'none', 5000);
        }

        function formatSize(bytes) {
            if (bytes < 1024) return bytes + ' B';
            if (bytes < 1024*1024) return Math.round(bytes/1024) + ' KB';
            return Math.round(bytes/1024/1024) + ' MB';
        }

        window.onload = refreshFiles;
    </script>
</body>
</html>
)rawliteral";

#endif
