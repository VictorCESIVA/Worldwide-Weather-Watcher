#!/usr/bin/env python3
"""Lit les sections reelles de l'ELF et les estimations de pile produites par GCC."""
from pathlib import Path
import subprocess,sys,json
mode=sys.argv[1] if len(sys.argv)>1 else 'release'
base=Path('.pio/build')/mode
outil=Path.home()/'.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-size'
if not (base/'firmware.elf').exists():raise SystemExit('Compiler avant de mesurer.')
resultat=subprocess.check_output([str(outil),'-A',str(base/'firmware.elf')],text=True)
print(resultat)
nm=outil.with_name('arm-none-eabi-nm')
symboles={ligne.split()[-1] for ligne in subprocess.check_output([str(nm),'--defined-only',str(base/'firmware.elf')],text=True).splitlines() if ligne.split()}
rows=[]
for fichier in base.rglob('*.su'):
 for ligne in fichier.read_text().splitlines():
  champs=ligne.split('\t')
  if len(champs)==3 and champs[0].rsplit(':',1)[-1] in symboles: rows.append((int(champs[1]),champs[0],champs[2]))
print('Plus grands cadres GCC des fonctions liees (un cadre seul, pas le pic de pile cumule) :')
for taille,fonction,genre in sorted(rows,reverse=True)[:15]:print(f'{taille:5} octets {fonction} [{genre}]')
print('Les .su ne couvrent pas les bibliotheques C precompilees. Le pic avec interruptions doit etre mesure sur carte.')
