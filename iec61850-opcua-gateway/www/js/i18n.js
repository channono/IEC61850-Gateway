// i18n.js - Internationalization System
const i18n = {
    currentLang: 'en',

    translations: {
        en: {
            // Sidebar
            'nav.dashboard': 'Dashboard',
            'nav.ieds': 'IED Management',
            'nav.goose': 'GOOSE',
            'nav.opcua': 'OPC UA',
            'nav.settings': 'Settings',
            'nav.help': 'Help',
            'footer.version': 'v1.0.0 © 2025',
            'footer.status': 'System Online',

            // Dashboard
            'dashboard.title': 'Dashboard',
            'dashboard.ieds': 'IEDs',
            'dashboard.ieds.label': 'Connected',
            'dashboard.goose': 'GOOSE',
            'dashboard.goose.label': 'Msg/sec',
            'dashboard.opcua': 'OPC UA',
            'dashboard.opcua.label': 'Clients',
            'dashboard.cpu': 'CPU',
            'dashboard.cpu.label': 'Usage',
            'dashboard.traffic': 'Traffic Overview',
            'dashboard.alarms': 'Recent Alarms',

            // IED Management
            'ieds.title': 'IED Management',
            'ieds.configured': 'Configured IEDs',
            'ieds.add': 'Add IED',
            'ieds.name': 'Name',
            'ieds.ip': 'IP Address',
            'ieds.status': 'Status',
            'ieds.actions': 'Actions',

            // Settings
            'settings.title': 'Settings',
            'settings.system': 'System Configuration',
            'settings.system.subtitle': 'Gateway identity and logging settings',
            'settings.gateway.name': 'Gateway Name',
            'settings.language': 'Language / 语言 / 言語',
            'settings.loglevel': 'Log Level',
            'settings.api': 'API & WebSocket',
            'settings.api.subtitle': 'REST API and real-time updates configuration',
            'settings.api.port': 'API Port',
            'settings.api.restart': 'Requires Restart',
            'settings.ws.interval': 'WebSocket Update Interval (ms)',
            'settings.api.enable': 'Enable REST API',
            'settings.ws.enable': 'Enable WebSocket Updates',
            'settings.database': 'Historical Database',
            'settings.database.subtitle': 'Time-series data storage configuration',
            'settings.storage.type': 'Storage Type',
            'settings.influx.url': 'InfluxDB URL',
            'settings.influx.org': 'Organization',
            'settings.influx.bucket': 'Bucket',
            'settings.influx.token': 'Auth Token',
            'settings.timescale.conn': 'Connection String',
            'button.save': 'Save',
            'button.test': 'Test Connection',

            // Help
            'help.title': 'Help & Documentation',
            'help.getting.started': 'Getting Started',
            'help.welcome': 'Welcome to the IEC61850 to OPC UA Gateway! Here\'s how to get started:',
            'help.step1': 'Configure IEDs: Go to the "IED Management" tab to add your IEC 61850 devices. You\'ll need their IP addresses.',
            'help.step2': 'Import SCL: If you have .scd or .icd files, use the Import feature to automatically configure data points.',
            'help.step3': 'Set Up OPC UA: Check the "OPC UA" tab to configure the server port and security settings.',
            'help.step4': 'Monitor: Use the "Dashboard" to see real-time traffic and connection status.',
            'help.api': 'API Reference',
            'help.api.desc': 'The Gateway provides a REST API for external integration.',
            'help.api.docs': 'View Full API Documentation →',
            'help.about': 'About',
            'help.version': 'Version:',
            'help.build': 'Build:',
            'help.license': 'License:',

            // GOOSE
            'goose.active': 'Active Subscriptions',
            'goose.subscriptions': 'Subscriptions',
            'goose.messages': 'Messages',
            'goose.total': 'Total Received',
            'goose.rate': 'Message Rate',
            'goose.persec': 'msg/sec',
            'goose.subscriptions.title': 'GOOSE Subscriptions',
            'goose.add': 'Add Subscription',
            'goose.gocbref': 'GoCB Reference',
            'goose.dataset': 'Dataset',
            'goose.status': 'Status',
            'goose.lastmsg': 'Last Message',
            'goose.actions': 'Actions',
            'goose.recent': 'Recent Messages',
            'goose.topology': 'Network Topology',
            'goose.network.a': 'Network A (eth0)',
            'goose.network.b': 'Network B (eth1)',
            'goose.online': 'Online',
            'goose.errors': 'Errors',
            'goose.quality': 'Quality',
            // OPC UA
            'opcua.server.status': 'Server Status',
            'opcua.uptime': 'Uptime',
            'opcua.clients': 'Clients',
            'opcua.connected': 'Connected',
            'opcua.nodes': 'Nodes',
            'opcua.addressspace': 'Address Space',
            'opcua.subscriptions': 'Subscriptions',
            'opcua.active': 'Active',
            'opcua.config': 'Server Configuration',
            'opcua.config.subtitle': 'OPC UA server network and security settings',
            'opcua.port': 'Server Port',
            'opcua.endpoint': 'Endpoint URL',
            'opcua.security': 'Security Policy',
            'opcua.anonymous': 'Allow Anonymous Access',
            'opcua.restart': 'Restart Server',
            'opcua.certificates': 'Certificates',
            'opcua.certificates.subtitle': 'Server certificates and trusted certificate list',
            'opcua.cert.server': 'Server Certificate',
            'opcua.cert.export': 'Export Certificate',
            'opcua.cert.trust': 'Add Trusted Certificate',
            'opcua.clients.title': 'Connected Clients',
            'opcua.client.endpoint': 'Endpoint',
            'opcua.client.user': 'User',
            'opcua.client.security': 'Security Mode',
            'opcua.client.connected': 'Connected At',
            'opcua.client.actions': 'Actions',
            'opcua.noclients': 'No clients connected',

            // Configuration Management
            'config.title': 'Configuration Management',
            'config.subtitle': 'Import/Export SCD and gateway configuration files',
            'config.upload': 'Upload Configuration',
            'config.upload.scd': 'Upload SCD File (.scd)',
            'config.upload.scd.hint': 'IEC 61850 Substation Configuration Description file',
            'config.upload.yaml': 'Upload Gateway Config (.yaml)',
            'config.upload.yaml.hint': 'Gateway configuration file',
            'config.available': 'Available SCD Files',
            'config.refresh': 'Refresh List',
            'config.download.scd': 'Download Active SCD',
            'config.download.yaml': 'Download Gateway Config',
            'config.apply': 'Apply & Reload',
        },

        zh: {
            // 侧边栏
            'nav.dashboard': '仪表盘',
            'nav.ieds': 'IED 管理',
            'nav.goose': 'GOOSE',
            'nav.opcua': 'OPC UA',
            'nav.settings': '设置',
            'nav.help': '帮助',
            'footer.version': 'v1.0.0 © 2025',
            'footer.status': '系统在线',

            // 仪表盘
            'dashboard.title': '仪表盘',
            'dashboard.ieds': 'IED 设备',
            'dashboard.ieds.label': '已连接',
            'dashboard.goose': 'GOOSE',
            'dashboard.goose.label': '消息/秒',
            'dashboard.opcua': 'OPC UA',
            'dashboard.opcua.label': '客户端',
            'dashboard.cpu': 'CPU',
            'dashboard.cpu.label': '使用率',
            'dashboard.traffic': '流量概览',
            'dashboard.alarms': '最近告警',

            // IED 管理
            'ieds.title': 'IED 管理',
            'ieds.configured': '已配置的 IED',
            'ieds.add': '添加 IED',
            'ieds.name': '名称',
            'ieds.ip': 'IP 地址',
            'ieds.status': '状态',
            'ieds.actions': '操作',

            // 设置
            'settings.title': '设置',
            'settings.system': '系统配置',
            'settings.system.subtitle': '网关标识和日志设置',
            'settings.gateway.name': '网关名称',
            'settings.language': 'Language / 语言 / 言語',
            'settings.loglevel': '日志级别',
            'settings.api': 'API 与 WebSocket',
            'settings.api.subtitle': 'REST API 和实时更新配置',
            'settings.api.port': 'API 端口',
            'settings.api.restart': '需要重启',
            'settings.ws.interval': 'WebSocket 更新间隔 (ms)',
            'settings.api.enable': '启用 REST API',
            'settings.ws.enable': '启用 WebSocket 更新',
            'settings.database': '历史数据库',
            'settings.database.subtitle': '时序数据存储配置',
            'settings.storage.type': '存储类型',
            'settings.influx.url': 'InfluxDB URL',
            'settings.influx.org': '组织',
            'settings.influx.bucket': '数据桶',
            'settings.influx.token': '认证令牌',
            'settings.timescale.conn': '连接字符串',
            'button.save': '保存',
            'button.test': '测试连接',

            // 帮助
            'help.title': '帮助与文档',
            'help.getting.started': '快速开始',
            'help.welcome': '欢迎使用 IEC61850 到 OPC UA 网关！以下是快速入门指南：',
            'help.step1': '配置 IED：前往"IED 管理"选项卡添加您的 IEC 61850 设备。您需要知道它们的 IP 地址。',
            'help.step2': '导入 SCL：如果您有 .scd 或 .icd 文件，请使用导入功能自动配置数据点。',
            'help.step3': '设置 OPC UA：查看"OPC UA"选项卡以配置服务器端口和安全设置。',
            'help.step4': '监控：使用"仪表盘"查看实时流量和连接状态。',
            'help.api': 'API 参考',
            'help.api.desc': '网关提供用于外部集成的 REST API。',
            'help.api.docs': '查看完整 API 文档 →',
            'help.about': '关于',
            'help.version': '版本：',
            'help.build': '构建：',
            'help.license': '许可证：',

            // GOOSE
            'goose.active': '活跃订阅',
            'goose.subscriptions': '订阅',
            'goose.messages': '消息',
            'goose.total': '总接收数',
            'goose.rate': '消息速率',
            'goose.persec': '消息/秒',
            'goose.subscriptions.title': 'GOOSE 订阅',
            'goose.add': '添加订阅',
            'goose.gocbref': 'GoCB 引用',
            'goose.dataset': '数据集',
            'goose.status': '状态',
            'goose.lastmsg': '最后消息',
            'goose.actions': '操作',
            'goose.recent': '最近消息',
            'goose.topology': '网络拓扑',
            'goose.network.a': '网络 A (eth0)',
            'goose.network.b': '网络 B (eth1)',
            'goose.online': '在线',
            'goose.errors': '错误',
            'goose.quality': '质量',
            // OPC UA
            'opcua.server.status': '服务器状态',
            'opcua.uptime': '运行时间',
            'opcua.clients': '客户端',
            'opcua.connected': '已连接',
            'opcua.nodes': '节点',
            'opcua.addressspace': '地址空间',
            'opcua.subscriptions': '订阅',
            'opcua.active': '活跃',
            'opcua.config': '服务器配置',
            'opcua.config.subtitle': 'OPC UA 服务器网络和安全设置',
            'opcua.port': '服务器端口',
            'opcua.endpoint': '端点 URL',
            'opcua.security': '安全策略',
            'opcua.anonymous': '允许匿名访问',
            'opcua.restart': '重启服务器',
            'opcua.certificates': '证书',
            'opcua.certificates.subtitle': '服务器证书和受信任证书列表',
            'opcua.cert.server': '服务器证书',
            'opcua.cert.export': '导出证书',
            'opcua.cert.trust': '添加可信证书',
            'opcua.clients.title': '已连接客户端',
            'opcua.client.endpoint': '端点',
            'opcua.client.user': '用户',
            'opcua.client.security': '安全模式',
            'opcua.client.connected': '连接时间',
            'opcua.client.actions': '操作',
            'opcua.noclients': '无客户端连接',

            // Configuration Management
            'config.title': '配置管理',
            'config.subtitle': '导入/导出 SCD 和网关配置文件',
            'config.upload': '上传配置',
            'config.upload.scd': '上传 SCD 文件 (.scd)',
            'config.upload.scd.hint': 'IEC 61850 变电站配置描述文件',
            'config.upload.yaml': '上传网关配置 (.yaml)',
            'config.upload.yaml.hint': '网关配置文件',
            'config.available': '可用的 SCD 文件',
            'config.refresh': '刷新列表',
            'config.download.scd': '下载当前 SCD',
            'config.download.yaml': '下载网关配置',
            'config.apply': '应用并重载',
        },

        ja: {
            // サイドバー
            'nav.dashboard': 'ダッシュボード',
            'nav.ieds': 'IED 管理',
            'nav.goose': 'GOOSE',
            'nav.opcua': 'OPC UA',
            'nav.settings': '設定',
            'nav.help': 'ヘルプ',
            'footer.version': 'v1.0.0 © 2025',
            'footer.status': 'システム稼働中',

            // ダッシュボード
            'dashboard.title': 'ダッシュボード',
            'dashboard.ieds': 'IED デバイス',
            'dashboard.ieds.label': '接続済み',
            'dashboard.goose': 'GOOSE',
            'dashboard.goose.label': 'メッセージ/秒',
            'dashboard.opcua': 'OPC UA',
            'dashboard.opcua.label': 'クライアント',
            'dashboard.cpu': 'CPU',
            'dashboard.cpu.label': '使用率',
            'dashboard.traffic': 'トラフィック概要',
            'dashboard.alarms': '最近のアラーム',

            // IED 管理
            'ieds.title': 'IED 管理',
            'ieds.configured': '設定済み IED',
            'ieds.add': 'IED を追加',
            'ieds.name': '名前',
            'ieds.ip': 'IP アドレス',
            'ieds.status': 'ステータス',
            'ieds.actions': 'アクション',

            // 設定
            'settings.title': '設定',
            'settings.system': 'システム構成',
            'settings.system.subtitle': 'ゲートウェイ識別とログ設定',
            'settings.gateway.name': 'ゲートウェイ名',
            'settings.language': 'Language / 语言 / 言語',
            'settings.loglevel': 'ログレベル',
            'settings.api': 'API と WebSocket',
            'settings.api.subtitle': 'REST API とリアルタイム更新設定',
            'settings.api.port': 'API ポート',
            'settings.api.restart': '再起動が必要',
            'settings.ws.interval': 'WebSocket 更新間隔 (ms)',
            'settings.api.enable': 'REST API を有効化',
            'settings.ws.enable': 'WebSocket 更新を有効化',
            'settings.database': '履歴データベース',
            'settings.database.subtitle': '時系列データストレージ設定',
            'settings.storage.type': 'ストレージタイプ',
            'settings.influx.url': 'InfluxDB URL',
            'settings.influx.org': '組織',
            'settings.influx.bucket': 'バケット',
            'settings.influx.token': '認証トークン',
            'settings.timescale.conn': '接続文字列',
            'button.save': '保存',
            'button.test': '接続テスト',

            // ヘルプ
            'help.title': 'ヘルプとドキュメント',
            'help.getting.started': 'はじめに',
            'help.welcome': 'IEC61850 から OPC UA へのゲートウェイへようこそ！以下は使い始めるための手順です：',
            'help.step1': 'IED の設定："IED 管理" タブに移動して、IEC 61850 デバイスを追加します。IP アドレスが必要です。',
            'help.step2': 'SCL のインポート：.scd または .icd ファイルがある場合は、インポート機能を使用してデータポイントを自動構成します。',
            'help.step3': 'OPC UA のセットアップ："OPC UA" タブでサーバーポートとセキュリティ設定を構成します。',
            'help.step4': '監視："ダッシュボード" でリアルタイムのトラフィックと接続状態を確認します。',
            'help.api': 'API リファレンス',
            'help.api.desc': 'ゲートウェイは外部統合用の REST API を提供します。',
            'help.api.docs': '完全な API ドキュメントを表示 →',
            'help.about': '概要',
            'help.version': 'バージョン：',
            'help.build': 'ビルド：',
            'help.license': 'ライセンス：',

            // GOOSE
            'goose.active': 'アクティブなサブスクリプション',
            'goose.subscriptions': 'サブスクリプション',
            'goose.messages': 'メッセージ',
            'goose.total': '受信総数',
            'goose.rate': 'メッセージレート',
            'goose.persec': 'メッセージ/秒',
            'goose.subscriptions.title': 'GOOSE サブスクリプション',
            'goose.add': 'サブスクリプション追加',
            'goose.gocbref': 'GoCB リファレンス',
            'goose.dataset': 'データセット',
            'goose.status': 'ステータス',
            'goose.lastmsg': '最終メッセージ',
            'goose.actions': 'アクション',
            'goose.recent': '最近のメッセージ',
            'goose.topology': 'ネットワークトポロジー',
            'goose.network.a': 'ネットワーク A (eth0)',
            'goose.network.b': 'ネットワーク B (eth1)',
            'goose.online': 'オンライン',
            'goose.errors': 'エラー',
            'goose.quality': '品質',
            // OPC UA
            'opcua.server.status': 'サーバーステータス',
            'opcua.uptime': '稼働時間',
            'opcua.clients': 'クライアント',
            'opcua.connected': '接続済み',
            'opcua.nodes': 'ノード',
            'opcua.addressspace': 'アドレス空間',
            'opcua.subscriptions': 'サブスクリプション',
            'opcua.active': 'アクティブ',
            'opcua.config': 'サーバー設定',
            'opcua.config.subtitle': 'OPC UA サーバーのネットワークとセキュリティ設定',
            'opcua.port': 'サーバーポート',
            'opcua.endpoint': 'エンドポイント URL',
            'opcua.security': 'セキュリティポリシー',
            'opcua.anonymous': '匿名アクセスを許可',
            'opcua.restart': 'サーバー再起動',
            'opcua.certificates': '証明書',
            'opcua.certificates.subtitle': 'サーバー証明書と信頼された証明書リスト',
            'opcua.cert.server': 'サーバー証明書',
            'opcua.cert.export': '証明書をエクスポート',
            'opcua.cert.trust': '信頼された証明書を追加',
            'opcua.clients.title': '接続されたクライアント',
            'opcua.client.endpoint': 'エンドポイント',
            'opcua.client.user': 'ユーザー',
            'opcua.client.security': 'セキュリティモード',
            'opcua.client.connected': '接続時刻',
            'opcua.client.actions': 'アクション',
            'opcua.noclients': 'クライアント接続なし',

            // Configuration Management
            'config.title': '設定管理',
            'config.subtitle': 'SCD とゲートウェイ設定ファイルのインポート/エクスポート',
            'config.upload': '設定をアップロード',
            'config.upload.scd': 'SCD ファイルをアップロード (.scd)',
            'config.upload.scd.hint': 'IEC 61850 変電所構成記述ファイル',
            'config.upload.yaml': 'ゲートウェイ設定をアップロード (.yaml)',
            'config.upload.yaml.hint': 'ゲートウェイ設定ファイル',
            'config.available': '利用可能な SCD ファイル',
            'config.refresh': 'リストを更新',
            'config.download.scd': 'アクティブな SCD をダウンロード',
            'config.download.yaml': 'ゲートウェイ設定をダウンロード',
            'config.apply': '適用して再読み込み',
        }
    },

    init() {
        // Load saved language or default to English
        this.currentLang = localStorage.getItem('gateway_language') || 'en';
        this.updateUI();
    },

    setLanguage(lang) {
        if (this.translations[lang]) {
            this.currentLang = lang;
            localStorage.setItem('gateway_language', lang);
            this.updateUI();
        }
    },

    t(key) {
        return this.translations[this.currentLang][key] || key;
    },

    updateUI() {
        // Update all elements with data-i18n attribute
        document.querySelectorAll('[data-i18n]').forEach(element => {
            const key = element.getAttribute('data-i18n');
            element.textContent = this.t(key);
        });

        // Update all elements with data-i18n-placeholder attribute
        document.querySelectorAll('[data-i18n-placeholder]').forEach(element => {
            const key = element.getAttribute('data-i18n-placeholder');
            element.placeholder = this.t(key);
        });

        // Update page title
        const pageTitle = document.getElementById('page-title');
        if (pageTitle) {
            const activePage = document.querySelector('.nav-link.active');
            if (activePage) {
                const pageKey = activePage.getAttribute('data-page');
                const titleKey = pageKey === 'dashboard' ? 'dashboard.title' :
                    pageKey === 'ieds' ? 'ieds.title' :
                        pageKey === 'settings' ? 'settings.title' :
                            pageKey === 'help' ? 'help.title' : '';
                if (titleKey) {
                    pageTitle.textContent = this.t(titleKey);
                }
            }
        }
    }
};

// Initialize i18n on page load
document.addEventListener('DOMContentLoaded', () => {
    i18n.init();
});
