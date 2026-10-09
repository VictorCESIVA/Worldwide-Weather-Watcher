#!/usr/bin/env python3
"""Relance l'enumeration d'un unique ST-LINK/V2.1 sous Linux.

Ne modifie ni le firmware ni les reglages USB permanents du PC.
"""
import fcntl
import os
from pathlib import Path
import sys
import time


def main():
    sondes = []
    for appareil in Path('/sys/bus/usb/devices').glob('*'):
        try:
            if ((appareil / 'idVendor').read_text().strip() == '0483'
                    and (appareil / 'idProduct').read_text().strip() == '374b'):
                sondes.append(appareil)
        except OSError:
            continue
    if len(sondes) != 1:
        sys.exit('Brancher exactement une sonde ST-LINK/V2.1 avant ce controle.')

    appareil = sondes[0]
    bus = int((appareil / 'busnum').read_text())
    numero = int((appareil / 'devnum').read_text())
    chemin = f'/dev/bus/usb/{bus:03d}/{numero:03d}'
    try:
        fichier = os.open(chemin, os.O_WRONLY)
        try:
            # USBDEVFS_RESET : _IO('U', 20), linux/usbdevice_fs.h.
            fcntl.ioctl(fichier, 0x5514, 0)
        finally:
            os.close(fichier)
    except OSError as erreur:
        sys.exit(f'Reinitialisation ST-LINK impossible : {erreur}')

    limite = time.monotonic() + 15
    while time.monotonic() < limite:
        try:
            if (appareil / 'bConfigurationValue').read_text().strip() == '1':
                print('Configuration USB ST-LINK retablie.')
                return
        except OSError:
            pass
        time.sleep(0.25)
    sys.exit('La configuration USB reste indisponible apres reinitialisation.')


if __name__ == '__main__':
    main()
