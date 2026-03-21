graph TD
    classDef ring3 fill:#1e1e1e,stroke:#4caf50,stroke-width:2px,color:#fff;
    classDef ring0 fill:#2d0000,stroke:#f44336,stroke-width:2px,color:#fff;
    classDef target fill:#001f3f,stroke:#2196f3,stroke-width:2px,color:#fff;
    classDef note fill:#333,stroke:#fff,stroke-width:1px,color:#fff;

    subgraph "RING 3 (USER MODE) - Restricted Sandbox"
        HI["hack_injector.exe<br>(The Deployment Tool)"]:::ring3
        WS["watchdog_stealth.exe<br>(The Security Monitor)"]:::ring3
    end

    subgraph "THE KERNEL BARRIER (Bypassing Syscalls)"
        direction TB
    end

    subgraph "RING 0 (KERNEL MODE) - Absolute Authority"
        BB["BlackBoneDrv.sys<br>(The Core Execution Engine)"]:::ring0
    end

    subgraph "TARGET PROCESS SPACE: app.exe (RING 3)"
        HP["hack_payload.dll<br>(Hidden from OS VAD)"]:::target
        ES["environmentState: 0"]:::target
        ACTION["7. Watchdog triggers. Driver overwrites '1' to '99'.<br>8. app.exe is cleanly terminated."]:::note
    end

    %% Connections across the Kernel Barrier
    HI -->|1. Commands driver to map DLL| BB
    WS -->|2. Commands driver to hook memory| BB
    
    %% Kernel to Target Process
    BB -->|3. Allocates physical memory & copies payload| HP
    BB -->|4. Configures CPU Debug Registers - DR0, DR1, DR2, DR3| ES
    
    %% Target Process Internal Interactions
    HP -.->|5. Writes '1'| ES
    ES -.->|6. CPU Hardware Halts!| ACTION