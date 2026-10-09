/* Essai isole du lecteur SD avec des adaptateurs Grove/Dupont femelles.
 * Aucun autre capteur n'est initialise. Cree un nouveau fichier de test,
 * puis relit son contenu. Ne remplace aucun fichier deja present.
 */
#include "setup.h"
#include "stm32l4xx_hal.h"
#include "board_pins.h"
#include "sd_logger.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

extern SPI_HandleTypeDef SD_SPI_HANDLE;
static FIL fichier;
static FILINFO information;
volatile bool diag_sd_init, diag_sd_montage, diag_sd_ecriture, diag_sd_lecture;
volatile bool diag_sd_cmd0_gpio;
volatile int diag_sd_statut;
static char nom[16];

/* Comparaison reservee au diagnostic : memes contacts, sans SPI1.
 * Le compteur de cycles du Cortex-M4 impose au moins 2,5 us par demi-periode
 * (horloge au plus 200 kHz). Il ne sert pas dans le programme de la station.
 */
static void pause_spi(void)
{
    uint32_t debut = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - debut) < SystemCoreClock / 400000U)
        ;
}

static uint8_t octet_gpio(uint8_t sortie)
{
    uint8_t entree = 0;
    for (unsigned int bit = 0; bit < 8; ++bit)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,
                          (sortie & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        sortie <<= 1;
        pause_spi();
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
        pause_spi();
        entree = (uint8_t)((entree << 1) |
                          (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET));
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    }
    return entree;
}

static bool tester_cmd0_gpio(void)
{
    __HAL_SPI_DISABLE(&SD_SPI_HANDLE);
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_3 | GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    gpio.Pin = GPIO_PIN_4;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);
    printf("GPIO CS haut=%d ", HAL_GPIO_ReadPin(SD_CS_PORT, SD_CS_PIN));
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET);
    printf("bas=%d\r\n", HAL_GPIO_ReadPin(SD_CS_PORT, SD_CS_PIN));
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
    for (unsigned int i = 0; i < 10; ++i)
        (void)octet_gpio(0xFF);

    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET);
    const uint8_t commande[] = {0x40, 0, 0, 0, 0, 0x95};
    for (unsigned int i = 0; i < sizeof commande; ++i)
        (void)octet_gpio(commande[i]);
    uint8_t reponse = 0xFF;
    for (unsigned int i = 0; i < 10; ++i)
    {
        reponse = octet_gpio(0xFF);
        if ((reponse & 0x80U) == 0)
            break;
    }
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);
    (void)octet_gpio(0xFF);
    printf("GPIO CMD0 reponse=%02X (attendu 01)\r\n", reponse);
    /* Aucun acces fichier apres ce test. RESET restaure le SPI materiel. */
    return reponse == 0x01;
}

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
    else
        diag_sd_cmd0_gpio = tester_cmd0_gpio();
    printf("Fin du diagnostic. RESET pour refaire l'essai.\r\n");
    while (1)
        HAL_Delay(1000);
}
