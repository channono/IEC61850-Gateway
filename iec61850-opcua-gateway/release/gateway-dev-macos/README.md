# IEC61850 OPC UA Gateway

## Quick Start

1. **Run the Gateway**
   ```bash
   ./iec61850-opcua-gateway
   ```
   (On Windows: double-click `iec61850-opcua-gateway.exe`)

2. **Open WebUI**
   Open your browser to: http://localhost:6850

3. **Connect to IEDs**
   - Go to "IED Management" tab
   - Add IED connections
   - Start monitoring

## Configuration

Edit `config/gateway.yaml` to customize:
- Gateway port (default: 6850)
- OPC UA endpoint (default: opc.tcp://localhost:4840)
- Pre-configured IED connections

## Directory Structure

```
gateway/
├── iec61850-opcua-gateway    # Main executable
├── www/                       # WebUI files
├── config/                    # Configuration files
│   └── gateway.yaml          # Main config
└── README.md                 # This file
```

## Support

For issues or questions, please contact support.
