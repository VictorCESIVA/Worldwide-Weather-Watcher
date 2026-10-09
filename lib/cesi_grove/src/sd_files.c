#include "sd_logger.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
static FATFS volume;
static FIL fichier, archive;
static uint8_t copie[512];
static SDLoggerResult resultat = SDLOGGER_ERROR;

bool SDLogger_Mount(void)
{
    SDLogger_Disconnect();
    return f_mount(&volume, "", 1) == FR_OK;
}
void SDLogger_Unmount(void)
{
    f_mount(NULL, "", 0);
    SDLogger_Disconnect();
}
SDLoggerResult SDLogger_GetResult(void)
{
    return resultat;
}
static bool ecrire_complet(FIL *f, const void *data, UINT taille)
{
    UINT ecrit = 0;
    FRESULT r = f_write(f, data, taille, &ecrit);
    if (r != FR_OK)
        return false;
    if (ecrit != taille)
    {
        resultat = SDLOGGER_FULL;
        return false;
    }
    return true;
}
/* Copie terminee et fermee AVANT de tronquer le fichier 0. */
static bool archiver(const char *nom)
{
    char revision[32];
    FILINFO info;
    unsigned numero;
    for (numero = 1; numero < 1000000; ++numero)
    {
        snprintf(revision, sizeof revision, "%.6s_%u.LOG", nom, numero);
        FRESULT r = f_stat(revision, &info);
        if (r == FR_NO_FILE)
            break;
        if (r != FR_OK)
            return false;
    }
    if (numero == 1000000)
        return false;
    if (f_open(&archive, "ARCHIVE.TMP", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return false;
    bool ok = f_lseek(&fichier, 0) == FR_OK;
    while (ok)
    {
        UINT lu = 0;
        if (f_read(&fichier, copie, sizeof copie, &lu) != FR_OK)
        {
            ok = false;
            break;
        }
        if (!lu)
            break;
        ok = ecrire_complet(&archive, copie, lu);
    }
    if (f_close(&archive) != FR_OK)
        ok = false;
    if (!ok)
        return false;
    if (f_rename("ARCHIVE.TMP", revision) != FR_OK)
        return false;
    return f_lseek(&fichier, 0) == FR_OK && f_truncate(&fichier) == FR_OK &&
           f_sync(&fichier) == FR_OK;
}
bool SDLogger_Append(const char *nom, const char *ligne, uint32_t limite)
{
    resultat = SDLOGGER_ERROR;
    UINT len = strlen(ligne);
    if (!len || len > limite)
        return false;
    FATFS *fs;
    DWORD libre;
    if (f_getfree("", &libre, &fs) != FR_OK)
        return false;
    if (libre == 0)
    {
        resultat = SDLOGGER_FULL;
        return false;
    }
    if (f_open(&fichier, nom, FA_WRITE | FA_READ | FA_OPEN_ALWAYS) != FR_OK)
        return false;
    bool ok = true;
    if (f_size(&fichier) > 0 && f_size(&fichier) + len > limite)
        ok = archiver(nom);
    if (ok)
        ok = f_lseek(&fichier, f_size(&fichier)) == FR_OK && ecrire_complet(&fichier, ligne, len);
    if (ok)
        ok = f_sync(&fichier) == FR_OK;
    if (ok && f_size(&fichier) >= limite)
        ok = archiver(nom);
    if (f_close(&fichier) != FR_OK)
        ok = false;
    if (ok)
        resultat = SDLOGGER_OK;
    return ok;
}
bool SDLogger_WriteLine(const char *nom, const char *ligne)
{
    return SDLogger_Append(nom, ligne, 2048);
}
