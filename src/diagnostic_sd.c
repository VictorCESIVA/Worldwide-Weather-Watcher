/* Essai isole du lecteur SD avec des adaptateurs Grove/Dupont femelles.
 * Aucun autre capteur n'est initialise. Cree un nouveau fichier de test,
 * puis relit son contenu. Ne remplace aucun fichier deja present.
 */
#include "setup.h"
#include "stm32l4xx_hal.h"
#include "sd_logger.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

static FIL fichier;
static FILINFO information;
volatile bool diag_sd_init, diag_sd_montage, diag_sd_ecriture, diag_sd_lecture;
volatile int diag_sd_statut;
static char nom[16];

static bool tester_fichier(void)
{
    bool libre = false;
    for (unsigned int i = 0; i < 1000; ++i)
    {
        snprintf(nom, sizeof nom, "3WWT%03u.LOG", i);
        FRESULT resultat = f_stat(nom, &information);
        if (resultat == FR_NO_FILE)
        {
            libre = true;
            break;
        }
        if (resultat != FR_OK)
            return false;
    }
    if (!libre || f_open(&fichier, nom, FA_CREATE_NEW | FA_WRITE) != FR_OK)
        return false;

    static const char contenu[] = "3WW test SD\r\n";
    UINT taille;
    FRESULT resultat = f_write(&fichier, contenu, sizeof contenu - 1, &taille);
    bool ecrit = resultat == FR_OK && taille == sizeof contenu - 1;
    if (ecrit)
        ecrit = f_sync(&fichier) == FR_OK;
    if (f_close(&fichier) != FR_OK)
        ecrit = false;
    diag_sd_ecriture = ecrit;
    if (!ecrit || f_open(&fichier, nom, FA_READ) != FR_OK)
        return false;

    char relu[sizeof contenu] = {0};
    resultat = f_read(&fichier, relu, sizeof contenu - 1, &taille);
    bool correct = resultat == FR_OK && taille == sizeof contenu - 1 &&
                   memcmp(contenu, relu, sizeof contenu - 1) == 0;
    if (f_close(&fichier) != FR_OK)
        correct = false;
    diag_sd_lecture = correct;
    return correct;
}

int main(void)
{
    Global_Init();
    printf("\r\n3WW diagnostic SD Grove : D2=CS D3=SCK D4=MOSI D5=MISO\r\n");
    diag_sd_init = SDLogger_Init();
    diag_sd_statut = SDLogger_GetLastInitStatus();
    printf("SD init=%d statut=%d\r\n", diag_sd_init, diag_sd_statut);
    if (diag_sd_init)
    {
        diag_sd_montage = SDLogger_Mount();
        printf("SD FAT montee=%d\r\n", diag_sd_montage);
        if (diag_sd_montage)
        {
            bool correct = tester_fichier();
            printf("SD fichier=%s ecriture=%d relecture=%d test=%d\r\n", nom,
                   diag_sd_ecriture, diag_sd_lecture, correct);
        }
        SDLogger_Unmount();
    }
    printf("Fin du diagnostic. RESET pour refaire l'essai.\r\n");
    while (1)
        HAL_Delay(1000);
}
