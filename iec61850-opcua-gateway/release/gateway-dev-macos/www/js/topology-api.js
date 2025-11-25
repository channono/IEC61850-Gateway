// Mock topology API endpoint
// This simulates the backend API response
// In production, this would be replaced by actual C++ REST API

class TopologyAPI {
    static async getTopology() {
        // Simulate network delay
        await new Promise(resolve => setTimeout(resolve, 100));

        // Return mock topology based on URL parameters or default
        const params = new URLSearchParams(window.location.search);
        const type = params.get('topology') || 'prp';
        const nodeCount = parseInt(params.get('nodes') || '5');

        return this.generateTopology(type, nodeCount);
    }

    static generateTopology(type, nodeCount) {
        const topology = {
            type: type.toUpperCase(),
            timestamp: new Date().toISOString(),
            nodes: [],
            connections: [],
            networks: []
        };

        // Generate IED nodes
        for (let i = 1; i <= nodeCount; i++) {
            topology.nodes.push({
                id: `ied-${i}`,
                name: `IED-${i}`,
                type: 'IED',
                ip: `192.168.1.${10 + i}`,
                status: Math.random() > 0.8 ? 'warning' : 'online',
                goosePublished: Math.floor(Math.random() * 5),
                gooseSubscribed: Math.floor(Math.random() * 3)
            });
        }

        // Add gateway node
        topology.nodes.push({
            id: 'gateway',
            name: 'Gateway',
            type: 'GATEWAY',
            ip: '192.168.1.1',
            status: 'online',
            goosePublished: 0,
            gooseSubscribed: nodeCount * 2
        });

        // Generate connections based on topology type
        if (type.toLowerCase() === 'hsr') {
            // HSR: Ring topology
            topology.networks = ['hsr-ring'];

            for (let i = 0; i < nodeCount; i++) {
                topology.connections.push({
                    from: topology.nodes[i].id,
                    to: topology.nodes[(i + 1) % nodeCount].id,
                    type: 'GOOSE',
                    network: 'hsr-ring'
                });
            }

            // Connect gateway to ring
            topology.connections.push({
                from: topology.nodes[nodeCount].id, // gateway
                to: topology.nodes[0].id,
                type: 'GOOSE',
                network: 'hsr-ring'
            });

        } else {
            // PRP: Dual network parallel
            topology.networks = ['eth0', 'eth1'];

            // Create mesh connections
            for (let i = 0; i < nodeCount; i++) {
                for (let j = i + 1; j < nodeCount; j++) {
                    if (Math.random() > 0.5) { // Sparse mesh
                        topology.connections.push({
                            from: topology.nodes[i].id,
                            to: topology.nodes[j].id,
                            type: 'GOOSE',
                            networks: ['eth0', 'eth1']
                        });
                    }
                }

                // Connect to gateway
                topology.connections.push({
                    from: topology.nodes[i].id,
                    to: 'gateway',
                    type: 'GOOSE',
                    networks: ['eth0', 'eth1']
                });
            }
        }

        // Add network statistics
        topology.statistics = {
            networkA: {
                interface: topology.networks[0] || 'eth0',
                messageRate: Math.floor(Math.random() * 2000) + 800,
                errors: Math.floor(Math.random() * 5),
                quality: 98 + Math.random() * 2,
                status: 'online'
            },
            networkB: topology.type === 'PRP' ? {
                interface: topology.networks[1] || 'eth1',
                messageRate: Math.floor(Math.random() * 2000) + 750,
                errors: Math.floor(Math.random() * 10),
                quality: 97 + Math.random() * 2,
                status: 'online'
            } : null
        };

        return topology;
    }
}

// Export for use by GOOSEMonitor
window.TopologyAPI = TopologyAPI;
