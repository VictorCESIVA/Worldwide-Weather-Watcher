/* Vrai FatFs et vrai gestionnaire de fichiers, secteurs fournis par une image. */
#include "sd_logger.h"
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
static FILE *image;
static bool panne;
void SDLogger_Disconnect(void)
{
}
DSTATUS disk_status(BYTE d)
{
    return d ? STA_NOINIT : 0;
}
DSTATUS disk_initialize(BYTE d)
{
    return disk_status(d);
}
DRESULT disk_read(BYTE d, BYTE *buf, DWORD sec, UINT n)
{
    if (d || panne || fseek(image, (long)sec * 512, SEEK_SET) || fread(buf, 512, n, image) != n)
        return RES_ERROR;
    return RES_OK;
}
DRESULT disk_write(BYTE d, const BYTE *buf, DWORD sec, UINT n)
{
    if (d || panne || fseek(image, (long)sec * 512, SEEK_SET) || fwrite(buf, 512, n, image) != n)
        return RES_ERROR;
    return RES_OK;
}
DRESULT disk_ioctl(BYTE d, BYTE cmd, void *buf)
{
    (void)buf;
    if (d || panne)
        return RES_ERROR;
    return cmd == CTRL_SYNC && fflush(image) == 0 ? RES_OK : RES_PARERR;
}
DWORD get_fattime(void)
{
    return (46u << 25) | (10u << 21) | (9u << 16);
}
static void verifier(const char *nom, unsigned taille, char valeur)
{
    FIL f;
    UINT n;
    char b[1024];
    assert(f_open(&f, nom, FA_READ) == FR_OK);
    assert(f_size(&f) == taille);
    assert(f_read(&f, b, sizeof b, &n) == FR_OK);
    assert(n == taille);
    for (unsigned i = 0; i < n; ++i)
        assert(b[i] == valeur);
    assert(f_close(&f) == FR_OK);
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    image = fopen(argv[1], "r+b");
    assert(image);
    assert(SDLogger_Mount());
    char ligne[101];
    memset(ligne, 'A', 100);
    ligne[100] = 0;
    for (unsigned i = 0; i < 3; ++i)
        assert(SDLogger_Append("261009_0.LOG", ligne, 256));
    verifier("261009_1.LOG", 200, 'A');
    verifier("261009_0.LOG", 100, 'A');
    for (unsigned i = 0; i < 24; ++i)
        assert(SDLogger_Append("261009_0.LOG", ligne, 256));
    verifier("261009_10.LOG", 200, 'A'); /* Nom long apres la revision 9. */
    SDLogger_Unmount();
    assert(SDLogger_Mount());
    verifier("261009_10.LOG", 200, 'A');
    panne = true;
    assert(!SDLogger_Append("261009_0.LOG", ligne, 256));
    assert(SDLogger_GetResult() == SDLOGGER_ERROR);
    panne = false;
    SDLogger_Unmount();
    assert(SDLogger_Mount());
    /* Remplissage du volume par un fichier artificiel. */
    FIL f;
    assert(f_open(&f, "PLEIN.BIN", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK);
    char bloc[512] = {0};
    UINT n;
    do
    {
        assert(f_write(&f, bloc, sizeof bloc, &n) == FR_OK);
    } while (n == sizeof bloc);
    assert(f_close(&f) == FR_OK);
    assert(!SDLogger_Append("261009_0.LOG", ligne, 256));
    assert(SDLogger_GetResult() == SDLOGGER_FULL);
    SDLogger_Unmount();
    fclose(image);
    puts("OK : FatFs sur image FAT16, append, revisions >9, remount, panne et SD pleine.");
}
