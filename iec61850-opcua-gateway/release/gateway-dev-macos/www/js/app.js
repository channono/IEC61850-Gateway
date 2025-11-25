document.addEventListener('DOMContentLoaded', () => {
    // Navigation Logic
    const navLinks = document.querySelectorAll('.nav-link');
    const views = document.querySelectorAll('.view');
    const pageTitle = document.getElementById('page-title');

    navLinks.forEach(link => {
        link.addEventListener('click', (e) => {
            e.preventDefault();

            // Update Active Link
            navLinks.forEach(l => l.classList.remove('active'));
            link.classList.add('active');

            // Show Target View
            const targetId = `view-${link.dataset.page}`;
            views.forEach(view => view.classList.remove('active'));
            document.getElementById(targetId).classList.add('active');

            // Update Title
            pageTitle.textContent = link.querySelector('span').textContent;
        });
    });

    // System Form
    const systemForm = document.getElementById('system-form');
    if (systemForm) {
        systemForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const formData = new FormData(systemForm);
            const settings = Object.fromEntries(formData.entries());
            console.log('System settings:', settings);

            // Save language preference
            if (settings.language) {
                localStorage.setItem('gateway_language', settings.language);
            }

            alert('System settings saved!');
        });
    }

    // Language Switcher
    const languageSelect = document.getElementById('language-select');
    if (languageSelect) {
        // Load saved language
        languageSelect.value = i18n.currentLang;

        // Language change event (live update)
        languageSelect.addEventListener('change', (e) => {
            const selectedLang = e.target.value;
            i18n.setLanguage(selectedLang);

            // Show notification
            const langNames = {
                'en': 'English',
                'zh': '简体中文',
                'ja': '日本語'
            };
            console.log('Language changed to:', langNames[selectedLang]);
        });
    }

    // API Form
    const apiForm = document.getElementById('api-form');
    if (apiForm) {
        apiForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const formData = new FormData(apiForm);
            const settings = Object.fromEntries(formData.entries());
            settings.apiEnable = apiForm.querySelector('[name="apiEnable"]').checked;
            settings.wsEnable = apiForm.querySelector('[name="wsEnable"]').checked;
            console.log('API settings:', settings);
            alert('API settings saved! (Port changes require restart)');
        });
    }

    // Database Form
    const databaseForm = document.getElementById('database-form');
    if (databaseForm) {
        databaseForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const formData = new FormData(databaseForm);
            const settings = Object.fromEntries(formData.entries());
            console.log('Database settings:', settings);
            alert('Database settings saved!');
        });
    }

    // Database Type Toggle
    const storageTypeSelect = document.getElementById('storageTypeSelect');
    const influxFields = document.getElementById('influxdb-fields');
    const timescaleFields = document.getElementById('timescaledb-fields');

    if (storageTypeSelect) {
        storageTypeSelect.addEventListener('change', (e) => {
            if (e.target.value === 'influxdb') {
                influxFields.style.display = 'block';
                timescaleFields.style.display = 'none';
            } else {
                influxFields.style.display = 'none';
                timescaleFields.style.display = 'block';
            }
        });
    }

    // Initialize GOOSE Monitor when GOOSE view becomes active
    let gooseMonitor = null;
    let mmsTestInitialized = false;

    navLinks.forEach(link => {
        link.addEventListener('click', (e) => {
            const page = link.getAttribute('data-page');
            if (page === 'goose' && !gooseMonitor) {
                // Wait a bit for the view to be visible
                setTimeout(() => {
                    const container = document.getElementById('goose-topology-container');
                    if (container && !gooseMonitor) {
                        gooseMonitor = new GOOSEMonitor('goose-topology-container');
                        console.log('GOOSE Monitor initialized');
                    }
                }, 100);
            } else if (page === 'mms' && !mmsTestInitialized) {
                // Initialize MMS Test page
                setTimeout(() => {
                    if (typeof window.initMMSTest === 'function') {
                        window.initMMSTest();
                        mmsTestInitialized = true;
                        console.log('MMS Test initialized');
                    }
                }, 100);
            }
        });
    });

    // Listen for topology loaded event
    window.addEventListener('topology-loaded', (e) => {
        const badge = document.getElementById('topology-type-badge');
        if (badge) {
            const type = e.detail.type;
            const nodeCount = e.detail.nodeCount;
            badge.textContent = `${type} (${nodeCount} nodes)`;
            badge.className = 'badge ' + (type === 'HSR' ? 'badge-info' : 'badge-success');
        }
    });

    // Modal Logic
    window.openModal = (modalId) => {
        document.getElementById(modalId).classList.add('active');
    };

    document.querySelectorAll('.close-modal').forEach(btn => {
        btn.addEventListener('click', (e) => {
            e.target.closest('.modal').classList.remove('active');
        });
    });

    // Configuration Management
    const scdUpload = document.getElementById('scd-upload');
    const yamlUpload = document.getElementById('yaml-upload');
    const refreshScdListBtn = document.getElementById('refresh-scd-list');
    const downloadScdBtn = document.getElementById('download-scd');
    const downloadYamlBtn = document.getElementById('download-yaml');
    const applyConfigBtn = document.getElementById('apply-config');

    // File upload handlers
    if (scdUpload) {
        scdUpload.addEventListener('change', async (e) => {
            const file = e.target.files[0];
            if (!file) return;

            await uploadFile(file, 'scd');
            scdUpload.value = ''; // Reset input
        });
    }

    if (yamlUpload) {
        yamlUpload.addEventListener('change', async (e) => {
            const file = e.target.files[0];
            if (!file) return;

            await uploadFile(file, 'yaml');
            yamlUpload.value = ''; // Reset input
        });
    }

    // Refresh SCD file list
    if (refreshScdListBtn) {
        refreshScdListBtn.addEventListener('click', async () => {
            await loadSCDFileList();
        });
    }

    // Download buttons
    if (downloadScdBtn) {
        downloadScdBtn.addEventListener('click', () => {
            downloadFile('/api/v1/config/scd/download', 'station.scd');
        });
    }

    if (downloadYamlBtn) {
        downloadYamlBtn.addEventListener('click', () => {
            downloadFile('/api/v1/config/gateway/download', 'gateway.yaml');
        });
    }

    // Apply configuration
    if (applyConfigBtn) {
        applyConfigBtn.addEventListener('click', async () => {
            const selectedScd = document.querySelector('input[name="active-scd"]:checked');
            if (!selectedScd) {
                showNotification('Please select an SCD file', 'warning');
                return;
            }

            if (!confirm(`Apply configuration "${selectedScd.value}" and reload topology?`)) {
                return;
            }

            try {
                const response = await fetch('/api/v1/config/scd/activate', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ filename: selectedScd.value })
                });

                const result = await response.json();

                if (result.success) {
                    showNotification('Configuration applied successfully!', 'success');

                    // Reload topology if on GOOSE page
                    if (gooseMonitor) {
                        await gooseMonitor.loadTopologyFromAPI();
                    }
                } else {
                    showNotification('Failed to apply configuration: ' + result.message, 'error');
                }
            } catch (error) {
                console.error('Apply config error:', error);
                showNotification('Failed to communicate with server', 'error');
            }
        });
    }

    // Load SCD file list on page load
    loadSCDFileList();

    // Mock Data Loading (Replace with Fetch API later)
    loadDashboardStats();
    loadIEDs();
    initCharts();
});

function loadDashboardStats() {
    // Simulate API call
    setTimeout(() => {
        document.getElementById('stat-ied-count').textContent = '12';
        document.getElementById('stat-goose-rate').textContent = '450';
        document.getElementById('stat-opcua-clients').textContent = '3';
        document.getElementById('stat-cpu').textContent = '24%';
    }, 500);
}

function loadIEDs() {
    const tbody = document.getElementById('ied-table-body');
    const mockIEDs = [
        { name: 'IED_Feeder_01', ip: '192.168.1.101', status: 'Connected' },
        { name: 'IED_Trafo_01', ip: '192.168.1.102', status: 'Connected' },
        { name: 'IED_Breaker_01', ip: '192.168.1.103', status: 'Disconnected' }
    ];

    tbody.innerHTML = mockIEDs.map(ied => `
        <tr>
            <td>${ied.name}</td>
            <td>${ied.ip}</td>
            <td><span class="status-dot ${ied.status === 'Connected' ? 'online' : 'offline'}" 
                style="background-color: ${ied.status === 'Connected' ? 'var(--success)' : 'var(--danger)'}; display: inline-block;"></span> ${ied.status}</td>
            <td>
                <button class="btn btn-secondary btn-sm"><i class="fa-solid fa-pen"></i></button>
                <button class="btn btn-secondary btn-sm"><i class="fa-solid fa-trash"></i></button>
            </td>
        </tr>
    `).join('');
}

function initCharts() {
    const ctx = document.getElementById('trafficChart');
    if (ctx) {
        new Chart(ctx, {
            type: 'line',
            data: {
                labels: ['10:00', '10:05', '10:10', '10:15', '10:20', '10:25'],
                datasets: [{
                    label: 'GOOSE Traffic (msg/s)',
                    data: [300, 450, 420, 500, 480, 450],
                    borderColor: '#667eea',
                    tension: 0.4,
                    fill: true,
                    backgroundColor: 'rgba(102, 126, 234, 0.1)'
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    legend: { display: false }
                },
                scales: {
                    y: { grid: { color: 'rgba(255,255,255,0.05)' } },
                    x: { grid: { display: false } }
                }
            }
        });
    }
}

// Helper: Upload file with progress
async function uploadFile(file, type) {
    const formData = new FormData();
    formData.append('file', file);

    const progressContainer = document.getElementById('upload-progress');
    const progressFill = document.getElementById('progress-fill');
    const progressText = document.getElementById('progress-text');

    progressContainer.style.display = 'block';
    progressFill.style.width = '0%';
    progressText.textContent = 'Uploading... 0%';

    try {
        const endpoint = type === 'scd' ? '/api/v1/config/scd' : '/api/v1/config/gateway';

        const xhr = new XMLHttpRequest();

        xhr.upload.addEventListener('progress', (e) => {
            if (e.lengthComputable) {
                const percent = Math.round((e.loaded / e.total) * 100);
                progressFill.style.width = `${percent}%`;
                progressText.textContent = `Uploading... ${percent}%`;
            }
        });

        xhr.addEventListener('load', async () => {
            if (xhr.status === 200) {
                const result = JSON.parse(xhr.responseText);
                progressText.textContent = 'Upload complete!';
                showNotification(`File uploaded: ${file.name}`, 'success');

                // Refresh file list if SCD
                if (type === 'scd') {
                    await loadSCDFileList();
                }

                setTimeout(() => {
                    progressContainer.style.display = 'none';
                }, 2000);
            } else {
                progressText.textContent = 'Upload failed!';
                showNotification('Upload failed: ' + xhr.statusText, 'error');
            }
        });

        xhr.addEventListener('error', () => {
            progressText.textContent = 'Upload error!';
            showNotification('Network error during upload', 'error');
        });

        xhr.open('POST', endpoint);
        xhr.send(formData);

    } catch (error) {
        console.error('Upload error:', error);
        showNotification('Upload failed: ' + error.message, 'error');
        progressContainer.style.display = 'none';
    }
}

// Helper: Download file
function downloadFile(url, filename) {
    fetch(url)
        .then(response => {
            if (!response.ok) throw new Error('Download failed');
            return response.blob();
        })
        .then(blob => {
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = filename;
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);
            showNotification(`Downloaded: ${filename}`, 'success');
        })
        .catch(error => {
            console.error('Download error:', error);
            showNotification('Download failed', 'error');
        });
}

// Helper: Load SCD file list
async function loadSCDFileList() {
    try {
        const response = await fetch('/api/v1/config/scd/list');
        const files = await response.json();

        const fileList = document.getElementById('scd-file-list');
        if (!fileList) return;

        fileList.innerHTML = files.map((file, index) => `
            <div class="file-item">
                <input type="radio" name="active-scd" id="scd-${index}" value="${file.filename}" 
                    ${file.active ? 'checked' : ''}>
                <label for="scd-${index}">
                    <i class="fa-solid fa-file"></i>
                    <span class="file-name">${file.filename}</span>
                    ${file.active ? '<span class="badge badge-success">Active</span>' : ''}
                    <span class="text-muted" style="font-size: 0.75rem; margin-left: auto;">
                        ${formatFileSize(file.size)}
                    </span>
                </label>
            </div>
        `).join('');

    } catch (error) {
        console.error('Failed to load SCD file list:', error);
        // Fallback to default
        const fileList = document.getElementById('scd-file-list');
        if (fileList) {
            fileList.innerHTML = `
                <div class="file-item">
                    <input type="radio" name="active-scd" id="scd-default" value="station.scd" checked>
                    <label for="scd-default">
                        <i class="fa-solid fa-file"></i>
                        <span class="file-name">station.scd</span>
                        <span class="badge badge-success">Active</span>
                    </label>
                </div>
            `;
        }
    }
}

// Helper: Show notification
function showNotification(message, type = 'info') {
    // Simple alert for now - can be replaced with toast notifications later
    const icon = {
        'success': '✅',
        'error': '❌',
        'warning': '⚠️',
        'info': 'ℹ️'
    };

    console.log(`${icon[type]} ${message}`);
    // TODO: Implement toast notification UI
    alert(`${icon[type]} ${message}`);
}

// Helper: Format file size
function formatFileSize(bytes) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return `${Math.round(bytes / Math.pow(k, i) * 100) / 100} ${sizes[i]}`;
}
