/**
 * GOOSE Monitor - Native SVG Visualization
 * Professional-grade dual-network GOOSE monitoring
 */

class GOOSEMonitor {
    constructor(containerId) {
        this.container = document.getElementById(containerId);
        this.svg = null;
        this.nodes = new Map();
        this.connections = [];
        this.stats = {
            networkA: { rate: 0, errors: 0, quality: 100 },
            networkB: { rate: 0, errors: 0, quality: 100 }
        };

        this.init();
    }

    async init() {
        this.createSVG();
        this.createDefs();
        await this.loadTopologyFromAPI();
        this.startAnimation();
    }

    createSVG() {
        this.svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
        this.svg.setAttribute('viewBox', '0 0 1200 600');
        this.svg.setAttribute('class', 'goose-topology');
        this.container.appendChild(this.svg);

        // Simple HTML tooltip (non-intrusive)
        this.tooltip = document.createElement('div');
        this.tooltip.style.cssText = `
            position: fixed;
            display: none;
            background: rgba(15, 23, 42, 0.95);
            border: 1px solid #334155;
            padding: 8px 12px;
            border-radius: 6px;
            color: #f8fafc;
            font-size: 12px;
            z-index: 10000;
            pointer-events: none;
            white-space: nowrap;
        `;
        document.body.appendChild(this.tooltip);
    }

    createDefs() {
        const defs = document.createElementNS('http://www.w3.org/2000/svg', 'defs');

        // Arrow marker for Network A
        const markerA = this.createMarker('arrow-a', '#10b981');
        defs.appendChild(markerA);

        // Arrow marker for Network B / HSR Port B
        const markerB = this.createMarker('arrow-b', '#06b6d4');
        defs.appendChild(markerB);

        // Gradient for nodes
        const gradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
        gradient.setAttribute('id', 'node-gradient');
        gradient.setAttribute('x1', '0%');
        gradient.setAttribute('y1', '0%');
        gradient.setAttribute('x2', '0%');
        gradient.setAttribute('y2', '100%');

        const stop1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        stop1.setAttribute('offset', '0%');
        stop1.setAttribute('stop-color', '#667eea');

        const stop2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        stop2.setAttribute('offset', '100%');
        stop2.setAttribute('stop-color', '#764ba2');

        gradient.appendChild(stop1);
        gradient.appendChild(stop2);
        defs.appendChild(gradient);

        // Gateway gradient (Gold/Orange)
        const gatewayGradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
        gatewayGradient.setAttribute('id', 'gateway-gradient');
        gatewayGradient.setAttribute('x1', '0%');
        gatewayGradient.setAttribute('y1', '0%');
        gatewayGradient.setAttribute('x2', '0%');
        gatewayGradient.setAttribute('y2', '100%');
        const gw1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        gw1.setAttribute('offset', '0%');
        gw1.setAttribute('stop-color', '#f59e0b');
        const gw2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        gw2.setAttribute('offset', '100%');
        gw2.setAttribute('stop-color', '#d97706');
        gatewayGradient.appendChild(gw1);
        gatewayGradient.appendChild(gw2);
        defs.appendChild(gatewayGradient);

        // Protection gradient (Red)
        const protectionGradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
        protectionGradient.setAttribute('id', 'protection-gradient');
        protectionGradient.setAttribute('x1', '0%');
        protectionGradient.setAttribute('y1', '0%');
        protectionGradient.setAttribute('x2', '0%');
        protectionGradient.setAttribute('y2', '100%');
        const pr1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        pr1.setAttribute('offset', '0%');
        pr1.setAttribute('stop-color', '#ef4444');
        const pr2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        pr2.setAttribute('offset', '100%');
        pr2.setAttribute('stop-color', '#dc2626');
        protectionGradient.appendChild(pr1);
        protectionGradient.appendChild(pr2);
        defs.appendChild(protectionGradient);

        // Meter gradient (Cyan/Blue)
        const meterGradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
        meterGradient.setAttribute('id', 'meter-gradient');
        meterGradient.setAttribute('x1', '0%');
        meterGradient.setAttribute('y1', '0%');
        meterGradient.setAttribute('x2', '0%');
        meterGradient.setAttribute('y2', '100%');
        const mt1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        mt1.setAttribute('offset', '0%');
        mt1.setAttribute('stop-color', '#06b6d4');
        const mt2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        mt2.setAttribute('offset', '100%');
        mt2.setAttribute('stop-color', '#0891b2');
        meterGradient.appendChild(mt1);
        meterGradient.appendChild(mt2);
        defs.appendChild(meterGradient);

        // Controller gradient (Orange/Amber)
        const controllerGradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
        controllerGradient.setAttribute('id', 'controller-gradient');
        controllerGradient.setAttribute('x1', '0%');
        controllerGradient.setAttribute('y1', '0%');
        controllerGradient.setAttribute('x2', '0%');
        controllerGradient.setAttribute('y2', '100%');
        const ct1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        ct1.setAttribute('offset', '0%');
        ct1.setAttribute('stop-color', '#fb923c');
        const ct2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        ct2.setAttribute('offset', '100%');
        ct2.setAttribute('stop-color', '#f97316');
        controllerGradient.appendChild(ct1);
        controllerGradient.appendChild(ct2);
        defs.appendChild(controllerGradient);

        this.svg.appendChild(defs);
    }

    createMarker(id, color) {
        const marker = document.createElementNS('http://www.w3.org/2000/svg', 'marker');
        marker.setAttribute('id', id);
        marker.setAttribute('markerWidth', '10');
        marker.setAttribute('markerHeight', '10');
        marker.setAttribute('refX', '9');
        marker.setAttribute('refY', '3');
        marker.setAttribute('orient', 'auto');
        marker.setAttribute('markerUnits', 'strokeWidth');

        const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
        path.setAttribute('d', 'M0,0 L0,6 L9,3 z');
        path.setAttribute('fill', color);

        marker.appendChild(path);
        return marker;
    }

    async loadTopologyFromAPI() {
        try {
            // Fetch topology from backend (disable cache to ensure fresh data)
            const response = await fetch('/api/v1/topology', {
                cache: 'no-cache',
                headers: {
                    'Cache-Control': 'no-cache',
                    'Pragma': 'no-cache'
                }
            });

            if (!response.ok) {
                console.warn('Failed to load topology from API, using mock data');
                this.topology = this.getMockTopology();
            } else {
                this.topology = await response.json();
            }

            console.log('Loaded topology:', this.topology);

            // Detect architecture type
            const hasRedBox = this.topology.nodes.some(n => n.type === 'RedBox');
            const displayType = hasRedBox ? 'PRP+HSR' : this.topology.type;

            // Update UI Badge
            const badge = document.getElementById('topology-type-badge');
            if (badge) {
                badge.textContent = displayType;
                const badgeClass = hasRedBox ? 'warning' : (this.topology.type === 'HSR' ? 'success' : 'primary');
                badge.className = `badge badge-${badgeClass}`;
            }

            this.renderTopology();
        } catch (error) {
            console.warn('Error loading topology:', error);
            this.topology = this.getMockTopology();
            this.renderTopology();
        }
    }

    getMockTopology() {
        // Mock data for testing - can be configured via query param
        const params = new URLSearchParams(window.location.search);
        const mockType = params.get('topology') || 'prp';
        const nodeCount = parseInt(params.get('nodes') || '4');

        let nodes = [];
        let connections = [];

        // Generate mock IEDs
        for (let i = 0; i < nodeCount; i++) {
            const iedTypes = ['ProtectionRelay', 'Controller', 'Meter'];
            const type = iedTypes[i % 3];
            nodes.push({
                id: `IED${i + 1}`,
                name: `IED_${i + 1}`,
                type: type,
                ip: `10.0.0.${i + 10}`,
                status: i % 5 === 0 ? 'warning' : 'online'
            });
        }

        // For HSR, add ring connections
        if (mockType === 'hsr') {
            for (let i = 0; i < nodes.length; i++) {
                connections.push({
                    from: nodes[i].id,
                    to: nodes[(i + 1) % nodes.length].id,
                    type: 'GOOSE'
                });
            }
        } else { // For PRP, connections are handled by switches
            // No direct IED-IED connections in mock PRP
        }

        // Add gateway node
        nodes.push({
            id: 'gateway',
            name: 'Gateway',
            ip: '192.168.1.1',
            status: 'online',
            type: 'GATEWAY',
            connection: 'AB' // PRP (Dual Attached)
        });

        // Add Mixed Architecture Examples
        if (mockType === 'prp') {
            // SAN (Single Attached Node) - Network A only
            nodes.push({
                id: 'meter-san-a',
                name: 'Meter (SAN-A)',
                status: 'online',
                type: 'Meter',
                connection: 'A'
            });

            // SAN (Single Attached Node) - Network B only
            nodes.push({
                id: 'meter-san-b',
                name: 'Meter (SAN-B)',
                status: 'online',
                type: 'Meter',
                connection: 'B'
            });

            // RedBox (Redundancy Box)
            nodes.push({
                id: 'redbox-1',
                name: 'HSR Ring RedBox',
                status: 'online',
                type: 'RedBox',
                connection: 'AB'
            });
        }

        return {
            type: mockType.toUpperCase(),
            nodes: nodes,
            connections: connections,
            networks: mockType === 'prp' ? ['eth0', 'eth1'] : ['hsr-ring']
        };
    }

    renderTopology() {
        // Clear existing nodes and connections
        this.nodes.clear();
        this.connections = [];
        this.pathCounter = 0; // Reset ID counter

        // CRITICAL: Completely clear SVG - remove ALL children
        while (this.svg.firstChild) {
            this.svg.removeChild(this.svg.firstChild);
        }

        // Re-create defs (gradients, markers)
        this.createDefs();

        // Dispatch event for other components
        window.dispatchEvent(new CustomEvent('topology-updated', {
            detail: { type: this.topology.type, nodes: this.topology.nodes }
        }));

        // Check if this is a mixed architecture (has RedBox)
        const hasRedBox = this.topology.nodes.some(n => n.type === 'RedBox');

        // Render based on topology type
        if (hasRedBox) {
            this.renderMixedTopology();
        } else if (this.topology.type === 'HSR') {
            this.renderHSRTopology();

            // For HSR, draw connections defined in topology (Ring connections)
            this.topology.connections.forEach(conn => {
                this.addConnection(conn.from, conn.to);
            });
        } else {
            this.renderPRPTopology();
            // For PRP, connections are created within renderPRPTopology (Physical switch connections)
            // We ignore logical connections from backend to avoid direct IED-Gateway lines
        }
    }

    renderPRPTopology() {
        // Parallel Redundancy Protocol - Dual star topology with switches
        const nodes = this.topology.nodes;

        // Find gateway
        const gateway = nodes.find(n => n.type === 'GATEWAY' || n.id === 'gateway');
        const otherNodes = nodes.filter(n => n.type !== 'GATEWAY' && n.id !== 'gateway');
        const iedCount = otherNodes.length;

        // Layout Configuration
        const canvasWidth = 1200;
        const startY = 350; // Starting Y position for IEDs
        const rowHeight = 120; // Vertical gap between rows
        const nodeWidth = 140; // Estimated visual width of node + gap
        const maxCols = Math.floor((canvasWidth - 100) / nodeWidth); // Max items per row (leaving margin)

        // Calculate rows needed
        const rows = Math.ceil(iedCount / maxCols);

        // Adjust SVG height if needed
        const requiredHeight = startY + (rows * rowHeight) + 50;
        if (requiredHeight > 600) {
            this.svg.setAttribute('viewBox', `0 0 ${canvasWidth} ${requiredHeight}`);
            this.svg.style.height = `${requiredHeight}px`;
        } else {
            this.svg.setAttribute('viewBox', `0 0 ${canvasWidth} 600`);
            this.svg.style.height = '600px';
        }

        // 1. Render Gateway (Top Center)
        if (gateway) {
            this.addNode(gateway.id, gateway.name, canvasWidth / 2, 80, gateway.status, gateway.type);
        }

        // 2. Render Switches (Middle, spread out)
        // Switch A (Left)
        this.addNode('switch-a', 'Switch A', canvasWidth * 0.25, 200, 'online', 'Switch');

        // Switch B (Right)
        this.addNode('switch-b', 'Switch B', canvasWidth * 0.75, 200, 'online', 'Switch');

        // Connect switches to gateway
        if (gateway) {
            this.addConnection('switch-a', gateway.id);
            this.addConnection('switch-b', gateway.id);
        }

        // 3. Render IEDs (Adaptive Grid)
        otherNodes.forEach((node, index) => {
            // Calculate row and column
            const row = Math.floor(index / maxCols);
            const col = index % maxCols;

            // Calculate items in this specific row (for centering)
            const isLastRow = row === rows - 1;
            const itemsInThisRow = isLastRow ? (iedCount % maxCols || maxCols) : maxCols;

            // Calculate X position to center the row
            const rowWidth = itemsInThisRow * nodeWidth;
            const startX = (canvasWidth - rowWidth) / 2 + (nodeWidth / 2);

            const x = startX + (col * nodeWidth);
            const y = startY + (row * rowHeight);

            this.addNode(node.id, node.name, x, y, node.status, node.type);

            // Connect to switches based on connection type (Mixed Architecture Support)
            // Default: PRP (Both)
            // 'A': SAN connected to Network A only
            // 'B': SAN connected to Network B only
            const connType = node.connection || 'AB';

            if (connType.includes('A')) {
                this.addConnection(node.id, 'switch-a');
            }
            if (connType.includes('B')) {
                this.addConnection(node.id, 'switch-b');
            }
        });
    }

    renderMixedTopology() {
        // Mixed PRP+HSR Architecture with intelligent layout
        const nodes = this.topology.nodes;

        // Categorize nodes
        const gateway = nodes.find(n => n.type === 'GATEWAY' || n.id === 'gateway' || n.name.includes('Gateway'));
        const redboxes = nodes.filter(n => n.type === 'RedBox');
        const otherIEDs = nodes.filter(n =>
            n.type !== 'GATEWAY' &&
            n.type !== 'RedBox' &&
            n.id !== 'gateway' &&
            !n.name.includes('Gateway')
        );

        console.log('Mixed topology - Gateway:', gateway?.name, 'RedBoxes:', redboxes.length, 'IEDs:', otherIEDs.length);

        if (redboxes.length === 0) {
            // Fallback to PRP if no RedBoxes
            this.renderPRPTopology();
            return;
        }

        // === LAYOUT CONFIGURATION ===
        // Adaptive canvas dimensions using viewport width
        let canvasWidth = Math.floor(window.innerWidth * 0.98); // 98% of viewport width
        let canvasHeight = 800; // Base height, will be adjusted by viewBox

        // Update SVG container size
        this.svg.setAttribute('width', `${canvasWidth}`);

        // Layer Y positions - Optimized for better vertical space usage
        const layerY = {
            gateway: 50,
            switches: 160,
            redboxes: 280,
            ieds: 400
        };

        // === L0: GATEWAY (Top Center) ===
        if (gateway) {
            this.addNode(gateway.id, gateway.name, canvasWidth / 2, layerY.gateway,
                gateway.status || 'online', gateway.type);
        }

        // === L1: VIRTUAL SWITCHES (Auto-created) ===
        const switchA = { id: 'switch-a', name: 'Switch A', type: 'SWITCH', virtual: true };
        const switchB = { id: 'switch-b', name: 'Switch B', type: 'SWITCH', virtual: true };

        const switchXOffset = canvasWidth * 0.3;
        this.addNode(switchA.id, switchA.name, canvasWidth / 2 - switchXOffset, layerY.switches, 'online', 'SWITCH');
        this.addNode(switchB.id, switchB.name, canvasWidth / 2 + switchXOffset, layerY.switches, 'online', 'SWITCH');

        // Connect Gateway to Switches
        if (gateway) {
            const gwNode = this.nodes.get(gateway.id);
            const swANode = this.nodes.get(switchA.id);
            const swBNode = this.nodes.get(switchB.id);

            if (gwNode && swANode && swBNode) {
                const g1 = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                g1.setAttribute('class', 'goose-connection');

                const pathToA = this.createConnectionPath(gwNode, swANode, 'network-a', '#10b981', 'arrow-a', false, 0);
                g1.appendChild(pathToA);
                g1.appendChild(this.createMessageDot(pathToA, '#10b981', '2s'));
                this.svg.appendChild(g1);

                const g2 = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                g2.setAttribute('class', 'goose-connection');

                const pathToB = this.createConnectionPath(gwNode, swBNode, 'network-b', '#3b82f6', 'arrow-b', true, 0);
                g2.appendChild(pathToB);
                g2.appendChild(this.createMessageDot(pathToB, '#3b82f6', '2.2s'));
                this.svg.appendChild(g2);
            }
        }

        // === L2: REDBOXES (Aligned with Switches) ===
        // Position RedBoxes to align with their respective Switches
        // Switch A at (0.5 - 0.3) = 0.2, Switch B at (0.5 + 0.3) = 0.8
        const redboxPositions = redboxes.length === 2
            ? [canvasWidth / 2 - switchXOffset, canvasWidth / 2 + switchXOffset]
            : redboxes.map((_, idx) => (canvasWidth / (redboxes.length + 1)) * (idx + 1));

        redboxes.forEach((redbox, idx) => {
            const x = redboxPositions[idx];
            this.addNode(redbox.id, redbox.name, x, layerY.redboxes, redbox.status || 'online', redbox.type);

            // Connect each RedBox to BOTH switches (3-port architecture)
            const rbNode = this.nodes.get(redbox.id);
            const swANode = this.nodes.get(switchA.id);
            const swBNode = this.nodes.get(switchB.id);

            if (rbNode && swANode && swBNode) {
                // Port A (Green, to Switch A)
                const gA = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                gA.setAttribute('class', 'goose-connection');
                const pathA = this.createConnectionPath(rbNode, swANode, 'network-a', '#10b981', 'arrow-a', false, -10);
                gA.appendChild(pathA);
                gA.appendChild(this.createMessageDot(pathA, '#10b981', '2s'));
                this.svg.appendChild(gA);

                // Port B (Blue, to Switch B)
                const gB = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                gB.setAttribute('class', 'goose-connection');
                const pathB = this.createConnectionPath(rbNode, swBNode, 'network-b', '#3b82f6', 'arrow-b', true, 10);
                gB.appendChild(pathB);
                gB.appendChild(this.createMessageDot(pathB, '#3b82f6', '2.2s'));
                this.svg.appendChild(gB);
            }
        });

        // === L3: GROUP IEDs BY REDBOX ===
        const iedGroups = {};
        redboxes.forEach(rb => {
            iedGroups[rb.id] = [];
        });

        console.log(`🔍 Starting IED Grouping for ${otherIEDs.length} IEDs into ${redboxes.length} RedBoxes`);

        // Smart grouping based on naming convention
        otherIEDs.forEach(ied => {
            let assigned = false;
            const iedNameUpper = (ied.name || ied.id || '').toUpperCase();

            // Try to match by name pattern (RingA_IED01 -> RedBox_A)
            for (const redbox of redboxes) {
                const rbNameUpper = (redbox.name || redbox.id || '').toUpperCase();

                // Check for "A" vs "B" distinction
                const isRingA = iedNameUpper.includes('RINGA') || iedNameUpper.includes('RING_A');
                const isRingB = iedNameUpper.includes('RINGB') || iedNameUpper.includes('RING_B');

                const isRedBoxA = rbNameUpper.includes('_A') || (rbNameUpper.includes('A') && !rbNameUpper.includes('B'));
                const isRedBoxB = rbNameUpper.includes('_B') || (rbNameUpper.includes('B') && !rbNameUpper.includes('A'));

                if ((isRingA && isRedBoxA) || (isRingB && isRedBoxB)) {
                    iedGroups[redbox.id].push(ied);
                    assigned = true;
                    break;
                }
            }

            // Fallback: distribute evenly
            if (!assigned) {
                const minGroupId = Object.keys(iedGroups).reduce((minId, currentId) => {
                    return (iedGroups[currentId].length < iedGroups[minId].length) ? currentId : minId;
                }, Object.keys(iedGroups)[0]);

                iedGroups[minGroupId].push(ied);
                console.log(`  ⚠️ Fallback: Assigned ${ied.name} to RedBox ID ${minGroupId}`);
            }
        });

        // Log final distribution
        redboxes.forEach(rb => {
            console.log(`📊 ${rb.name}: ${iedGroups[rb.id]?.length || 0} IEDs`);
        });

        // === RENDER IED GROUPS (Grid layout per RedBox) ===
        // Calculate layout constraints
        const edgeMargin = 10; // Minimal edge margin
        const availableHalfWidth = (canvasWidth / 2) - (edgeMargin * 2);

        let maxRowsNeeded = 0;
        let maxRowSpacing = 0; // Track the largest rowSpacing for viewBox calculation

        redboxes.forEach((redbox, rbIdx) => {
            const ieds = iedGroups[redbox.id] || [];
            if (ieds.length === 0) {
                console.warn(`⚠️ No IEDs found for RedBox ${redbox.name}`);
                return;
            }

            console.log(`✅ Rendering ${ieds.length} IEDs for ${redbox.name} (index: ${rbIdx})`);

            // === DYNAMIC LAYOUT OPTIMIZER (FAVORS 20+ COLUMNS) ===
            // Goal: Dynamically optimize to naturally prefer 20+ column layouts

            // Base dimensions
            const baseW = 140;
            const baseH = 80;
            const baseGapX = 15;
            const baseGapY = 30;
            const baseUnitW = baseW + baseGapX;

            let bestLayout = {
                cols: 10,
                scale: 0.5,
                rows: Math.ceil(ieds.length / 10),
                score: 0
            };

            // Layout constraints - favor wider layouts
            const minCols = 18; // Start from 18 columns minimum (forces 20-col consideration for 30-device rings)
            const maxColsLimit = Math.max(minCols, Math.min(ieds.length, 30));
            const MIN_ACCEPTABLE_SCALE = 0.35; // Allow smaller devices for more columns

            for (let c = minCols; c <= maxColsLimit; c++) {
                // Calculate scale for this column count
                let s = availableHalfWidth / (c * baseUnitW);

                // Skip if too small
                if (s < MIN_ACCEPTABLE_SCALE) {
                    continue;
                }

                // Calculate rows
                const r = Math.ceil(ieds.length / c);
                const isPerfectDivision = (ieds.length % c === 0); // No partial rows

                // === INTELLIGENT SCORING ALGORITHM ===
                let score = 0;

                // 1. Device size (low weight - not the priority)
                if (s >= 0.7) {
                    score += s * 200;
                } else if (s >= 0.5) {
                    score += s * 150;
                } else {
                    score += s * 100;
                }

                // 2. **SMART: Column count with PEAK at 20 (not linear)**
                // Instead of "more columns = always better", create a peak around 20
                const distanceFrom20 = Math.abs(c - 20);

                // Base column score (diminishing returns beyond 20)
                if (c <= 20) {
                    score += c * 500; // Strong growth up to 20
                } else {
                    score += 20 * 500 + (c - 20) * 50; // Slow growth beyond 20
                }

                // 3. **SMART BONUS: Perfect division (no partial rows)**
                // 60 ÷ 20 = 3.0 ✓  vs  60 ÷ 30 = 2.0 ✓ (both perfect, but 20 is better)
                if (isPerfectDivision) {
                    score += 1000; // Major bonus for clean grid
                }

                // 4. **SMART BONUS: Proximity to 20 columns (golden standard)**
                // 20 is the "golden standard", closer = better
                if (distanceFrom20 === 0) {
                    score += 3000; // Exactly 20 columns - HUGE bonus!
                } else if (distanceFrom20 <= 2) {
                    score += 1500; // 18-22 columns - excellent
                } else if (distanceFrom20 <= 5) {
                    score += 500; // 15-25 columns - acceptable
                } else if (distanceFrom20 > 10) {
                    score -= 1000; // Far from 20 (e.g., 30+) - penalty
                }

                // 5. Aspect ratio preference (prefer 3-4 rows over 2 rows)
                const aspectRatio = r / c;
                if (r === 3) {
                    score += 800; // Exactly 3 rows - ideal!
                } else if (r === 2) {
                    score += 200; // 2 rows - acceptable but not ideal
                } else if (r <= 5) {
                    score += 400; // 4-5 rows - good
                }

                // 6. Penalty for too many rows
                if (r > 5) {
                    score -= (r - 5) * 200; // Penalty grows with more rows
                }

                if (score > bestLayout.score) {
                    bestLayout = { cols: c, scale: s, rows: r, score: score };
                }
            }

            // Apply best layout
            const maxCols = bestLayout.cols;
            const scaleFactor = bestLayout.scale;
            const actualRows = bestLayout.rows;

            console.log(`  Optimized Layout: ${maxCols} cols × ${actualRows} rows, Scale: ${scaleFactor.toFixed(2)}, Score: ${bestLayout.score.toFixed(0)}`);

            maxRowsNeeded = Math.max(maxRowsNeeded, actualRows);

            // Recalculate dimensions with final scale
            const deviceWidth = baseW * scaleFactor;
            const deviceHeight = baseH * scaleFactor;
            const horizontalGap = baseGapX * scaleFactor;
            const verticalGap = baseGapY * scaleFactor;
            const colSpacing = deviceWidth + horizontalGap;
            const rowSpacing = deviceHeight + verticalGap;

            // Track the largest row spacing for viewBox height
            maxRowSpacing = Math.max(maxRowSpacing, rowSpacing);

            // Calculate total width of the group
            const groupWidth = maxCols * colSpacing - horizontalGap; // Remove last gap

            // Reduce centering - push groups towards edges for better separation
            const extraSpace = availableHalfWidth - groupWidth;
            const centerOffset = Math.max(0, extraSpace * 0.15); // Only 15% of extra space (was 50%)

            // Calculate starting X position
            let groupStartX;

            if (rbIdx === 0) {
                // RingA (Left): Margin + Center Offset
                groupStartX = edgeMargin + centerOffset + (deviceWidth / 2);
            } else {
                // RingB (Right): Center Line + Margin + Center Offset
                groupStartX = (canvasWidth / 2) + edgeMargin + centerOffset + (deviceWidth / 2);
            }

            console.log(`  Layout Optimized: ${maxCols} cols, Scale: ${scaleFactor.toFixed(2)}, Device: ${deviceWidth.toFixed(0)}x${deviceHeight.toFixed(0)}`);

            // Store IED nodes IN GRID ORDER for HSR ring connection
            const iedNodesInRing = [];

            ieds.forEach((ied, iedIdx) => {
                const row = Math.floor(iedIdx / maxCols);
                let col = iedIdx % maxCols;

                // Snake Layout: Even rows (0, 2...) L->R, Odd rows (1, 3...) R->L
                if (row % 2 === 1) {
                    col = (maxCols - 1) - col;
                }

                // Calculate X based on start + col * (width + gap)
                const x = groupStartX + (col * colSpacing);

                // Calculate Y based on layer start + row * (height + gap)
                const y = layerY.ieds + (row * rowSpacing);

                this.addNode(ied.id, ied.name, x, y, ied.status || 'online', ied.type || 'IED', scaleFactor);
                iedNodesInRing.push(this.nodes.get(ied.id));
            });

            // === HSR RING CONNECTIONS (Grid Order) ===
            // Connect IEDs in GRID layout order: left-to-right, top-to-bottom

            if (iedNodesInRing.length > 1) {
                const rbNode = this.nodes.get(redbox.id);

                // Connect consecutive IEDs in grid order
                for (let i = 0; i < iedNodesInRing.length - 1; i++) {
                    const currentIED = iedNodesInRing[i];
                    const nextIED = iedNodesInRing[i + 1];

                    if (currentIED && nextIED) {
                        const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                        g.setAttribute('class', 'goose-connection');

                        const path = this.createConnectionPath(currentIED, nextIED, 'hsr-ring', '#f59e0b', 'arrow-hsr', false, 0);
                        g.appendChild(path);
                        g.appendChild(this.createMessageDot(path, '#f59e0b', '1.5s'));
                        this.svg.appendChild(g);
                    }
                }

                // Connect FIRST IED to RedBox (entry point) - using orthogonal routing
                const firstIED = iedNodesInRing[0];
                if (firstIED && rbNode) {
                    const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                    g.setAttribute('class', 'goose-connection');

                    const path = this.createOrthogonalConnectionPath(rbNode, firstIED, 'hsr-ring', '#fb923c', 'arrow-hsr');
                    g.appendChild(path);
                    g.appendChild(this.createMessageDot(path, '#fb923c', '1.8s'));
                    this.svg.appendChild(g);
                }

                // Connect LAST IED back to RedBox (close the ring) - using outer routing
                const lastIED = iedNodesInRing[iedNodesInRing.length - 1];
                if (lastIED && rbNode) {
                    const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
                    g.setAttribute('class', 'goose-connection');

                    const path = this.createOrthogonalConnectionPath(lastIED, rbNode, 'hsr-ring', '#fb923c', 'arrow-hsr', true); // isLastIED=true
                    g.appendChild(path);
                    g.appendChild(this.createMessageDot(path, '#fb923c', '1.8s'));
                    this.svg.appendChild(g);
                }
            }
        });

        // === FIXED CANVAS with VIEWBOX SCALING ===
        // Use the actual maxRowSpacing from the optimizer
        const finalHeight = layerY.ieds + (maxRowsNeeded * maxRowSpacing) + 100;
        this.svg.setAttribute('viewBox', `0 0 ${canvasWidth} ${finalHeight}`);
    }

    renderHSRTopology() {
        // High-availability Seamless Redundancy ring layout
        const nodes = this.topology.nodes;
        const nodeCount = nodes.length;

        // Circular layout with Gateway at the top
        const radius = Math.min(250, 150 + nodeCount * 10);
        const centerX = 600;
        const centerY = 300;

        // Find gateway index
        const gatewayIndex = nodes.findIndex(n => n.type === 'GATEWAY' || n.id === 'gateway');

        // Position gateway at the top (12 o'clock = -90 degrees)
        if (gatewayIndex !== -1) {
            const gateway = nodes[gatewayIndex];
            this.addNode(gateway.id, gateway.name, centerX, centerY - radius, gateway.status, gateway.type);
        }

        // Position other nodes clockwise around the ring
        let otherNodes = nodes.filter((n, i) => i !== gatewayIndex);
        const angleStep = (2 * Math.PI) / nodeCount;

        otherNodes.forEach((node, i) => {
            // Start from slightly after top, go clockwise
            const angle = -Math.PI / 2 + angleStep * (i + 1);
            const x = centerX + radius * Math.cos(angle);
            const y = centerY + radius * Math.sin(angle);
            this.addNode(node.id, node.name, x, y, node.status, node.type);
        });
    }

    createTopology() {
        // Deprecated - now using dynamic loading
        console.warn('createTopology() is deprecated, use loadTopologyFromAPI()');
    }

    addNode(id, label, x, y, status = 'online', type = 'IED', scale = 1.0) {
        const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        g.setAttribute('class', 'ied-node');
        g.setAttribute('id', id);
        g.setAttribute('transform', `translate(${x}, ${y}) scale(${scale})`);

        // Background container
        const bgRect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        bgRect.setAttribute('x', '-70');
        bgRect.setAttribute('y', '-40');
        bgRect.setAttribute('width', '140');
        bgRect.setAttribute('height', '80');
        bgRect.setAttribute('rx', '10');
        bgRect.setAttribute('fill', 'rgba(30, 30, 50, 0.8)');
        bgRect.setAttribute('stroke', 'rgba(255, 255, 255, 0.2)');
        bgRect.setAttribute('stroke-width', '2');
        g.appendChild(bgRect);

        // Device icon based on type
        const iconGroup = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        iconGroup.setAttribute('transform', 'translate(0, -10)');

        if (type === 'GATEWAY' || type === 'Gateway') {
            this.createGatewayIcon(iconGroup);
        } else if (type === 'ProtectionRelay') {
            this.createProtectionIcon(iconGroup);
        } else if (type === 'Meter') {
            this.createMeterIcon(iconGroup);
        } else if (type === 'Controller') {
            this.createControllerIcon(iconGroup);
        } else if (type === 'Switch') {
            this.createSwitchIcon(iconGroup);
        } else if (type === 'RedBox') {
            this.createRedBoxIcon(iconGroup);
        } else {
            this.createIEDIcon(iconGroup);
        }

        g.appendChild(iconGroup);

        // Label
        const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        text.setAttribute('class', 'node-label');
        text.setAttribute('text-anchor', 'middle');
        text.setAttribute('y', '25');
        text.setAttribute('font-size', '11');
        text.setAttribute('fill', '#ffffff');
        text.textContent = label;
        g.appendChild(text);

        // Status indicator
        const statusColors = {
            online: '#10b981',
            warning: '#f59e0b',
            error: '#ef4444',
            offline: '#6b7280'
        };

        const statusCircle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
        statusCircle.setAttribute('class', 'status-indicator');
        statusCircle.setAttribute('cx', '60');
        statusCircle.setAttribute('cy', '-30');
        statusCircle.setAttribute('r', '5');
        statusCircle.setAttribute('fill', statusColors[status]);

        if (status === 'online') {
            const animate = document.createElementNS('http://www.w3.org/2000/svg', 'animate');
            animate.setAttribute('attributeName', 'opacity');
            animate.setAttribute('values', '1;0.3;1');
            animate.setAttribute('dur', '2s');
            animate.setAttribute('repeatCount', 'indefinite');
            statusCircle.appendChild(animate);
        }
        g.appendChild(statusCircle);

        // HTML Tooltip events
        g.addEventListener('mouseenter', (e) => {
            if (!this.tooltip) return;
            this.tooltip.style.display = 'block';
            this.tooltip.innerHTML = `
                <div style="font-weight: bold; margin-bottom: 4px;">${label}</div>
                <div style="font-size: 11px; color: #94a3b8;">Type: ${type}</div>
                <div style="font-size: 11px; color: #94a3b8;">Status: <span style="color: ${statusColors[status]}">${status.toUpperCase()}</span></div>
            `;
        });

        g.addEventListener('mousemove', (e) => {
            if (!this.tooltip || this.tooltip.style.display === 'none') return;
            // Use clientX/clientY for stable viewport positioning
            this.tooltip.style.left = `${e.clientX + 10}px`;
            this.tooltip.style.top = `${e.clientY - 40}px`;
        });

        g.addEventListener('mouseleave', () => {
            if (this.tooltip) {
                this.tooltip.style.display = 'none';
            }
        });

        this.svg.appendChild(g);
        this.nodes.set(id, { x, y, element: g, status, type });
    }

    // Gateway icon - Dell PowerEdge Server style
    createGatewayIcon(group) {
        // Server chassis - silver/black
        const chassis = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        chassis.setAttribute('x', '-30');
        chassis.setAttribute('y', '-18');
        chassis.setAttribute('width', '60');
        chassis.setAttribute('height', '36');
        chassis.setAttribute('rx', '2');
        chassis.setAttribute('fill', '#2d3748');
        chassis.setAttribute('stroke', '#4a5568');
        chassis.setAttribute('stroke-width', '1.5');
        group.appendChild(chassis);

        // Dell branding stripe (blue)
        const stripe = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        stripe.setAttribute('x', '-30');
        stripe.setAttribute('y', '-18');
        stripe.setAttribute('width', '8');
        stripe.setAttribute('height', '36');
        stripe.setAttribute('fill', '#0078d4');
        group.appendChild(stripe);

        // Drive bays (3 horizontal slots)
        for (let i = 0; i < 3; i++) {
            const bay = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            bay.setAttribute('x', '-18');
            bay.setAttribute('y', -12 + i * 10);
            bay.setAttribute('width', '30');
            bay.setAttribute('height', '7');
            bay.setAttribute('rx', '1');
            bay.setAttribute('fill', '#1a202c');
            bay.setAttribute('stroke', '#4a5568');
            bay.setAttribute('stroke-width', '0.5');
            group.appendChild(bay);

            // Drive LED
            const led = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            led.setAttribute('cx', -16);
            led.setAttribute('cy', -8.5 + i * 10);
            led.setAttribute('r', '1');
            led.setAttribute('fill', i === 0 ? '#10b981' : '#6b7280');
            group.appendChild(led);
        }

        // Right side status LEDs
        const ledY = [-12, -6, 0, 6];
        const ledColors = ['#10b981', '#10b981', '#fbbf24', '#6b7280'];
        const ledLabels = ['PWR', 'NET', 'HDD', 'FAN'];

        ledY.forEach((y, i) => {
            // LED
            const led = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            led.setAttribute('cx', 20);
            led.setAttribute('cy', y);
            led.setAttribute('r', '1.5');
            led.setAttribute('fill', ledColors[i]);
            group.appendChild(led);

            // LED label (tiny text)
            const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            label.setAttribute('x', 15);
            label.setAttribute('y', y + 1);
            label.setAttribute('font-size', '3');
            label.setAttribute('fill', '#94a3b8');
            label.setAttribute('text-anchor', 'end');
            label.textContent = ledLabels[i];
            group.appendChild(label);
        });

        // Dell logo area
        const logoText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        logoText.setAttribute('x', 0);
        logoText.setAttribute('y', 16);
        logoText.setAttribute('font-size', '6');
        logoText.setAttribute('font-weight', 'bold');
        logoText.setAttribute('fill', '#cbd5e0');
        logoText.setAttribute('text-anchor', 'middle');
        logoText.textContent = 'DELL';
        group.appendChild(logoText);
    }

    // Protection Relay icon - ABB REF615 style
    createProtectionIcon(group) {
        // Device chassis - black panel
        const chassis = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        chassis.setAttribute('x', '-22');
        chassis.setAttribute('y', '-18');
        chassis.setAttribute('width', '44');
        chassis.setAttribute('height', '36');
        chassis.setAttribute('rx', '2');
        chassis.setAttribute('fill', '#1a1a1a');
        chassis.setAttribute('stroke', '#3d3d3d');
        chassis.setAttribute('stroke-width', '1.5');
        group.appendChild(chassis);

        // LCD Display (top section)
        const displayBg = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        displayBg.setAttribute('x', '-18');
        displayBg.setAttribute('y', '-14');
        displayBg.setAttribute('width', '36');
        displayBg.setAttribute('height', '14');
        displayBg.setAttribute('rx', '1');
        displayBg.setAttribute('fill', '#2d5016');
        displayBg.setAttribute('stroke', '#4a7c59');
        displayBg.setAttribute('stroke-width', '0.5');
        group.appendChild(displayBg);

        // LCD text simulation
        const lcdText1 = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        lcdText1.setAttribute('x', 0);
        lcdText1.setAttribute('y', -9);
        lcdText1.setAttribute('font-size', '4');
        lcdText1.setAttribute('font-family', 'monospace');
        lcdText1.setAttribute('fill', '#7dd3fc');
        lcdText1.setAttribute('text-anchor', 'middle');
        lcdText1.textContent = 'REF615';
        group.appendChild(lcdText1);

        const lcdText2 = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        lcdText2.setAttribute('x', 0);
        lcdText2.setAttribute('y', -3);
        lcdText2.setAttribute('font-size', '3.5');
        lcdText2.setAttribute('font-family', 'monospace');
        lcdText2.setAttribute('fill', '#22d3ee');
        lcdText2.setAttribute('text-anchor', 'middle');
        lcdText2.textContent = 'READY';
        group.appendChild(lcdText2);

        // LED indicators row
        const leds = [
            { x: -12, color: '#ef4444', label: 'TRIP' },
            { x: -4, color: '#fbbf24', label: 'ALRM' },
            { x: 4, color: '#10b981', label: 'RUN' },
            { x: 12, color: '#3b82f6', label: 'COM' }
        ];

        leds.forEach(led => {
            // LED circle
            const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            circle.setAttribute('cx', led.x);
            circle.setAttribute('cy', 4);
            circle.setAttribute('r', '2');
            circle.setAttribute('fill', led.label === 'RUN' ? led.color : '#2d2d2d');
            circle.setAttribute('stroke', led.color);
            circle.setAttribute('stroke-width', '0.5');
            group.appendChild(circle);

            // LED label
            const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            label.setAttribute('x', led.x);
            label.setAttribute('y', 9);
            label.setAttribute('font-size', '2.5');
            label.setAttribute('fill', '#94a3b8');
            label.setAttribute('text-anchor', 'middle');
            label.textContent = led.label;
            group.appendChild(label);
        });

        // Function buttons (bottom)
        for (let i = 0; i < 4; i++) {
            const button = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            button.setAttribute('x', -16 + i * 10);
            button.setAttribute('y', 12);
            button.setAttribute('width', '7');
            button.setAttribute('height', '4');
            button.setAttribute('rx', '0.5');
            button.setAttribute('fill', '#3d3d3d');
            button.setAttribute('stroke', '#666');
            button.setAttribute('stroke-width', '0.5');
            group.appendChild(button);
        }

        // ABB logo
        const abbLogo = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        abbLogo.setAttribute('x', -20);
        abbLogo.setAttribute('y', -16);
        abbLogo.setAttribute('font-size', '3');
        abbLogo.setAttribute('font-weight', 'bold');
        abbLogo.setAttribute('fill', '#ef4444');
        abbLogo.textContent = 'ABB';
        group.appendChild(abbLogo);
    }

    // Meter icon - ABB M2M/Energy Meter style
    createMeterIcon(group) {
        // Meter housing
        const housing = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        housing.setAttribute('x', '-24');
        housing.setAttribute('y', '-18');
        housing.setAttribute('width', '48');
        housing.setAttribute('height', '36');
        housing.setAttribute('rx', '3');
        housing.setAttribute('fill', '#f8fafc');
        housing.setAttribute('stroke', '#cbd5e0');
        housing.setAttribute('stroke-width', '1.5');
        group.appendChild(housing);

        // Display screen (black LCD)
        const screen = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        screen.setAttribute('x', '-20');
        screen.setAttribute('y', '-14');
        screen.setAttribute('width', '40');
        screen.setAttribute('height', '16');
        screen.setAttribute('rx', '1');
        screen.setAttribute('fill', '#0f172a');
        screen.setAttribute('stroke', '#1e293b');
        screen.setAttribute('stroke-width', '1');
        group.appendChild(screen);

        // Digital readings
        const reading1 = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        reading1.setAttribute('x', 0);
        reading1.setAttribute('y', -9);
        reading1.setAttribute('font-size', '5');
        reading1.setAttribute('font-family', 'monospace');
        reading1.setAttribute('font-weight', 'bold');
        reading1.setAttribute('fill', '#22c55e');
        reading1.setAttribute('text-anchor', 'middle');
        reading1.textContent = '230.5V';
        group.appendChild(reading1);

        const reading2 = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        reading2.setAttribute('x', 0);
        reading2.setAttribute('y', -2);
        reading2.setAttribute('font-size', '4');
        reading2.setAttribute('font-family', 'monospace');
        reading2.setAttribute('fill', '#3b82f6');
        reading2.setAttribute('text-anchor', 'middle');
        reading2.textContent = '50.0Hz';
        group.appendChild(reading2);

        // Phase indicators (L1, L2, L3)
        const phases = ['L1', 'L2', 'L3'];
        const phaseColors = ['#ef4444', '#fbbf24', '#3b82f6'];

        phases.forEach((phase, i) => {
            const x = -12 + i * 12;
            // LED
            const led = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            led.setAttribute('cx', x);
            led.setAttribute('cy', 6);
            led.setAttribute('r', '2.5');
            led.setAttribute('fill', phaseColors[i]);
            group.appendChild(led);

            // Label
            const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            label.setAttribute('x', x);
            label.setAttribute('y', 12);
            label.setAttribute('font-size', '3');
            label.setAttribute('fill', '#64748b');
            label.setAttribute('text-anchor', 'middle');
            label.textContent = phase;
            group.appendChild(label);
        });

        // Meter type label
        const typeLabel = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        typeLabel.setAttribute('x', 0);
        typeLabel.setAttribute('y', 16);
        typeLabel.setAttribute('font-size', '3');
        typeLabel.setAttribute('fill', '#475569');
        typeLabel.setAttribute('text-anchor', 'middle');
        typeLabel.textContent = 'ENERGY METER';
        group.appendChild(typeLabel);
    }

    // Controller icon - ABB Emax2 Circuit Breaker style
    createControllerIcon(group) {
        // Breaker housing - gray/black
        const housing = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        housing.setAttribute('x', '-26');
        housing.setAttribute('y', '-18');
        housing.setAttribute('width', '52');
        housing.setAttribute('height', '36');
        housing.setAttribute('rx', '3');
        housing.setAttribute('fill', '#374151');
        housing.setAttribute('stroke', '#4b5563');
        housing.setAttribute('stroke-width', '2');
        group.appendChild(housing);

        // Touch display area
        const display = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        display.setAttribute('x', '-22');
        display.setAttribute('y', '-14');
        display.setAttribute('width', '44');
        display.setAttribute('height', '20');
        display.setAttribute('rx', '2');
        display.setAttribute('fill', '#1e40af');
        display.setAttribute('stroke', '#3b82f6');
        display.setAttribute('stroke-width', '1');
        group.appendChild(display);

        // Display content - breaker status
        const statusText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        statusText.setAttribute('x', 0);
        statusText.setAttribute('y', -7);
        statusText.setAttribute('font-size', '5');
        statusText.setAttribute('font-weight', 'bold');
        statusText.setAttribute('fill', '#60a5fa');
        statusText.setAttribute('text-anchor', 'middle');
        statusText.textContent = 'CLOSED';
        group.appendChild(statusText);

        // Current reading
        const currentText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        currentText.setAttribute('x', 0);
        currentText.setAttribute('y', 0);
        currentText.setAttribute('font-size', '4');
        currentText.setAttribute('fill', '#93c5fd');
        currentText.setAttribute('text-anchor', 'middle');
        currentText.textContent = '1250 A';
        group.appendChild(currentText);

        // Breaker icon (simple)
        const breakerIcon = document.createElementNS('http://www.w3.org/2000/svg', 'path');
        breakerIcon.setAttribute('d', 'M -8,2 L -8,-2 M -8,-2 L 8,2 M 8,2 L 8,6');
        breakerIcon.setAttribute('stroke', '#22d3ee');
        breakerIcon.setAttribute('stroke-width', '1.5');
        breakerIcon.setAttribute('fill', 'none');
        breakerIcon.setAttribute('stroke-linecap', 'round');
        group.appendChild(breakerIcon);

        // Status LEDs
        const statusLeds = [
            { x: -18, y: 10, color: '#10b981', label: 'ON' },
            { x: -6, y: 10, color: '#6b7280', label: 'TRIP' },
            { x: 6, y: 10, color: '#6b7280', label: 'ALARM' },
            { x: 18, y: 10, color: '#10b981', label: 'COM' }
        ];

        statusLeds.forEach(led => {
            const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            circle.setAttribute('cx', led.x);
            circle.setAttribute('cy', led.y);
            circle.setAttribute('r', '2');
            circle.setAttribute('fill', led.color);
            group.appendChild(circle);
        });

        // ABB Emax branding
        const brand = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        brand.setAttribute('x', 0);
        brand.setAttribute('y', 16);
        brand.setAttribute('font-size', '4');
        brand.setAttribute('font-weight', 'bold');
        brand.setAttribute('fill', '#ef4444');
        brand.setAttribute('text-anchor', 'middle');
        brand.textContent = 'ABB Emax2';
        group.appendChild(brand);
    }

    // Generic IED icon - improved microchip style
    createIEDIcon(group) {
        // Chip body
        const chip = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        chip.setAttribute('x', '-18');
        chip.setAttribute('y', '-14');
        chip.setAttribute('width', '36');
        chip.setAttribute('height', '28');
        chip.setAttribute('rx', '2');
        chip.setAttribute('fill', '#4c1d95');
        chip.setAttribute('stroke', '#6d28d9');
        chip.setAttribute('stroke-width', '2');
        group.appendChild(chip);

        // Chip pins (left side)
        for (let i = 0; i < 4; i++) {
            const pin = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            pin.setAttribute('x', '-22');
            pin.setAttribute('y', -10 + i * 7);
            pin.setAttribute('width', '4');
            pin.setAttribute('height', '2');
            pin.setAttribute('fill', '#9ca3af');
            group.appendChild(pin);
        }

        // Chip pins (right side)
        for (let i = 0; i < 4; i++) {
            const pin = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            pin.setAttribute('x', '18');
            pin.setAttribute('y', -10 + i * 7);
            pin.setAttribute('width', '4');
            pin.setAttribute('height', '2');
            pin.setAttribute('fill', '#9ca3af');
            group.appendChild(pin);
        }

        // Chip label area (darker)
        const labelBg = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        labelBg.setAttribute('x', '-14');
        labelBg.setAttribute('y', '-10');
        labelBg.setAttribute('width', '28');
        labelBg.setAttribute('height', '20');
        labelBg.setAttribute('rx', '1');
        labelBg.setAttribute('fill', 'rgba(0, 0, 0, 0.3)');
        group.appendChild(labelBg);

        // IEC 61850 text
        const iecText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        iecText.setAttribute('x', 0);
        iecText.setAttribute('y', -2);
        iecText.setAttribute('font-size', '4');
        iecText.setAttribute('font-weight', 'bold');
        iecText.setAttribute('fill', '#a78bfa');
        iecText.setAttribute('text-anchor', 'middle');
        iecText.textContent = 'IEC';
        group.appendChild(iecText);

        const numberText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        numberText.setAttribute('x', 0);
        numberText.setAttribute('y', 4);
        numberText.setAttribute('font-size', '3.5');
        numberText.setAttribute('fill', '#c4b5fd');
        numberText.setAttribute('text-anchor', 'middle');
        numberText.textContent = '61850';
        group.appendChild(numberText);
    }

    // Network Switch icon - for PRP topology
    createSwitchIcon(group) {
        // Switch chassis - dark gray
        const chassis = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        chassis.setAttribute('x', '-28');
        chassis.setAttribute('y', '-16');
        chassis.setAttribute('width', '56');
        chassis.setAttribute('height', '32');
        chassis.setAttribute('rx', '3');
        chassis.setAttribute('fill', '#1f2937');
        chassis.setAttribute('stroke', '#374151');
        chassis.setAttribute('stroke-width', '2');
        group.appendChild(chassis);

        // Front panel
        const panel = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        panel.setAttribute('x', '-24');
        panel.setAttribute('y', '-12');
        panel.setAttribute('width', '48');
        panel.setAttribute('height', '24');
        panel.setAttribute('rx', '2');
        panel.setAttribute('fill', '#111827');
        panel.setAttribute('stroke', '#4b5563');
        panel.setAttribute('stroke-width', '0.5');
        group.appendChild(panel);

        // Network ports (8 small squares in 2 rows)
        for (let row = 0; row < 2; row++) {
            for (let col = 0; col < 4; col++) {
                const port = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                port.setAttribute('x', -18 + col * 11);
                port.setAttribute('y', -8 + row * 10);
                port.setAttribute('width', '7');
                port.setAttribute('height', '6');
                port.setAttribute('rx', '1');
                port.setAttribute('fill', '#374151');
                port.setAttribute('stroke', '#6b7280');
                port.setAttribute('stroke-width', '0.5');
                group.appendChild(port);

                // Port LED (some green, some off)
                const ledOn = (row === 0 && col < 3) || (row === 1 && col < 2);
                const led = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
                led.setAttribute('cx', -16.5 + col * 11);
                led.setAttribute('cy', -6 + row * 10);
                led.setAttribute('r', '1');
                led.setAttribute('fill', ledOn ? '#10b981' : '#374151');
                group.appendChild(led);
            }
        }

        // Switch label
        const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        label.setAttribute('x', 0);
        label.setAttribute('y', 14);
        label.setAttribute('font-size', '4');
        label.setAttribute('fill', '#94a3b8');
        label.setAttribute('text-anchor', 'middle');
        label.textContent = 'SWITCH';
        group.appendChild(label);
    }

    // RedBox Icon (Redundancy Box)
    createRedBoxIcon(group) {
        // Chassis
        const chassis = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        chassis.setAttribute('x', '-24');
        chassis.setAttribute('y', '-16');
        chassis.setAttribute('width', '48');
        chassis.setAttribute('height', '32');
        chassis.setAttribute('rx', '2');
        chassis.setAttribute('fill', '#7f1d1d'); // Dark Red
        chassis.setAttribute('stroke', '#ef4444');
        chassis.setAttribute('stroke-width', '2');
        group.appendChild(chassis);

        // Label
        const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        label.setAttribute('x', 0);
        label.setAttribute('y', 4);
        label.setAttribute('font-size', '6');
        label.setAttribute('font-weight', 'bold');
        label.setAttribute('fill', '#fecaca');
        label.setAttribute('text-anchor', 'middle');
        label.textContent = 'REDBOX';
        group.appendChild(label);

        // Ports
        const p1 = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
        p1.setAttribute('cx', -12);
        p1.setAttribute('cy', -8);
        p1.setAttribute('r', '2');
        p1.setAttribute('fill', '#10b981');
        group.appendChild(p1);

        const p2 = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
        p2.setAttribute('cx', 12);
        p2.setAttribute('cy', -8);
        p2.setAttribute('r', '2');
        p2.setAttribute('fill', '#3b82f6');
        group.appendChild(p2);
    }

    createHexagonPoints(width, height) {
        // Create hexagon points for Gateway
        const w = width;
        const h = height / 2;
        return `${-w},0 ${-w / 2},${-h} ${w / 2},${-h} ${w},0 ${w / 2},${h} ${-w / 2},${h}`;
    }

    addConnection(fromId, toId) {
        const from = this.nodes.get(fromId);
        const to = this.nodes.get(toId);

        if (!from || !to) return;

        const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        g.setAttribute('class', 'goose-connection');

        if (this.topology.type === 'HSR') {
            // HSR: Single physical connection (one Ethernet cable)
            const path = this.createConnectionPath(from, to, 'hsr-connection', '#10b981', 'arrow-a', false, 0);
            g.appendChild(path);

            // Animated message dot
            const dot = this.createMessageDot(path, '#10b981', '2s');
            g.appendChild(dot);

        } else {
            // PRP: Determine connection type based on node types

            // Check if this is a connection involving a switch (Physical Link)
            const isSwitchConnection =
                fromId === 'switch-a' || fromId === 'switch-b' ||
                toId === 'switch-a' || toId === 'switch-b';

            if (isSwitchConnection) {
                // Determine which network based on the switch involved
                // If connected to Switch A -> Network A (Green, Solid)
                // If connected to Switch B -> Network B (Blue, Dashed)
                const isNetworkA = (fromId === 'switch-a' || toId === 'switch-a');
                const color = isNetworkA ? '#10b981' : '#3b82f6';
                const dashed = !isNetworkA;

                // Single connection line (each represents one network)
                const path = this.createConnectionPath(from, to,
                    isNetworkA ? 'network-a' : 'network-b',
                    color,
                    isNetworkA ? 'arrow-a' : 'arrow-b',
                    dashed,
                    0);
                g.appendChild(path);

                // Animated message dot
                const dot = this.createMessageDot(path, color, '2s');
                g.appendChild(dot);
            } else {
                // Fallback: dual-network connection with offset (Logical View)
                const pathA = this.createConnectionPath(from, to, 'network-a', '#10b981', 'arrow-a', false, -8);
                g.appendChild(pathA);

                const pathB = this.createConnectionPath(from, to, 'network-b', '#3b82f6', 'arrow-b', true, 8);
                g.appendChild(pathB);

                const dotA = this.createMessageDot(pathA, '#10b981', '2s');
                g.appendChild(dotA);

                const dotB = this.createMessageDot(pathB, '#3b82f6', '2.2s');
                g.appendChild(dotB);
            }
        }

        // Insert before nodes
        this.svg.insertBefore(g, this.svg.firstChild.nextSibling);
        this.connections.push({ from: fromId, to: toId, element: g });
    }

    addPortLabel(from, to, port, color, offset) {
        // Add small port label near the connection midpoint
        try {
            const dx = to.x - from.x;
            const dy = to.y - from.y;
            const len = Math.sqrt(dx * dx + dy * dy);

            if (len === 0) return; // Avoid division by zero

            const perpX = -dy / len * offset;
            const perpY = dx / len * offset;

            const labelX = (from.x + to.x) / 2 + perpX;
            const labelY = (from.y + to.y) / 2 + perpY;

            const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            label.setAttribute('x', labelX);
            label.setAttribute('y', labelY);
            label.setAttribute('font-size', '9');
            label.setAttribute('font-weight', 'bold');
            label.setAttribute('fill', color);
            label.setAttribute('text-anchor', 'middle');
            label.setAttribute('opacity', '0.7');
            label.textContent = port;

            this.svg.appendChild(label);
        } catch (e) {
            console.error('Error adding port label:', e);
        }
    }

    createConnectionPath(from, to, className, color, marker, dashed, offset) {
        const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');

        try {
            // Calculate control point for curved path with offset for parallel lines
            const midX = (from.x + to.x) / 2;
            const midY = (from.y + to.y) / 2;

            // Perpendicular offset for dual lines
            const dx = to.y - from.y;
            const dy = from.x - to.x;
            const len = Math.sqrt(dx * dx + dy * dy);

            const ctrlX = len > 0 ? midX + (dx / len) * offset : midX;
            const ctrlY = len > 0 ? midY + (dy / len) * offset : midY;

            const d = `M ${from.x} ${from.y} Q ${ctrlX} ${ctrlY} ${to.x} ${to.y}`;

            // Generate unique ID with counter to avoid conflicts and decimal issues
            if (!this.pathCounter) this.pathCounter = 0;
            this.pathCounter++;
            const uniqueId = `${className}-${this.pathCounter}`;

            path.setAttribute('id', uniqueId);
            path.setAttribute('class', className);
            path.setAttribute('d', d);
            path.setAttribute('stroke', color);
            path.setAttribute('stroke-width', '2.5');
            path.setAttribute('fill', 'none');
            path.setAttribute('marker-end', `url(#${marker})`);
            path.setAttribute('opacity', '0.8');

            if (dashed) {
                path.setAttribute('stroke-dasharray', '6,4');
            }
        } catch (e) {
            console.error('Error creating connection path:', e);
        }

        return path;
    }

    // Create orthogonal (L-shaped or Z-shaped) connection path that routes around devices
    // Used for RedBox to first/last IED connections to avoid crossing over device grid
    // isLastIED: if true, routes around the outer edge of device grid
    createOrthogonalConnectionPath(from, to, className, color, marker, isLastIED = false) {
        const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');

        try {
            let d;
            const redboxOffset = 25; // Horizontal offset at RedBox to separate connection points

            if (!isLastIED) {
                // FIRST IED: Direct Z-path (RedBox → down → across → down → IED)
                // Connection point: LEFT side of RedBox
                const verticalClearance = 60;
                const redboxX = from.x - redboxOffset; // Left side of RedBox

                if (from.y < to.y) {
                    // RedBox is above IED
                    const waypoint1Y = from.y + verticalClearance;
                    const waypoint2Y = to.y - verticalClearance;

                    d = `M ${redboxX} ${from.y}
                         L ${redboxX} ${waypoint1Y}
                         L ${to.x} ${waypoint2Y}
                         L ${to.x} ${to.y}`;
                } else {
                    // IED is above RedBox (shouldn't happen for first IED)
                    const waypoint1Y = from.y - verticalClearance;
                    const waypoint2Y = to.y + verticalClearance;

                    d = `M ${redboxX} ${from.y}
                         L ${redboxX} ${waypoint1Y}
                         L ${to.x} ${waypoint2Y}
                         L ${to.x} ${to.y}`;
                }
            } else {
                // LAST IED: Outer route (IED → down → outer edge → up → across → RedBox)
                // Connection point: RIGHT side of RedBox
                // Route around the RIGHT edge of device grid

                const horizontalMargin = 100; // Distance to go outside the grid
                const verticalClearance = 60;
                const redboxX = to.x + redboxOffset; // Right side of RedBox

                // Calculate outer edge position based on IED position (from.x)
                const outerX = from.x + horizontalMargin; // Go RIGHT from IED position

                // Route: IED → down → right (outer) → up → across left to RedBox RIGHT side
                d = `M ${from.x} ${from.y}
                     L ${from.x} ${from.y + verticalClearance}
                     L ${outerX} ${from.y + verticalClearance}
                     L ${outerX} ${to.y + verticalClearance}
                     L ${redboxX} ${to.y + verticalClearance}
                     L ${redboxX} ${to.y}`;
            }

            // Generate unique ID with counter
            if (!this.pathCounter) this.pathCounter = 0;
            this.pathCounter++;
            const uniqueId = `${className}-orthogonal-${this.pathCounter}`;

            path.setAttribute('id', uniqueId);
            path.setAttribute('class', className);
            path.setAttribute('d', d);
            path.setAttribute('stroke', color);
            path.setAttribute('stroke-width', '3');
            path.setAttribute('fill', 'none');
            path.setAttribute('marker-end', `url(#${marker})`);
            path.setAttribute('opacity', '0.8');
            path.setAttribute('stroke-linejoin', 'round'); // Smooth corners

        } catch (e) {
            console.error('Error creating orthogonal connection path:', e);
        }

        return path;
    }

    createMessageDot(path, color, duration) {
        const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
        circle.setAttribute('r', '4');
        circle.setAttribute('fill', color);
        circle.setAttribute('opacity', '0.8');

        const animateMotion = document.createElementNS('http://www.w3.org/2000/svg', 'animateMotion');
        animateMotion.setAttribute('dur', duration);
        animateMotion.setAttribute('repeatCount', 'indefinite');

        const mpath = document.createElementNS('http://www.w3.org/2000/svg', 'mpath');
        mpath.setAttributeNS('http://www.w3.org/1999/xlink', 'href', `#${path.id}`);

        animateMotion.appendChild(mpath);
        circle.appendChild(animateMotion);

        return circle;
    }

    updateNetworkStats(networkA, networkB) {
        this.stats.networkA = networkA;
        this.stats.networkB = networkB;

        // Update UI elements
        this.updateStatsDisplay();
    }

    updateStatsDisplay() {
        // Update external stats cards (handled by main app)
        const event = new CustomEvent('goose-stats-update', {
            detail: this.stats
        });
        window.dispatchEvent(event);
    }

    updateNodeStatus(nodeId, status) {
        const node = this.nodes.get(nodeId);
        if (!node) return;

        const statusColors = {
            online: '#10b981',
            warning: '#f59e0b',
            error: '#ef4444',
            offline: '#6b7280'
        };

        const circle = node.element.querySelector('.status-indicator');
        circle.setAttribute('fill', statusColors[status]);

        node.status = status;
    }

    startAnimation() {
        // Animation loop for smooth updates
        const animate = () => {
            // Future: Add any frame-by-frame animations here
            requestAnimationFrame(animate);
        };
        requestAnimationFrame(animate);
    }

    destroy() {
        if (this.container && this.svg) {
            this.container.removeChild(this.svg);
        }
        this.nodes.clear();
        this.connections = [];
    }
}

// Export for use in main app
window.GOOSEMonitor = GOOSEMonitor;
