# TMS9918B VRAM cycles against DRAM data sheets (generated)

VDP assumptions: RD bus setup 40 ns before the CAS rising edge, board delay budget 20 ns (strobe out plus data back), unit 46.56 ns.

Waveform of an n-byte cycle: RAS low [0, 5n-1), precharge 3 units; CAS for byte k low [1+5k, 4+5k); single-byte writes are late writes: WE low [2, 4), data on AD after the column address.

## Fujitsu MB8118-12

### Read, 1 byte (7 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 325.9 | 55.9 |
| tRAS RAS low | 140.0 | 186.2 | 46.2 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |

### Read, 2 bytes (12 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 558.7 | 288.7 |
| tRPM RAS low (page mode) | 140.0 | 419.0 | 279.0 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| tCP CAS precharge (page mode) | 70.0 | 93.1 | 23.1 |
| tPC page mode cycle | 145.0 | 232.8 | 87.8 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |
| data byte k>0: tCAC + delays + VDP setup | 125.0 | 139.7 | 14.7 |

### Read, 3 bytes (17 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 791.5 | 521.5 |
| tRPM RAS low (page mode) | 140.0 | 651.9 | 511.9 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| tCP CAS precharge (page mode) | 70.0 | 93.1 | 23.1 |
| tPC page mode cycle | 145.0 | 232.8 | 87.8 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |
| data byte k>0: tCAC + delays + VDP setup | 125.0 | 139.7 | 14.7 |

### Read, 4 bytes (22 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 1024.3 | 754.3 |
| tRPM RAS low (page mode) | 140.0 | 884.7 | 744.7 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| tCP CAS precharge (page mode) | 70.0 | 93.1 | 23.1 |
| tPC page mode cycle | 145.0 | 232.8 | 87.8 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |
| data byte k>0: tCAC + delays + VDP setup | 125.0 | 139.7 | 14.7 |

### Write, 1 byte: PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 325.9 | 55.9 |
| tRAS RAS low | 140.0 | 186.2 | 46.2 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| column address hold + bus switch before data setup (tCAH) | 25.0 | 46.6 | 21.6 |
| tWCH write hold after CAS fall | 35.0 | 139.7 | 104.7 |
| tWCR write hold from RAS | 90.0 | 186.2 | 96.2 |
| tWP write pulse | 35.0 | 93.1 | 58.1 |
| tRWL write to RAS rise | 65.0 | 93.1 | 28.1 |
| tCWL write to CAS rise | 50.0 | 93.1 | 43.1 |
| tDH data hold after WE fall | 35.0 | 93.1 | 58.1 |
| tDHR data hold from RAS | 90.0 | 186.2 | 96.2 |

## Motorola MCM4517-12

### Read, 1 byte (7 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 325.9 | 55.9 |
| tRAS RAS low | 140.0 | 186.2 | 46.2 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |

### Read, 2 bytes (12 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 558.7 | 288.7 |
| tRPM RAS low (page mode) | 140.0 | 419.0 | 279.0 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| tCP CAS precharge (page mode) | 70.0 | 93.1 | 23.1 |
| tPC page mode cycle | 145.0 | 232.8 | 87.8 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |
| data byte k>0: tCAC + delays + VDP setup | 125.0 | 139.7 | 14.7 |

### Read, 3 bytes (17 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 791.5 | 521.5 |
| tRPM RAS low (page mode) | 140.0 | 651.9 | 511.9 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| tCP CAS precharge (page mode) | 70.0 | 93.1 | 23.1 |
| tPC page mode cycle | 145.0 | 232.8 | 87.8 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |
| data byte k>0: tCAC + delays + VDP setup | 125.0 | 139.7 | 14.7 |

### Read, 4 bytes (22 units): PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 1024.3 | 754.3 |
| tRPM RAS low (page mode) | 140.0 | 884.7 | 744.7 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| tCP CAS precharge (page mode) | 70.0 | 93.1 | 23.1 |
| tPC page mode cycle | 145.0 | 232.8 | 87.8 |
| data byte 0: access + delays + VDP setup | 180.0 | 186.2 | 6.2 |
| data byte k>0: tCAC + delays + VDP setup | 125.0 | 139.7 | 14.7 |

### Write, 1 byte: PASS

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 270.0 | 325.9 | 55.9 |
| tRAS RAS low | 140.0 | 186.2 | 46.2 |
| tRP RAS precharge | 120.0 | 139.7 | 19.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 55.0 | 8.4 |
| tCAS CAS low | 65.0 | 139.7 | 74.7 |
| tCSH CAS hold after RAS fall | 120.0 | 186.2 | 66.2 |
| tRSH RAS hold after last CAS fall | 85.0 | 139.7 | 54.7 |
| tCPN CAS precharge before next cycle | 55.0 | 186.2 | 131.2 |
| tAR column address hold from RAS | 70.0 | 186.2 | 116.2 |
| tCAH column address hold | 15.0 | 139.7 | 124.7 |
| tRAH + tASC within tRCD | 15.0 | 46.6 | 31.6 |
| column address hold + bus switch before data setup (tCAH) | 25.0 | 46.6 | 21.6 |
| tWCH write hold after CAS fall | 30.0 | 139.7 | 109.7 |
| tWCR write hold from RAS | 85.0 | 186.2 | 101.2 |
| tWP write pulse | 30.0 | 93.1 | 63.1 |
| tRWL write to RAS rise | 65.0 | 93.1 | 28.1 |
| tCWL write to CAS rise | 50.0 | 93.1 | 43.1 |
| tDH data hold after WE fall | 30.0 | 93.1 | 63.1 |
| tDHR data hold from RAS | 85.0 | 186.2 | 101.2 |

## Motorola MCM4517-15

### Read, 1 byte (7 units): FAIL

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 320.0 | 325.9 | 5.9 |
| tRAS RAS low | 175.0 | 186.2 | 11.2 |
| tRP RAS precharge | 135.0 | 139.7 | 4.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 70.0 | 23.4 |
| tCAS CAS low | 95.0 | 139.7 | 44.7 |
| tCSH CAS hold after RAS fall | 165.0 | 186.2 | 21.2 |
| tRSH RAS hold after last CAS fall | 105.0 | 139.7 | 34.7 |
| tCPN CAS precharge before next cycle | 70.0 | 186.2 | 116.2 |
| tAR column address hold from RAS | 90.0 | 186.2 | 96.2 |
| tCAH column address hold | 20.0 | 139.7 | 119.7 |
| tRAH + tASC within tRCD | 20.0 | 46.6 | 26.6 |
| data byte 0: access + delays + VDP setup | 210.0 | 186.2 | **-23.8 |

### Read, 2 bytes (12 units): FAIL

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 320.0 | 558.7 | 238.7 |
| tRPM RAS low (page mode) | 175.0 | 419.0 | 244.0 |
| tRP RAS precharge | 135.0 | 139.7 | 4.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 70.0 | 23.4 |
| tCAS CAS low | 95.0 | 139.7 | 44.7 |
| tCSH CAS hold after RAS fall | 165.0 | 186.2 | 21.2 |
| tRSH RAS hold after last CAS fall | 105.0 | 139.7 | 34.7 |
| tCPN CAS precharge before next cycle | 70.0 | 186.2 | 116.2 |
| tAR column address hold from RAS | 90.0 | 186.2 | 96.2 |
| tCAH column address hold | 20.0 | 139.7 | 119.7 |
| tRAH + tASC within tRCD | 20.0 | 46.6 | 26.6 |
| tCP CAS precharge (page mode) | 85.0 | 93.1 | 8.1 |
| tPC page mode cycle | 190.0 | 232.8 | 42.8 |
| data byte 0: access + delays + VDP setup | 210.0 | 186.2 | **-23.8 |
| data byte k>0: tCAC + delays + VDP setup | 140.0 | 139.7 | **-0.3 |

### Read, 3 bytes (17 units): FAIL

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 320.0 | 791.5 | 471.5 |
| tRPM RAS low (page mode) | 175.0 | 651.9 | 476.9 |
| tRP RAS precharge | 135.0 | 139.7 | 4.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 70.0 | 23.4 |
| tCAS CAS low | 95.0 | 139.7 | 44.7 |
| tCSH CAS hold after RAS fall | 165.0 | 186.2 | 21.2 |
| tRSH RAS hold after last CAS fall | 105.0 | 139.7 | 34.7 |
| tCPN CAS precharge before next cycle | 70.0 | 186.2 | 116.2 |
| tAR column address hold from RAS | 90.0 | 186.2 | 96.2 |
| tCAH column address hold | 20.0 | 139.7 | 119.7 |
| tRAH + tASC within tRCD | 20.0 | 46.6 | 26.6 |
| tCP CAS precharge (page mode) | 85.0 | 93.1 | 8.1 |
| tPC page mode cycle | 190.0 | 232.8 | 42.8 |
| data byte 0: access + delays + VDP setup | 210.0 | 186.2 | **-23.8 |
| data byte k>0: tCAC + delays + VDP setup | 140.0 | 139.7 | **-0.3 |

### Read, 4 bytes (22 units): FAIL

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 320.0 | 1024.3 | 704.3 |
| tRPM RAS low (page mode) | 175.0 | 884.7 | 709.7 |
| tRP RAS precharge | 135.0 | 139.7 | 4.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 70.0 | 23.4 |
| tCAS CAS low | 95.0 | 139.7 | 44.7 |
| tCSH CAS hold after RAS fall | 165.0 | 186.2 | 21.2 |
| tRSH RAS hold after last CAS fall | 105.0 | 139.7 | 34.7 |
| tCPN CAS precharge before next cycle | 70.0 | 186.2 | 116.2 |
| tAR column address hold from RAS | 90.0 | 186.2 | 96.2 |
| tCAH column address hold | 20.0 | 139.7 | 119.7 |
| tRAH + tASC within tRCD | 20.0 | 46.6 | 26.6 |
| tCP CAS precharge (page mode) | 85.0 | 93.1 | 8.1 |
| tPC page mode cycle | 190.0 | 232.8 | 42.8 |
| data byte 0: access + delays + VDP setup | 210.0 | 186.2 | **-23.8 |
| data byte k>0: tCAC + delays + VDP setup | 140.0 | 139.7 | **-0.3 |

### Write, 1 byte: FAIL

| Parameter | Required (ns) | Provided (ns) | Margin (ns) |
|---|---|---|---|
| tRC cycle time | 320.0 | 325.9 | 5.9 |
| tRAS RAS low | 175.0 | 186.2 | 11.2 |
| tRP RAS precharge | 135.0 | 139.7 | 4.7 |
| tRCD RAS to CAS delay (min) | 25.0 | 46.6 | 21.6 |
| tRCD RAS to CAS delay (max, tRAC applies) | 46.6 | 70.0 | 23.4 |
| tCAS CAS low | 95.0 | 139.7 | 44.7 |
| tCSH CAS hold after RAS fall | 165.0 | 186.2 | 21.2 |
| tRSH RAS hold after last CAS fall | 105.0 | 139.7 | 34.7 |
| tCPN CAS precharge before next cycle | 70.0 | 186.2 | 116.2 |
| tAR column address hold from RAS | 90.0 | 186.2 | 96.2 |
| tCAH column address hold | 20.0 | 139.7 | 119.7 |
| tRAH + tASC within tRCD | 20.0 | 46.6 | 26.6 |
| column address hold + bus switch before data setup (tCAH) | 30.0 | 46.6 | 16.6 |
| tWCH write hold after CAS fall | 45.0 | 139.7 | 94.7 |
| tWCR write hold from RAS | 115.0 | 186.2 | 71.2 |
| tWP write pulse | 50.0 | 93.1 | 43.1 |
| tRWL write to RAS rise | 110.0 | 93.1 | -16.9 |
| tCWL write to CAS rise | 100.0 | 93.1 | -6.9 |
| tDH data hold after WE fall | 45.0 | 93.1 | 48.1 |
| tDHR data hold from RAS | 115.0 | 186.2 | 71.2 |

