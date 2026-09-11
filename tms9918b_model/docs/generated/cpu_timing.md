# TMS9918B CPU to VDP access times (generated)

Rules of GearSF7000's TMS99xxVramSequencer in units: 44-unit port latency (2 us), issue on the next CPU cycle, newer write replaces a waiting one. Z80 at 3.58 MHz: 1 T-state = 6 units.

| Condition | Mode | VDP delay | Time waiting for an access window | Total time | Total T-states @ 3.58 MHz |
|---|---|---|---|---|---|
| Active display area | Text (TMS) | 2 us | 0 - 1.12 us | 2 - 3.17 us | 12 |
| Active display area | Graphics I, II (TMS) | 2 us | 0 - 5.96 us | 2 - 8.01 us | 29 |
| Active display area | Multicolor (TMS) | 2 us | 0 - 1.49 us | 2 - 3.54 us | 13 |
| Whole line | Graphics1X, Graphics2Fat | 2 us | 0 - 2.98 us | 2 - 5.03 us | 18 |
| Whole line | Bitmap, BitmapQ | 2 us | 0 - 1.91 us | 2 - 3.96 us | 15 |
| Whole line | Text40X, Text40XQ | 2 us | 0 - 2.23 us | 2 - 4.28 us | 16 |
| Whole line | Text64, Text64Q | 2 us | 0 - 1.63 us | 2 - 3.68 us | 14 |
| 4300 us after vertical interrupt | All | 2 us | 0 us | 2 us | 8 |
| Register 1 blank bit 0 | All | 2 us | 0 us | 2 us | 8 |

## Writes lost by continuous loops

| Calendar | CPU cycles | OUTI chain (16 T) | OUTI+NOP (20 T) | OTIR (21 T) | OUTI+2 NOP (24 T) | OUTI+JR NZ (28 T) |
|---|---|---|---|---|---|---|
| TMS Graphics | 19 | 50.9% | 31.6% | 27.6% | 15.8% | 0.0% |
| TMS Multicolor | 51 | 8.8% | 3.5% | 2.6% | 5.3% | 0.0% |
| TMS Text | 91 | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% |
| Tiles (Graphics1X, Graphics2Fat) | 29 | 8.8% | 0.0% | 0.0% | 0.0% | 0.0% |
| Bitmap (Bitmap, BitmapQ) | 45 | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% |
| Text40 (Text40X, Text40XQ) | 67 | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% |
| Text64 (Text64, Text64Q) | 70 | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% |
