// MMS Test Page JavaScript
// Handles IED connection, data browsing, reading, writing, and report subscriptions

let mmsState = {
    connected: false,
    currentIed: null,
    selectedRef: null,
    reportSubscriptionId: null
};

// Initialize MMS Test page
function initMMSTest() {
    console.log('Initializing MMS Test page');

    // Load IED list
    loadIEDList();

    // Export buttons
    const exportIcdBtn = document.getElementById('mms-export-icd');
    if (exportIcdBtn) exportIcdBtn.addEventListener('click', () => exportSCL('icd'));

    const exportCidBtn = document.getElementById('mms-export-cid');
    if (exportCidBtn) exportCidBtn.addEventListener('click', () => exportSCL('cid'));

    // Initial state
    updateUI();

    // Event listeners
    document.getElementById('mms-connect-btn').addEventListener('click', connectToIED);
    document.getElementById('mms-disconnect-btn').addEventListener('click', disconnectFromIED);
    document.getElementById('mms-read-btn').addEventListener('click', readDataPoint);
    document.getElementById('mms-write-btn').addEventListener('click', writeControl);
    document.getElementById('mms-browse-btn').addEventListener('click', browseDataModel);
    document.getElementById('mms-subscribe-btn').addEventListener('click', subscribeToReport);
    document.getElementById('mms-unsubscribe-btn').addEventListener('click', unsubscribeFromReport);

    // Control type selector
    document.getElementById('mms-write-type').addEventListener('change', function () {
        const type = this.value;
        const boolGroup = document.getElementById('mms-write-boolean-group');
        const valueGroup = document.getElementById('mms-write-value-group');

        if (type === 'boolean') {
            boolGroup.style.display = 'block';
            valueGroup.style.display = 'none';
        } else {
            boolGroup.style.display = 'none';
            valueGroup.style.display = 'block';
        }
    });

    // Boolean control buttons
    const onBtn = document.getElementById('mms-write-on');
    const offBtn = document.getElementById('mms-write-off');

    if (onBtn && offBtn) {
        onBtn.addEventListener('click', function () {
            document.getElementById('mms-write-value').value = 'true';
            // Highlight ON button
            onBtn.style.backgroundColor = '#28a745';
            onBtn.style.color = 'white';
            onBtn.style.fontWeight = 'bold';
            onBtn.style.border = '2px solid #28a745';
            // Unhighlight OFF button
            offBtn.style.backgroundColor = '';
            offBtn.style.color = '';
            offBtn.style.fontWeight = '';
            offBtn.style.border = '';
        });

        offBtn.addEventListener('click', function () {
            document.getElementById('mms-write-value').value = 'false';
            // Highlight OFF button
            offBtn.style.backgroundColor = '#dc3545';
            offBtn.style.color = 'white';
            offBtn.style.fontWeight = 'bold';
            offBtn.style.border = '2px solid #dc3545';
            // Unhighlight ON button
            onBtn.style.backgroundColor = '';
            onBtn.style.color = '';
            onBtn.style.fontWeight = '';
            onBtn.style.border = '';
        });
    }
}

function updateUI() {
    const connectBtn = document.getElementById('mms-connect-btn');
    const disconnectBtn = document.getElementById('mms-disconnect-btn');
    const exportGroup = document.getElementById('mms-export-group');

    if (mmsState.connected) {
        if (connectBtn) connectBtn.style.display = 'none';
        if (disconnectBtn) disconnectBtn.style.display = 'inline-block';
        if (exportGroup) exportGroup.style.display = 'block';
    } else {
        if (connectBtn) connectBtn.style.display = 'inline-block';
        if (disconnectBtn) disconnectBtn.style.display = 'none';
        if (exportGroup) exportGroup.style.display = 'none';
    }
}

async function exportSCL(format) {
    if (!mmsState.connected) {
        alert('Please connect to an IED first');
        return;
    }

    // Open in new tab to trigger download
    const url = `/api/mms/export-scl?ied=${mmsState.currentIed}&format=${format}`;
    window.open(url, '_blank');
}

// Load configured IEDs
async function loadIEDList() {
    try {
        const response = await fetch('/api/config');
        const config = await response.json();

        const select = document.getElementById('mms-ied-select');
        select.innerHTML = '<option value="">Select IED...</option>';

        if (config.ieds && config.ieds.length > 0) {
            config.ieds.forEach(ied => {
                if (ied.enabled) {
                    const option = document.createElement('option');
                    option.value = ied.name;
                    option.textContent = `${ied.name} (${ied.ip}:${ied.port})`;
                    option.dataset.ip = ied.ip;
                    option.dataset.port = ied.port;
                    select.appendChild(option);
                }
            });
        }
    } catch (error) {
        console.error('Failed to load IED list:', error);
        addConnectionLog('Error loading IED list: ' + error.message, 'error');
    }
}

// Connect to selected IED
async function connectToIED() {
    const select = document.getElementById('mms-ied-select');
    const selectedOption = select.options[select.selectedIndex];

    if (!selectedOption || !selectedOption.value) {
        alert('Please select an IED');
        return;
    }

    const iedName = selectedOption.value;
    const ip = selectedOption.dataset.ip;
    const port = selectedOption.dataset.port;

    addConnectionLog(`Connecting to ${iedName} (${ip}:${port})...`);

    try {
        const response = await fetch('/api/mms/connect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ ied: iedName })
        });

        const result = await response.json();

        if (result.success) {
            mmsState.connected = true;
            mmsState.currentIed = iedName;

            document.getElementById('mms-ied-status').textContent = 'Connected';
            document.getElementById('mms-ied-status').classList.add('text-success');
            document.getElementById('mms-ied-name').textContent = iedName;

            // Update button states AND visibility
            const connectBtn = document.getElementById('mms-connect-btn');
            const disconnectBtn = document.getElementById('mms-disconnect-btn');
            connectBtn.disabled = true;
            connectBtn.style.display = 'none';  // Hide Connect button
            disconnectBtn.disabled = false;
            disconnectBtn.style.display = 'inline-block';  // Show Disconnect button

            document.getElementById('mms-ied-select').disabled = true;

            addConnectionLog('✓ Connected successfully', 'success');

            // Auto-browse data model
            browseDataModel();

            // Populate RCBs
            populateRCBs();

            // Update UI (show export buttons)
            updateUI();

            // Start polling for events
            startEventPolling();
        } else {
            addConnectionLog('✗ Connection failed: ' + result.message, 'error');
        }
    } catch (error) {
        console.error('Connection error:', error);
        addConnectionLog('✗ Connection error: ' + error.message, 'error');
    }
}

// Disconnect from IED
async function disconnectFromIED() {
    console.log('[DEBUG] disconnectFromIED called');
    console.log('[DEBUG] Current IED:', mmsState.currentIed);
    try {
        const response = await fetch('/api/mms/disconnect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ ied: mmsState.currentIed })
        });

        console.log('[DEBUG] Response status:', response.status);

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }

        const result = await response.json();
        console.log('[DEBUG] Response result:', result);

        if (!result.success) {
            throw new Error(result.message || 'Disconnect failed');
        }

        // Update state
        mmsState.connected = false;
        mmsState.currentIed = null;

        console.log('[DEBUG] Updating UI elements...');

        // Get button elements and inspect them
        const connectBtn = document.getElementById('mms-connect-btn');
        const disconnectBtn = document.getElementById('mms-disconnect-btn');

        console.log('[DEBUG] Connect button before:', connectBtn, 'disabled:', connectBtn?.disabled);
        console.log('[DEBUG] Disconnect button before:', disconnectBtn, 'disabled:', disconnectBtn?.disabled);

        // Update UI
        document.getElementById('mms-ied-status').textContent = 'Disconnected';
        document.getElementById('mms-ied-status').classList.remove('text-success');
        document.getElementById('mms-ied-name').textContent = '-';

        // Update button states AND visibility
        connectBtn.disabled = false;
        connectBtn.style.display = 'inline-block';  // Show Connect button
        disconnectBtn.disabled = true;
        disconnectBtn.style.display = 'none';  // Hide Disconnect button

        document.getElementById('mms-ied-select').disabled = false;

        console.log('[DEBUG] Connect button after:', connectBtn.disabled, 'display:', connectBtn.style.display);
        console.log('[DEBUG] Disconnect button after:', disconnectBtn.disabled, 'display:', disconnectBtn.style.display);
        console.log('[DEBUG] UI updated successfully');

        stopEventPolling();
        addConnectionLog('Disconnected', 'info');
    } catch (error) {
        console.error('[DEBUG] Disconnect error:', error);
        addConnectionLog('Disconnect error: ' + error.message, 'error');

        // Even on error, update UI to disconnected state since backend might have disconnected
        mmsState.connected = false;
        mmsState.currentIed = null;

        console.log('[DEBUG] Updating UI in error handler...');

        const connectBtn = document.getElementById('mms-connect-btn');
        const disconnectBtn = document.getElementById('mms-disconnect-btn');

        document.getElementById('mms-ied-status').textContent = 'Disconnected';
        document.getElementById('mms-ied-status').classList.remove('text-success');
        document.getElementById('mms-ied-name').textContent = '-';

        // Update button states AND visibility
        connectBtn.disabled = false;
        connectBtn.style.display = 'inline-block';  // Show Connect button
        disconnectBtn.disabled = true;
        disconnectBtn.style.display = 'none';  // Hide Disconnect button

        document.getElementById('mms-ied-select').disabled = false;
        console.log('[DEBUG] UI updated in error handler');
    }
}

// Read data point
async function readDataPoint() {
    if (!mmsState.connected) {
        alert('Please connect to an IED first');
        return;
    }

    const ref = document.getElementById('mms-read-ref').value.trim();
    if (!ref) {
        alert('Please enter a data reference');
        return;
    }

    try {
        let response;
        let result;

        // Use selected FC if available (e.g. DC for PhyNam)
        if (mmsState.selectedFC) {
            console.log('[DEBUG] Reading with selected FC:', mmsState.selectedFC);
            response = await fetch(`/api/mms/read?ied=${mmsState.currentIed}&ref=${encodeURIComponent(ref)}&fc=${mmsState.selectedFC}`);
            result = await response.json();
        } else {
            // Try reading with FC=ST first (for Digital Outputs like SPCSO)
            response = await fetch(`/api/mms/read?ied=${mmsState.currentIed}&ref=${encodeURIComponent(ref)}&fc=ST`);
            result = await response.json();

            // If failed, try with FC=MX (for Analog Inputs)
            if (!result.success || result.type === 'ERROR') {
                response = await fetch(`/api/mms/read?ied=${mmsState.currentIed}&ref=${encodeURIComponent(ref)}&fc=MX`);
                result = await response.json();
            }
        }

        if (result.success && result.type !== 'ERROR') {
            // Parse complex value to extract stVal if it's a COMPLEX type
            let displayValue = result.value || '-';

            if (result.type === 'COMPLEX' && typeof displayValue === 'string') {
                // For SPCSO format: {{0,},0,false,0000000000000,19700101000000.000Z}
                // Extract boolean value (stVal is typically the 3rd field)
                const parts = displayValue.replace(/[{}]/g, '').split(',').map(s => s.trim()).filter(s => s);

                // Find the boolean value (true/false)
                const boolValue = parts.find(p => p === 'true' || p === 'false');
                if (boolValue) {
                    displayValue = boolValue;
                } else if (parts.length >= 3) {
                    // Try to get the 3rd value (stVal position for SPCSO)
                    displayValue = parts[2] || displayValue;
                }
            }

            document.getElementById('mms-read-result').style.display = 'block';
            document.getElementById('mms-read-value').textContent = displayValue;
            document.getElementById('mms-read-type').textContent = result.type || '-';
            document.getElementById('mms-read-quality').textContent = result.quality || 'GOOD';
            document.getElementById('mms-read-timestamp').textContent = result.timestamp || new Date().toISOString();
            document.getElementById('mms-last-read').textContent = new Date().toLocaleTimeString();
        } else {
            alert('Read failed: ' + (result.message || result.value));
        }
    } catch (error) {
        console.error('Read error:', error);
        alert('Read error: ' + error.message);
    }
}

// Read data point and show in right panel
async function readDataPointToRightPanel(node) {
    if (!mmsState.connected) {
        return;
    }

    try {
        const fc = node.fc || 'MX';
        const response = await fetch(`/api/mms/read?ied=${mmsState.currentIed}&ref=${encodeURIComponent(node.reference)}&fc=${fc}`);
        const result = await response.json();

        if (result.success) {
            // Update right panel with compact display
            const rightPanel = document.getElementById('mms-values-container');
            if (rightPanel) {
                const formattedValue = formatMmsValue(result.value, node.type);

                // If this is a STRUCTURE type and we simplified it, update Ref to show the actual nested path
                let displayRef = node.reference;
                if (result.type === 'STRUCTURE' && typeof result.value === 'string' && result.value.startsWith('[')) {
                    try {
                        const jsonArray = JSON.parse(result.value);
                        if (Array.isArray(jsonArray) && jsonArray.length === 3) {
                            // Check if first element is boolean/number (SPS/DPS/INS) - we show .stVal
                            if (typeof jsonArray[0] === 'boolean' || (typeof jsonArray[0] === 'number' && typeof jsonArray[0] !== 'object')) {
                                displayRef = node.reference + '.stVal';
                            }
                            // Check if first element is object (MV) - we show .mag.f
                            else if (typeof jsonArray[0] === 'object' && jsonArray[0] !== null) {
                                displayRef = node.reference + '.mag.f';
                            }
                        }
                    } catch (e) {
                        // Not JSON, use original ref
                    }
                }

                rightPanel.innerHTML = `
                    <div style="color: #667eea; margin-bottom: 12px; font-weight: 600;">${node.name}</div>
                    <div style="font-size: 11px; color: var(--text-muted); line-height: 1.6;">
                        <div style="margin-bottom: 4px;"><strong>Value:</strong> <span style="color: #e0e0e0;">${formattedValue}</span></div>
                        <div style="margin-bottom: 4px;"><strong>Type:</strong> ${result.type || '-'}</div>
                        <div style="margin-bottom: 4px;"><strong>Quality:</strong> <span style="color: ${result.quality === 'GOOD' ? '#10b981' : '#f59e0b'};">${result.quality || 'GOOD'}</span></div>
                        <div style="margin-bottom: 4px;"><strong>Time:</strong> ${result.timestamp || new Date().toISOString()}</div>
                        <div style="margin-top: 8px; padding-top: 8px; border-top: 1px solid rgba(255,255,255,0.05);"><strong>Ref:</strong> ${displayRef}</div>
                    </div>
                `;
            } else {
                console.error('Could not find mms-values-container element');
            }
        }
    } catch (error) {
        console.error('Read error:', error);
    }
}

// Write control
async function writeControl() {
    if (!mmsState.connected) {
        alert('Please connect to an IED first');
        return;
    }

    const ref = document.getElementById('mms-write-ref').value.trim();
    const type = document.getElementById('mms-write-type').value;
    let value;

    if (type === 'boolean') {
        // Convert boolean to string for backend
        const boolValue = document.getElementById('mms-write-value').value === 'true';
        value = boolValue ? 'true' : 'false';
    } else {
        value = document.getElementById('mms-write-value').value;
    }

    if (!ref) {
        alert('Please enter a control reference');
        return;
    }

    try {
        const response = await fetch('/api/mms/write', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                ied: mmsState.currentIed,
                ref: ref,
                value: value,  // Now always a string
                type: type
            })
        });

        const result = await response.json();
        const resultDiv = document.getElementById('mms-write-result');
        resultDiv.style.display = 'block';

        if (result.success) {
            resultDiv.innerHTML = '<div class="alert alert-success">Control sent successfully</div>';
        } else {
            resultDiv.innerHTML = `<div class="alert alert-danger">Control failed: ${result.message}</div>`;
        }

        setTimeout(() => {
            resultDiv.style.display = 'none';
        }, 3000);
    } catch (error) {
        console.error('Write error:', error);
        alert('Write error: ' + error.message);
    }
}

// Browse data model
async function browseDataModel() {
    if (!mmsState.connected) {
        return;
    }

    try {
        const response = await fetch(`/api/mms/browse?ied=${mmsState.currentIed}`);
        const result = await response.json();

        if (result.success && result.nodes) {
            renderDataTree(result.nodes);
            document.getElementById('mms-data-count').textContent = result.nodes.length;
        }
    } catch (error) {
        console.error('Browse error:', error);
    }
}

// Render data tree
function renderDataTree(nodes) {
    const treeDiv = document.getElementById('mms-data-tree');
    treeDiv.innerHTML = '';

    if (!nodes || nodes.length === 0) {
        treeDiv.innerHTML = '<div class="text-muted text-center">No data objects found</div>';
        return;
    }

    nodes.forEach(node => {
        const nodeDiv = document.createElement('div');
        nodeDiv.className = 'tree-node';
        nodeDiv.style.padding = '8px 12px';
        nodeDiv.style.cursor = 'pointer';
        nodeDiv.style.borderRadius = '4px';
        nodeDiv.style.marginBottom = '4px';
        nodeDiv.style.transition = 'background-color 0.2s';
        nodeDiv.dataset.reference = node.reference;

        // Icon based on type
        const icon = node.type === 'Analog Input' ? 'fa-wave-square' : 'fa-toggle-on';
        nodeDiv.innerHTML = `<i class="fa-solid ${icon}" style="margin-right: 8px; color: #667eea;"></i> <strong>${node.name}</strong><br><small style="color: #888;">${node.type}</small>`;

        // Click handler
        nodeDiv.onclick = () => selectDataNode(node, nodeDiv);

        // Hover effect
        nodeDiv.onmouseenter = () => {
            if (!nodeDiv.classList.contains('selected')) {
                nodeDiv.style.backgroundColor = 'rgba(102, 126, 234, 0.1)';
            }
        };
        nodeDiv.onmouseleave = () => {
            if (!nodeDiv.classList.contains('selected')) {
                nodeDiv.style.backgroundColor = 'transparent';
            }
        };

        treeDiv.appendChild(nodeDiv);
    });
}

// Select data node
function selectDataNode(node, element) {
    // Remove previous selection
    document.querySelectorAll('.tree-node').forEach(n => {
        n.classList.remove('selected');
        n.style.backgroundColor = 'transparent';
        n.style.fontWeight = 'normal';
    });

    // Highlight selected
    element.classList.add('selected');
    element.style.backgroundColor = 'rgba(102, 126, 234, 0.2)';
    element.style.fontWeight = 'bold';

    // Determine full reference with suffix
    let fullRef = node.reference;
    let fc = node.fc; // Default to node's FC if available

    // Heuristic to append suffix if not present
    if (!fullRef.endsWith('.stVal') && !fullRef.endsWith('.mag.f') && !fullRef.endsWith('.instMag.f')) {
        // Get the last segment of the reference (Leaf name)
        // Handle both dot and dollar separators if present
        const lastSegment = fullRef.split(/[\/\$]/).pop().split('.').pop();

        console.log('[DEBUG] Suffix Logic - Ref:', fullRef, 'LastSegment:', lastSegment, 'Type:', node.type);

        // Special handling for Name Plates (DPL/LPL) -> FC=DC, No Suffix (Structure)
        if (lastSegment.match(/^(PhyNam|NamPlt)/i)) {
            fc = 'DC';
            console.log('[DEBUG] Detected NamePlate, set FC=DC');
        }
        // Status values (SPS, DPS, INS, ENS) -> .stVal
        // Common names: Ind, Beh, Health, Proxy, Pos, St, Mod, Blk
        // Check if lastSegment CONTAINS these keywords (more permissive)
        else if (lastSegment.match(/(Ind|Beh|Health|Proxy|Pos|St|Mod|Blk|Loc|Op|Cl)/i) ||
            node.type === 'Digital Input' ||
            node.type === 'Status') {
            fullRef += '.stVal';
            fc = 'ST';
            console.log('[DEBUG] Appended .stVal ->', fullRef);
        }
        // Measured values (MV, CMV) -> .mag.f
        // Common names: Amp, Vol, Watt, Hz, PhV, A, V, W, TotW, TotVAr
        // Check if lastSegment CONTAINS these keywords (more permissive)
        else if (lastSegment.match(/(Amp|Vol|Watt|Hz|PhV|A|V|W|TotW|TotVAr|Har|AnIn)/i) ||
            node.type === 'Analog Input') {
            fullRef += '.mag.f';
            fc = 'MX';
            console.log('[DEBUG] Appended .mag.f ->', fullRef);
        }
    }

    // Store reference and FC
    mmsState.selectedRef = fullRef;
    mmsState.selectedFC = fc;

    // Update read input field
    document.getElementById('mms-read-ref').value = fullRef;

    // Update write input field
    document.getElementById('mms-write-ref').value = fullRef;

    // Auto-read the value with the full reference
    // Create a temporary node object with the full reference
    const readNode = { ...node, reference: fullRef };

    // Update FC if we determined a specific one
    if (fc) {
        readNode.fc = fc;
        console.log('[DEBUG] Using FC:', fc);
    } else if (fullRef.endsWith('.stVal')) {
        readNode.fc = 'ST';
    } else if (fullRef.endsWith('.mag.f') || fullRef.endsWith('.instMag.f')) {
        readNode.fc = 'MX';
    }

    readDataPointToRightPanel(readNode);
}

// Subscribe to report
async function subscribeToReport() {
    if (!mmsState.connected) {
        alert('Please connect to an IED first');
        return;
    }

    const rcbSelect = document.getElementById('mms-rcb-select');
    const rcbRef = rcbSelect.value;

    if (!rcbRef) {
        alert('Please select an RCB');
        return;
    }

    try {
        const response = await fetch('/api/mms/subscribe', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                ied: mmsState.currentIed,
                rcb_ref: rcbRef
            })
        });

        const result = await response.json();

        if (result.success) {
            mmsState.reportSubscriptionId = result.sub_id;
            document.getElementById('mms-subscribe-btn').disabled = true;
            document.getElementById('mms-unsubscribe-btn').disabled = false;
            addReportLog(`Subscribed to ${rcbRef}`);
        } else {
            alert('Subscription failed: ' + result.message);
        }
    } catch (error) {
        console.error('Subscribe error:', error);
        alert('Subscribe error: ' + error.message);
    }
}

// Unsubscribe from report
async function unsubscribeFromReport() {
    try {
        const response = await fetch('/api/mms/unsubscribe', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                ied: mmsState.currentIed,
                sub_id: mmsState.reportSubscriptionId
            })
        });

        const result = await response.json();

        mmsState.reportSubscriptionId = null;
        document.getElementById('mms-subscribe-btn').disabled = false;
        document.getElementById('mms-unsubscribe-btn').disabled = true;
        addReportLog('Unsubscribed');
    } catch (error) {
        console.error('Unsubscribe error:', error);
    }
}

// Event Polling
let pollingInterval = null;

function startEventPolling() {
    if (pollingInterval) return;
    pollingInterval = setInterval(pollEvents, 50); // Poll every 50ms for near real-time updates
}

function stopEventPolling() {
    if (pollingInterval) {
        clearInterval(pollingInterval);
        pollingInterval = null;
    }
}

async function pollEvents() {
    if (!mmsState.connected) return;

    try {
        const response = await fetch('/api/mms/events');
        const events = await response.json();

        if (events && events.length > 0) {
            events.forEach(event => {
                if (event.type === 'report') {
                    handleReportEvent(event);
                }
            });
        }
    } catch (error) {
        console.error('Polling error:', error);
    }
}

function handleReportEvent(event) {
    const time = event.timestamp || new Date().toLocaleTimeString();
    let msg = `Report from ${event.rcb}:\n`;

    if (event.values) {
        for (const [key, value] of Object.entries(event.values)) {
            msg += `  [${key}]: ${value}\n`;
        }
    }

    addReportLog(msg);
}

// Populate RCBs
async function populateRCBs() {
    const rcbSelect = document.getElementById('mms-rcb-select');
    rcbSelect.innerHTML = '<option value="">Loading...</option>';

    if (!mmsState.connected) {
        rcbSelect.innerHTML = '<option value="">Select RCB...</option>';
        return;
    }

    try {
        const response = await fetch(`/api/mms/rcbs?ied=${mmsState.currentIed}`);
        const result = await response.json();

        rcbSelect.innerHTML = '<option value="">Select RCB...</option>';

        if (result.success && result.rcbs && result.rcbs.length > 0) {
            result.rcbs.forEach(rcb => {
                const option = document.createElement('option');
                option.value = rcb;
                option.textContent = rcb;
                rcbSelect.appendChild(option);
            });
        } else {
            const option = document.createElement('option');
            option.disabled = true;
            option.textContent = 'No RCBs found';
            rcbSelect.appendChild(option);
        }
    } catch (error) {
        console.error('Failed to load RCBs:', error);
        rcbSelect.innerHTML = '<option value="">Error loading RCBs</option>';
    }
}

// Read data point and show in right panel
async function readDataPointToRightPanel(node) {
    if (!mmsState.connected) {
        return;
    }

    try {
        const fc = node.fc || 'MX';
        const response = await fetch(`/api/mms/read?ied=${mmsState.currentIed}&ref=${encodeURIComponent(node.reference)}&fc=${fc}`);
        const result = await response.json();

        if (result.success) {
            // Update right panel with compact display
            const rightPanel = document.getElementById('mms-values-container');
            if (rightPanel) {
                const formattedValue = formatMmsValue(result.value, node.type);
                const qualityColor = result.quality === 'GOOD' ? '#10b981' : '#f59e0b';

                rightPanel.innerHTML = `
                    <div class="mms-detail-header">${node.name}</div>
                    <div class="mms-detail-grid">
                        <div class="mms-detail-label">Value:</div>
                        <div class="mms-detail-value">${formattedValue}</div>
                        
                        <div class="mms-detail-label">Type:</div>
                        <div class="mms-detail-value">${result.type || '-'}</div>
                        
                        <div class="mms-detail-label">Quality:</div>
                        <div class="mms-detail-value" style="color: ${qualityColor}">${result.quality || 'GOOD'}</div>
                        
                        <div class="mms-detail-label">Time:</div>
                        <div class="mms-detail-value">${result.timestamp || new Date().toISOString()}</div>
                        
                        <div class="mms-detail-label" style="margin-top: 0.5vw;">Ref:</div>
                        <div class="mms-detail-value" style="margin-top: 0.5vw; word-break: break-all;">${node.reference}</div>
                    </div>
                `;
            } else {
                console.error('Could not find mms-values-container element');
            }
        }
    } catch (error) {
        console.error('Read error:', error);
    }
}


// Helper: Add connection log entry
function addConnectionLog(message, type = 'info') {
    const logDiv = document.getElementById('mms-connection-log');
    logDiv.style.display = 'block';

    const entry = document.createElement('div');
    entry.className = `log-entry log-${type}`;
    entry.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;
    logDiv.appendChild(entry);

    // Auto-scroll
    logDiv.scrollTop = logDiv.scrollHeight;
}

// Helper: Add report log entry
function addReportLog(message) {
    const logDiv = document.getElementById('mms-report-log');
    const entry = document.createElement('div');
    entry.style.whiteSpace = 'pre-wrap'; // Preserve newlines
    entry.style.borderBottom = '1px solid #eee';
    entry.style.padding = '4px 0';
    entry.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;

    // Check if user is at the top BEFORE adding new content
    const isAtTop = logDiv.scrollTop === 0;

    // Prepend to show newest at top
    if (logDiv.firstChild) {
        logDiv.insertBefore(entry, logDiv.firstChild);
    } else {
        logDiv.appendChild(entry);
    }

    // Only force scroll to top if user was already at the top
    // This prevents jumping when reading older logs
    if (isAtTop) {
        logDiv.scrollTop = 0;
    }
}

// Helper: Format MMS Value (handle complex structs)
function formatMmsValue(value, type) {
    if (!value) return '-';

    // If not complex (no braces), format based on value type
    if (typeof value !== 'string' || !value.startsWith('{')) {
        // Try to parse as JSON array (backend now sends arrays as JSON)
        if (typeof value === 'string' && value.startsWith('[')) {
            try {
                const jsonArray = JSON.parse(value);
                if (Array.isArray(jsonArray) && jsonArray.length > 0) {
                    // Check if this is a typical SPS/DPS/INS structure [stVal, q, t]
                    // where stVal is a boolean or integer (not an object like MV)
                    if (jsonArray.length === 3 && (typeof jsonArray[0] === 'boolean' || typeof jsonArray[0] === 'number' && typeof jsonArray[0] !== 'object')) {
                        // This is likely SPS/DPS/INS: [stVal, q, t]
                        // Extract and display only stVal
                        return formatSingleValue(jsonArray[0]);
                    }

                    // Check if this is a typical MV structure with mag as nested object
                    if (jsonArray.length === 3 && typeof jsonArray[0] === 'object' && jsonArray[0] !== null) {
                        // This is likely {mag: {f: value}, q: xxx, t: xxx}
                        // Extract mag[0] or mag.f for simplified display if available
                        const magObj = jsonArray[0];
                        if (typeof magObj === 'object') {
                            // Try to extract the float value - could be mag[0] or mag.f
                            const magValue = magObj.f !== undefined ? magObj.f :
                                magObj[0] !== undefined ? magObj[0] :
                                    magObj;

                            // Only simplify if we got a number
                            if (typeof magValue === 'number') {
                                return `<span style="color: #2196F3; font-weight: bold;">${magValue}</span> <span style="color: #999; font-size: 0.9em;">(float)</span>`;
                            }
                        }
                    }

                    console.log('[DEBUG] Formatting Structure:', jsonArray, 'Length:', jsonArray.length);

                    // Special case: very small arrays (e.g. [""] or ["value"])
                    if (jsonArray.length === 1) {
                        const content = jsonArray[0];
                        // If empty, show nothing
                        if (content === "" || content === null || content === undefined) {
                            return '';
                        }
                        // Otherwise show the content directly
                        return formatSingleValue(content);
                    }

                    // Special case: 2 elements
                    if (jsonArray.length === 2) {
                        const allStrings = jsonArray.every(item => typeof item === 'string');
                        if (allStrings) {
                            // Filter out empty strings
                            const nonEmpty = jsonArray.filter(item => item !== "");
                            if (nonEmpty.length === 0) {
                                return ''; // All empty, show nothing
                            }
                            // Show non-empty elements
                            return nonEmpty.join(', ');
                        }
                    }

                    // For arrays with 3+ elements, use grid display
                    let html = '<div class="mms-nested-grid">';
                    let labels = [];

                    if (jsonArray.length === 3) {
                        labels = ['mag', 'q', 't'];  // Typical MV structure
                    } else if (jsonArray.length >= 4 && jsonArray.length <= 6) {
                        // Likely DPL (Device Name Plate) or LPL (Logical Node Name Plate)
                        // Structure: vendor, swRev, d, configRev, (ldNs)
                        // Check if all elements are strings to be sure
                        const allStrings = jsonArray.every(item => typeof item === 'string');
                        console.log('[DEBUG] DPL Check - All Strings:', allStrings);

                        if (allStrings) {
                            labels = ['vendor', 'swRev', 'd', 'configRev', 'ldNs', 'res'].slice(0, jsonArray.length);
                        } else {
                            labels = jsonArray.map((_, i) => `[${i}]`);
                        }
                    } else {
                        labels = jsonArray.map((_, i) => `[${i}]`);
                    }

                    jsonArray.forEach((item, index) => {
                        let label = labels[index];
                        let displayValue = formatSingleValue(item, label);

                        html += `<div class="mms-nested-key">${label}:</div>
                                 <div class="mms-nested-value">${displayValue}</div>`;
                    });

                    html += '</div>';
                    return html;
                }
            } catch (e) {
                // If JSON parsing fails, treat as regular value
            }
        }

        // Format single values
        return formatSingleValue(value);
    }

    // Parse the comma-separated string respecting nested braces
    const items = parseComplexString(value);

    let html = '<div class="mms-nested-grid">';

    // Define field labels based on type
    let labels = [];
    if (type === 'Digital Output') { // SPCSO
        if (items.length === 3) {
            // Simple SPCSO: stVal, q, t
            labels = ['stVal', 'q', 't'];
        } else {
            // Full SPCSO (like SPCSO1): origin, ctlNum, stVal, q, t, ctlModel
            labels = ['origin', 'ctlNum', 'stVal', 'q', 't', 'ctlModel'];
        }
    } else if (type === 'Analog Input') { // AnIn
        // Expected: mag, q, t
        labels = ['mag', 'q', 't'];
    }

    items.forEach((item, index) => {
        let label = labels[index] ? labels[index] : `[${index}]`;

        // Highlight important values
        let displayValue = item;
        if (labels[index] === 'stVal') {
            const color = item.includes('true') ? '#4CAF50' : '#F44336';
            displayValue = `<span style="color: ${color}; font-weight: bold;">${item}</span>`;
        } else if (labels[index] === 'mag') {
            displayValue = `<span style="color: #2196F3; font-weight: bold;">${item}</span>`;
        }

        // Recursive formatting if item is complex
        if (item.startsWith('{')) {
            displayValue = formatMmsValue(item, ''); // Recurse without type context for now
        }

        html += `<div class="mms-nested-key">${label}:</div>
                 <div class="mms-nested-value">${displayValue}</div>`;
    });

    html += '</div>';
    return html;
}

// Helper function to format a single value with type detection
function formatSingleValue(value, label) {
    // Handle arrays (nested structures) - recursively format
    if (Array.isArray(value)) {
        if (value.length === 0) return '<span style="color: #999;">(empty array)</span>';
        if (value.length === 1) return formatSingleValue(value[0]);

        // Multiple elements - format as inline list
        const formatted = value.map((v, i) => {
            const formattedVal = formatSingleValue(v);
            return `<span style="margin-right: 0.5em;">[${i}]: ${formattedVal}</span>`;
        }).join(' ');
        return formatted;
    }

    // Handle string values - check for hex encoding
    if (typeof value === 'string') {
        // Try hex decode if it looks like a hex string (even length, only hex chars, at least 4 chars)
        if (/^[0-9A-Fa-f]+$/.test(value) && value.length >= 4 && value.length % 2 === 0) {
            let decoded = '';
            let isAscii = true;
            for (let i = 0; i < value.length; i += 2) {
                const byte = parseInt(value.substr(i, 2), 16);
                if (byte >= 32 && byte <= 126) {
                    decoded += String.fromCharCode(byte);
                } else {
                    isAscii = false;
                    break;
                }
            }
            if (isAscii && decoded.length > 0) {
                return `<span style="color: #9c27b0;">"${decoded}"</span> <span style="color: #999; font-size: 0.85em;">(hex: ${value})</span>`;
            }
        }
        return `<span style="color: #4CAF50; font-weight: bold;">"${value}"</span>`;
    }

    // Handle objects (like mag: {0: 0.0}) - extract the first numeric value
    if (typeof value === 'object' && value !== null && !Array.isArray(value)) {
        // Try to find a numeric value in the object
        for (let key in value) {
            if (typeof value[key] === 'number') {
                value = value[key];
                break;
            }
        }
        // If still an object, convert to string
        if (typeof value === 'object') {
            value = JSON.stringify(value);
        }
    }

    const strValue = String(value);

    // Boolean values
    if (strValue === 'true' || strValue === 'false') {
        const color = strValue === 'true' ? '#4CAF50' : '#F44336';
        return `<span style="color: ${color}; font-weight: bold;">${strValue}</span>`;
    }

    // Integer/Unsigned/Enum values (pure numbers) - but not quality bitstrings
    if (/^-?\d+$/.test(strValue) && label !== 'q') {
        return `<span style="color: #9C27B0; font-weight: bold;">${strValue}</span> <span style="color: #999; font-size: 0.9em;">(int)</span>`;
    }

    // Quality bitstring (special handling for 'q' field)
    if (label === 'q' && /^\d+$/.test(strValue)) {
        const qualityInt = parseInt(strValue);
        const binary = qualityInt.toString(2).padStart(13, '0');
        return `<span style="color: #607D8B; font-family: monospace;">${binary}</span> <span style="color: #999; font-size: 0.9em;">(quality)</span>`;
    }

    // Float values (numbers with decimal point) - highlight 'mag' field
    if (/^-?\d+\.?\d*$/.test(strValue) && strValue.includes('.')) {
        const color = label === 'mag' ? '#2196F3' : '#00BCD4';
        const weight = label === 'mag' ? 'bold' : 'normal';
        return `<span style="color: ${color}; font-weight: ${weight};">${strValue}</span> <span style="color: #999; font-size: 0.9em;">(float)</span>`;
    }

    // Timestamp values (ISO format or IEC format)
    if (/^\d{4}-\d{2}-\d{2}/.test(strValue) || /\d{14,}\.?\d*Z?$/.test(strValue)) {
        return `<span style="color: #FF9800; font-weight: bold;">🕐 ${strValue}</span> <span style="color: #999; font-size: 0.9em;">(time)</span>`;
    }

    // Hex String (OctetString) - all uppercase hex chars
    if (/^[0-9A-F]{2,}$/.test(strValue)) {
        const formatted = strValue.match(/.{1,2}/g).join(' ');
        return `<span style="color: #00BCD4; font-family: monospace;">${formatted}</span> <span style="color: #999; font-size: 0.9em;">(hex)</span>`;
    }

    // Default string formatting
    return `<span style="color: #4CAF50; font-weight: bold;">${value}</span>`;
}

// Helper: Parse complex string "{a, b, {c, d}, e}" into array ["a", "b", "{c, d}", "e"]
function parseComplexString(str) {
    // Remove outer braces
    if (str.startsWith('{') && str.endsWith('}')) {
        str = str.substring(1, str.length - 1);
    }

    const result = [];
    let current = '';
    let braceCount = 0;

    for (let i = 0; i < str.length; i++) {
        const char = str[i];

        if (char === '{') {
            braceCount++;
            current += char;
        } else if (char === '}') {
            braceCount--;
            current += char;
        } else if (char === ',' && braceCount === 0) {
            result.push(current.trim());
            current = '';
        } else {
            current += char;
        }
    }

    if (current) {
        result.push(current.trim());
    }

    return result;
}

// Export for global access
window.initMMSTest = initMMSTest;
