#!/usr/bin/env python3
from pathlib import Path
import os
sdk=Path(os.environ['PS2SDK'])
names=['iomanX','fileXio','usbd','bdm','bdmfs_fatfs','usbmass_bd']
roots=[sdk/'iop/irx',sdk/'iop/irx/usb',sdk/'iop/irx/usbmass']
out=['/* generated from PS2SDK IRX modules */']
for name in names:
    matches=[p for root in roots for p in root.glob('*.irx') if p.stem.lower()==name.lower()]
    if not matches:
        raise SystemExit(f'MISSING IRX {name}; inspect PS2SDK iop/irx')
    b=matches[0].read_bytes()
    symbol=name.lower()
    out.append(f'unsigned int size_{symbol}_irx = {len(b)};')
    out.append(f'unsigned char {symbol}_irx[] __attribute__((aligned(16))) = {{')
    out.extend('  '+','.join('0x%02x'%v for v in b[i:i+16])+',' for i in range(0,len(b),16))
    out.append('};')
    print(name,matches[0],len(b))
Path('src/usb_irx.c').write_text('\n'.join(out)+'\n')
